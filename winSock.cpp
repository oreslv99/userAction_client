#include "winSock.h"

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

	// 초기화
	WSADATA wsaData;
	errorNo = ::WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (errorNo != 0)
	{
		help->writeLog(logId::error, L"[%s:%03d] code[%d] WSAStartup is failed.", __FUNCTIONW__, __LINE__, errorNo);
		return this->initialized;
	}

	// 소켓 정보 설정
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

	// 소켓
	SOCKET socket = ::WSASocketW(
		this->addrInfo->ai_family,
		this->addrInfo->ai_socktype,
		this->addrInfo->ai_protocol, nullptr, 0, WSA_FLAG_OVERLAPPED);
	if (socket == INVALID_SOCKET)
	{
		help->writeLog(logId::error, L"[%s:%03d] code[%d] WSASocketW is failed.", __FUNCTIONW__, __LINE__, ::WSAGetLastError());
		return this->initialized;
	}

	// 연결
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
enum socket_status
{
	socket_status_error = 0,
	socket_status_success,
	socket_status_warning,
};
enum socket_command
{
	socket_command_request_rule = 0,
	socket_command_store_file,
};
enum socket_file_type
{
	socket_file_type_log = 0,
	socket_file_type_detection_result,
	socket_file_type_error_report,
};
struct socket_header_param
{
	socket_file_type fileType;
	char *fileName;
	int fileSize;
	const char *addPath;
};
struct socket_header
{
	int command;
	socket_header_param param;
	bool no_more_data;
};

