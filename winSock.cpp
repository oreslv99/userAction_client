#include "winSock.h"

const int REQUEST_DOWNLOAD_RULEDATA = 0;
const int REQUEST_UPLOAD_USERDATA	= 1;

//
// public
//
winSock::winSock(std::wstring ip, std::wstring port)
	: ip(ip), port(port), initialized(false), addrInfo(nullptr)
{
}
winSock::~winSock()
{
	if (this->addrInfo != nullptr)
	{
		::FreeAddrInfoW(this->addrInfo);
		this->addrInfo = nullptr;
	}

	::WSACleanup();
}
bool winSock::initialize()
{
	this->initialized = false;

	if ((this->ip.length() <= 0) || (this->port.length() <= 0))
	{
		help->writeLog(logId::error, L"[%s:%03d] Invalid ip or port.", __FUNCTIONW__, __LINE__);
		return this->initialized;
	}

	int errorNo = -1;

	// �ʱ�ȭ
	WSADATA wsaData;
	errorNo = ::WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (errorNo != 0)
	{
		help->writeLog(logId::error, L"[%s:%03d] code[%d] WSAStartup is failed.", __FUNCTIONW__, __LINE__, errorNo);
		return this->initialized;
	}

	// ���� ���� ����
	addrinfoW hints;
	::memset(&hints, 0x00, sizeof(addrinfoW));
	hints.ai_family = PF_INET; //PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	errorNo = ::GetAddrInfoW(this->ip.c_str(), this->port.c_str(), &hints, &this->addrInfo);
	if (errorNo != 0)
	{
		help->writeLog(logId::error, L"[%s:%03d] code[%d] GetAddrInfoW is failed.", __FUNCTIONW__, __LINE__, errorNo);
		return this->initialized;
	}

	// ����
	SOCKET socket = ::WSASocketW(
		this->addrInfo->ai_family,
		this->addrInfo->ai_socktype,
		this->addrInfo->ai_protocol, nullptr, 0, WSA_FLAG_OVERLAPPED);
	if (socket == INVALID_SOCKET)
	{
		help->writeLog(logId::error, L"[%s:%03d] code[%d] WSASocketW is failed.", __FUNCTIONW__, __LINE__, ::WSAGetLastError());
		return this->initialized;
	}

	// ����
	if (::connect(socket, this->addrInfo->ai_addr, static_cast<int>(this->addrInfo->ai_addrlen)) == SOCKET_ERROR)
	{
		int err = ::WSAGetLastError();
		std::wstring errMessage = L"";
		switch (err)
		{
		case WSAECONNREFUSED: 
			errMessage += L"The attempt to connect was forcefully rejected."; 
			break;
		case WSAETIMEDOUT: 
			errMessage += L"An attempt to connect timed out without establishing a connection";
			break;
		default:
			errMessage += L"code[";
			errMessage += std::to_wstring(err);
			errMessage += L"] connect is failed.";
			break;
		}

		help->writeLog(logId::error, L"[%s:%03d] %s", __FUNCTIONW__, __LINE__, errMessage.c_str());
		return this->initialized;
	}

	::closesocket(socket);

	return (this->initialized = true);
}
bool winSock::isOnline() const
{
	return this->initialized;
}
bool winSock::request(requestId id, std::wstring *buffer)
{
	bool result = false;
	switch (id)
	{
	case rule:
		result = requestRule(buffer);
		break;
	case uploadUserData:
		break;
	}

	return result;
}