bool winSock::requestRule(std::wstring *buffer)
{
	bool result = false;

	// 소켓
	SOCKET socket = ::WSASocketW(
		this->addrInfo->ai_family,
		this->addrInfo->ai_socktype,
		this->addrInfo->ai_protocol, nullptr, 0, WSA_FLAG_OVERLAPPED);
	if (socket == INVALID_SOCKET)
	{
		help->writeLog(logId::error, L"[%s:%03d] code[%d] WSASocketW is failed.", __FUNCTIONW__, __LINE__, ::WSAGetLastError());
		return this->initialized;
	}

	char *ruleDataA = nullptr;
	DWORD flags = -1;
	while (true)
	{
		// 연결
		if (::connect(socket, this->addrInfo->ai_addr, static_cast<int>(this->addrInfo->ai_addrlen)) == SOCKET_ERROR)
		{
			help->writeLog(logId::error, L"[%s:%03d] code[%d] connect is failed.", __FUNCTIONW__, __LINE__, ::WSAGetLastError());
			break;
		}

#pragma region request 전송
		// send - overlapped
		WSAEVENT events_send[1];
		events_send[0] = ::WSACreateEvent();
		WSAOVERLAPPED overlapped_send;
		::memset(&overlapped_send, 0x00, sizeof(WSAOVERLAPPED));
		overlapped_send.hEvent = events_send[0];

		// send - data
		socket_header data;
		::memset(&data, 0x00, sizeof(socket_header));
		data.command = socket_command::socket_command_request_rule;
		data.no_more_data = true;

		char *jsonData = nullptr;
		generate_socket_header(data, &jsonData, false);

		WSABUF send[1];
		::memset(send, 0x00, sizeof(WSABUF));
		send[0].len = ::strlen(jsonData);
		send[0].buf = jsonData;

		int err = 0;
		DWORD numberOfBytesSent = 0;
		err = ::WSASend(socket, send, _countof(send), &numberOfBytesSent, 0, &overlapped_send, nullptr);
		if ((err == SOCKET_ERROR) && (WSA_IO_PENDING != (err = ::WSAGetLastError())))
		{
			traceW(L"error [%s:%d] code[%d] WSASend is failed.\n", __FUNCTIONW__, __LINE__, err);
			errLog::getInstance()->write(error, L"[%s:%d] code[%d] WSASend is failed.", __FUNCTIONW__, __LINE__, err);
			break;
		}

		// 데이터 전송 끝났는지 확인
		err = ::WSAWaitForMultipleEvents(_countof(events_send), events_send, TRUE, WSA_INFINITE, FALSE);
		if (err == WSA_WAIT_FAILED)
		{
			traceW(L"error [%s:%d] code[%d] WSAWaitForMultipleEvents is failed.\n", __FUNCTIONW__, __LINE__, err);
			errLog::getInstance()->write(error, L"[%s:%d] code[%d]WSAWaitForMultipleEvents is failed.", __FUNCTIONW__, __LINE__, err);
			break;
		}

		// 실제로 전송된 바이트 수 확인
		err = ::WSAGetOverlappedResult(socket, &overlapped_send, &numberOfBytesSent, FALSE, &flags);
		if (err == FALSE)
		{
			err = ::WSAGetLastError();
			traceW(L"error [%s:%d] code[%d] WSAGetOverlappedResult is failed.\n", __FUNCTIONW__, __LINE__, err);
			errLog::getInstance()->write(error, L"[%s:%d] code[%d] WSAGetOverlappedResult is failed.", __FUNCTIONW__, __LINE__, err);
			break;
		}
#pragma endregion

#pragma region recv
		// recv - overlapped
		WSAEVENT events_recv[1];
		events_recv[0] = ::WSACreateEvent();
		WSAOVERLAPPED overlapped_recv;
		::memset(&overlapped_recv, 0x00, sizeof(WSAOVERLAPPED));
		overlapped_recv.hEvent = events_recv[0];

		// recv - data
		DWORD numberOfBytesReceived = 0;
		flags = 0;

		WSABUF recv_buffer[1];
		char buffer_header[2048];
		::memset(buffer_header, 0x00, sizeof(buffer_header));
		recv_buffer[0].len = _countof(buffer_header);
		recv_buffer[0].buf = buffer_header;

		bool recieved_header = false;
		while (true)
		{
			err = ::WSARecv(socket, recv_buffer, _countof(recv_buffer), &numberOfBytesReceived, &flags, &overlapped_recv, NULL);
			if ((err == SOCKET_ERROR) && (WSA_IO_PENDING != (err = ::WSAGetLastError())))
			{
				traceW(L"error [%s:%d] code[%d] WSARecv is failed.\n", __FUNCTIONW__, __LINE__, err);
				errLog::getInstance()->write(error, L"[%s:%d] code[%d] WSARecv is failed.", __FUNCTIONW__, __LINE__, err);
				break;
			}

			// 데이터 수신 끝났는지 확인
			err = ::WSAWaitForMultipleEvents(1, &overlapped_recv.hEvent, TRUE, INFINITE, TRUE);
			if (err == WSA_WAIT_FAILED)
			{
				traceW(L"error [%s:%d] code[%d] WSAWaitForMultipleEvents is failed.\n", __FUNCTIONW__, __LINE__, err);
				errLog::getInstance()->write(error, L"[%s:%d] code[%d] WSAWaitForMultipleEvents is failed.", __FUNCTIONW__, __LINE__, err);
				break;
			}

			// 실제로 수신된 바이트 수 확인
			err = ::WSAGetOverlappedResult(socket, &overlapped_recv, &numberOfBytesReceived, FALSE, &flags);
			if (err == FALSE)
			{
				err = ::WSAGetLastError();
				traceW(L"error [%s:%d] code[%d] WSAGetOverlappedResult is failed.\n", __FUNCTIONW__, __LINE__, err);
				errLog::getInstance()->write(error, L"[%s:%d] code[%d] WSAGetOverlappedResult is failed.", __FUNCTIONW__, __LINE__, err);
				break;
			}

			::WSAResetEvent(overlapped_recv.hEvent);

			////// TODO
			//////	: 지금은 버퍼크기보다 아래라서 처리할 필요가 없는데, 이거 realloc 으로 변경해서 더 커질경우를 대비해야함
			////traceA("debug [%s:%d] recv-data[%s]\n", __FUNCTION__, __LINE__, recv_buffer[0].buf);

			if ((recieved_header == false) && (::strlen(recv_buffer[0].buf) > 0))
			{
				jsonDocumentA recv_header;
				if (getJsonDocumentFromStringA(recv_buffer[0].buf, &recv_header) == false)
				{
					traceW(L"error [%s:%d] getJsonDocumentFromStringA is failed.\n", __FUNCTIONW__, __LINE__, err);
					errLog::getInstance()->write(error, L"[%s:%d] code[%d] WSAGetOverlappedResult is failed.", __FUNCTIONW__, __LINE__, err);
					break;
				}

				int command = -1;
				int status = -1;
				if ((recv_header.HasMember("command") == false) || (recv_header["command"].IsInt() == false))
				{
					traceW(L"error [%s:%d] invalid json data. [command]\n", __FUNCTIONW__, __LINE__);
					errLog::getInstance()->write(error, L"[%s:%d] invalid json data. [command]", __FUNCTIONW__, __LINE__);
					break;
				}
				else
				{
					command = recv_header["command"].GetInt();
				}

				if ((recv_header.HasMember("status") == false) || (recv_header["status"].IsInt() == false))
				{
					traceW(L"error [%s:%d] invalid json data. [command]\n", __FUNCTIONW__, __LINE__);
					errLog::getInstance()->write(error, L"[%s:%d] invalid json data. [command]", __FUNCTIONW__, __LINE__);
					break;
				}
				else
				{
					status = recv_header["status"].GetInt();
				}

				if ((command != socket_command_request_rule) && (status != socket_status_success))
				{
					traceW(L"error [%s:%d] invalid recv.\n", __FUNCTIONW__, __LINE__);
					errLog::getInstance()->write(error, L"[%s:%d] invalid recv.", __FUNCTIONW__, __LINE__);
					break;
				}

				recieved_header = true;
			}
			else
			{
				size_t length = ::strlen(recv_buffer[0].buf);
				length++;
				ruleDataA = (char*)::calloc(length, sizeof(char));
				::strcpy_s(ruleDataA, length, recv_buffer[0].buf);
			}

			if (numberOfBytesReceived == 0)
			{
				//traceA("debug [%s:%d] end of recv-data.\n", __FUNCTION__, __LINE__);
				break;
			}
		}
#pragma endregion

		result = ((ruleDataA != nullptr) ? (::strlen(ruleDataA) > 0) : false);

		break;
	}

	// 2020-11-20 orseL 
	//	: 서버쪽에서도 소켓끊음. 근데 내가먼저 끊어서 오류로 표시됨 (code: 'EPIPE', Error: This socket has been ended by the other party)
	::closesocket(socket);

	////// 1 : recv data (main)
	if (result == true)
	{
		size_t length = ::MultiByteToWideChar(CP_ACP, 0, ruleDataA, -1, nullptr, 0);
		length++;
		wchar_t *ruleDataW = (wchar_t*)::calloc(length, sizeof(wchar_t));
		::MultiByteToWideChar(CP_ACP, 0, ruleDataA, -1, ruleDataW, length);

		result = deserialize_rules_from_string(ruleDataW, buffer);

		// 정책파일로 생성
		if (result == true)
		{
			while (true)
			{
				// 파일 열기 (있으면 삭제하고 다시 만듬)
				std::wofstream rule;
				rule.open(rule_file_name, std::ios::trunc);
				if (rule.is_open() == false)
				{
					traceW(L"error [%s:%d] code[%d] rule.is_open() is failed.\n", __FUNCTIONW__, __LINE__, GetLastError());
					errLog::getInstance()->write(error, L"[%s:%d] rule.is_open() is failed.", __FUNCTIONW__, __LINE__, GetLastError());
					break;
				}

				rule << ruleDataW;

				// 파일 닫기
				rule.close();
				break;
			}
		}

		safeFree(ruleDataW);
	}

	safeFree(ruleDataA);

	errLog::getInstance()->write(info, L"%s to request rule from server.", ((result == true) ? L"succeeded" : L"failed"));

	return result;
}