//
// private
//
bool winSock::generateHeader(headerData header, bool newLine, std::string *buffer)
{
	bool result = false;

	// json data ����
	jsonDocumentForWriteA jsonResult;
	jsonResult.SetObject();

	// id
	jsonValueForWriteA id;
	id.SetInt(header.id);
	jsonResult.AddMember("id", id, jsonResult.GetAllocator());

	// param
	jsonValueForWriteA param;
	if (header.moreData == true)
	{
		// TODO: ����� ������ ����
	}
	else
	{
		param.SetNull();
	}
	jsonResult.AddMember("param", param, jsonResult.GetAllocator());

	// noData
	jsonValueForWriteA moreData;
	moreData.SetBool(header.moreData);
	jsonResult.AddMember("moreData", moreData, jsonResult.GetAllocator());

	// buffer ����
	jsonStringBufferA stringBuffer;
	stringBuffer.Clear();
	jsonStringWriterA stringWriter(stringBuffer);
	jsonResult.Accept(stringWriter);

	*buffer += stringBuffer.GetString();
	if (newLine == true)
	{
		*buffer += "\n";
	}

	return (buffer->empty() == false);
}
bool winSock::requestRule(std::wstring *buffer)
{
	bool result = false;

#pragma region ����
	// ����
	SOCKET socket = ::WSASocketW(
		this->addrInfo->ai_family,
		this->addrInfo->ai_socktype,
		this->addrInfo->ai_protocol, nullptr, 0, WSA_FLAG_OVERLAPPED);
	if (socket == INVALID_SOCKET)
	{
		help->writeLog(logId::error, L"[%s:%03d] code[%d] WSASocketW is failed.", __FUNCTIONW__, __LINE__, ::WSAGetLastError());
		return this->initialized;
	}

	while (true)
	{
		// ����
		if (::connect(socket, this->addrInfo->ai_addr, static_cast<int>(this->addrInfo->ai_addrlen)) == SOCKET_ERROR)
		{
			help->writeLog(logId::error, L"[%s:%03d] code[%d] connect is failed.", __FUNCTIONW__, __LINE__, ::WSAGetLastError());
			break;
		}
#pragma endregion

		int err = 0;

#pragma region request ����
		//// overlapped (Async & Non-blocking)
		//WSAEVENT sendEvents[1];
		//sendEvents[0] = ::WSACreateEvent();
		//WSAOVERLAPPED sendOverlapped;
		//::memset(&sendOverlapped, 0x00, sizeof(WSAOVERLAPPED));
		//sendOverlapped.hEvent = sendEvents[0];

		// ���
		headerData header;
		::memset(&header, 0x00, sizeof(headerData));
		header.id = REQUEST_DOWNLOAD_RULEDATA;
		header.moreData = false;

		// ������ �̸� �Ծ��س��� ���˿� ���� �������
		std::string jsonData;
		generateHeader(header, false, &jsonData);

		WSABUF sendBuffer[1];
		::memset(sendBuffer, 0x00, sizeof(WSABUF));
		sendBuffer[0].len = jsonData.length();
		sendBuffer[0].buf = const_cast<char*>(jsonData.c_str());

		// ����
		DWORD numberOfBytesSent = 0;
		err = ::WSASend(socket, sendBuffer, _countof(sendBuffer), &numberOfBytesSent, 0, nullptr, nullptr);
		if ((err == SOCKET_ERROR) && (WSA_IO_PENDING != (err = ::WSAGetLastError())))
		{
			help->writeLog(logId::error, L"[%s:%03d] code[%d] WSASend is failed.", __FUNCTIONW__, __LINE__, err);
			break;
		}
#pragma endregion

#pragma region request return
		WSABUF recvBuffer[1];
		// TODO: ���۸� �˳��ϰ� �ϴ°ź��� ����ũ��� ó���� �� �ִ��� Ȯ���� ��
		char buffer_header[2048];
		::memset(buffer_header, 0x00, sizeof(buffer_header));
		recvBuffer[0].len = _countof(buffer_header);
		recvBuffer[0].buf = buffer_header;

		bool recieved_header = false;
		DWORD numberOfBytesReceived = 0;
		DWORD flags = 0;
		while (true)
		{
			err = ::WSARecv(socket, recvBuffer, _countof(recvBuffer), &numberOfBytesReceived, &flags, nullptr, nullptr);
			if ((err == SOCKET_ERROR) && (WSA_IO_PENDING != (err = ::WSAGetLastError())))
			{
				help->writeLog(logId::error, L"[%s:%03d] code[%d] WSARecv is failed.", __FUNCTIONW__, __LINE__, err);
				break;
			}

			if ((recvBuffer[0].len <= 0) || (recvBuffer[0].buf == nullptr))
			{
				help->writeLog(logId::warning, L"[%s:%03d] code[%d] Invalid received data.", __FUNCTIONW__, __LINE__);
				break;
			}

			// c ��Ÿ�Ϸ� ��ȯ�Ͽ� ���
			size_t length = ::MultiByteToWideChar(CP_ACP, 0, recvBuffer[0].buf, -1, nullptr, 0);
			length++;
			buffer->resize(length);
			::MultiByteToWideChar(CP_ACP, 0, recvBuffer[0].buf, -1, const_cast<wchar_t*>(buffer->data()), length);
			result = (buffer->empty() == false);
			break;
		}
#pragma endregion

		break;
	}

	// �����ʿ����� ���ϲ���. �ٵ� �������� ��� ������ ǥ�õ� (code: 'EPIPE', Error: This socket has been ended by the other party)
	::closesocket(socket);

	return result;
}
bool winSock::requestUpload(std::wstring path)
{
	bool result = false;

#pragma region ����
	// ����
	SOCKET socket = ::WSASocketW(
		this->addrInfo->ai_family,
		this->addrInfo->ai_socktype,
		this->addrInfo->ai_protocol, nullptr, 0, WSA_FLAG_OVERLAPPED);
	if (socket == INVALID_SOCKET)
	{
		help->writeLog(logId::error, L"[%s:%03d] code[%d] WSASocketW is failed.", __FUNCTIONW__, __LINE__, ::WSAGetLastError());
		return this->initialized;
	}

	while (true)
	{
		// ����
		if (::connect(socket, this->addrInfo->ai_addr, static_cast<int>(this->addrInfo->ai_addrlen)) == SOCKET_ERROR)
		{
			help->writeLog(logId::error, L"[%s:%03d] code[%d] connect is failed.", __FUNCTIONW__, __LINE__, ::WSAGetLastError());
			break;
		}
#pragma endregion

#pragma region ����� ������ Ȯ��
		std::ifstream stream(path, std::ios::in);
		if (stream.is_open() == false)
		{
			help->writeLog(logId::error, L"[%s:%03d] code[%d] is_open is failed.", __FUNCTIONW__, __LINE__, ::GetLastError());
			return result;
		}

		// size Ȯ���� ���� position �̵�
		stream.seekg(0, std::ios::end);

		// ���
		headerData header;
		::memset(&header, 0x00, sizeof(headerData));
		header.id = REQUEST_UPLOAD_USERDATA;
		header.moreData = true;
		header.param = nullptr;	// TODO: ���������� ������ �ʿ��Ѱ�? (unique name (�����+@) / ����ũ�� (����ũ��) ��)

		// ������ �̸� �Ծ��س��� ���˿� ���� �������
		std::string jsonData;
		generateHeader(header, true, &jsonData);
#pragma endregion

		int err = 0;

#pragma region request ����
		WSABUF sendBuffer[1];
		::memset(sendBuffer, 0x00, sizeof(WSABUF));
		sendBuffer[0].len = jsonData.length();
		sendBuffer[0].buf = const_cast<char*>(jsonData.c_str());

		// ����
		DWORD numberOfBytesSent = 0;
		err = ::WSASend(socket, sendBuffer, _countof(sendBuffer), &numberOfBytesSent, 0, nullptr, nullptr);
		if ((err == SOCKET_ERROR) && (WSA_IO_PENDING != (err = ::WSAGetLastError())))
		{
			help->writeLog(logId::error, L"[%s:%03d] code[%d] WSASend is failed.", __FUNCTIONW__, __LINE__, err);
			break;
		}
#pragma endregion

		break;
	}

	::closesocket(socket);

	return result;
}