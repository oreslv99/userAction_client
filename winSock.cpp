#include "winSock.h"

winSock::winSock(std::wstring ip, std::wstring port, int retryInterval)
	: ip(ip), port(port), retryInterval(retryInterval), initialized(false), addrInfo(nullptr)
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
		log->write(errId::error, L"[%s:%03d] Invalid ip or port.", __FUNCTIONW__, __LINE__);
		return this->initialized;
	}

	int errorNo = -1;

	// 초기화
	WSADATA wsaData;
	errorNo = ::WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (errorNo != 0)
	{
		log->write(errId::error, L"[%s:%03d] code[%d] WSAStartup is failed.", __FUNCTIONW__, __LINE__, errorNo);
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
		log->write(errId::error, L"[%s:%03d] code[%d] GetAddrInfoW is failed.", __FUNCTIONW__, __LINE__, errorNo);
		return this->initialized;
	}

	// 소켓
	SOCKET socket = ::WSASocketW(
		this->addrInfo->ai_family,
		this->addrInfo->ai_socktype,
		this->addrInfo->ai_protocol, nullptr, 0, WSA_FLAG_OVERLAPPED);
	if (socket == INVALID_SOCKET)
	{
		log->write(errId::error, L"[%s:%03d] code[%d] WSASocketW is failed.", __FUNCTIONW__, __LINE__, ::WSAGetLastError());
		return this->initialized;
	}

	// 연결
	if (::connect(socket, addrInfo->ai_addr, static_cast<int>(addrInfo->ai_addrlen)) == SOCKET_ERROR)
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

		log->write(errId::error, L"[%s:%03d] %s", __FUNCTIONW__, __LINE__, errMessage.c_str());
		return this->initialized;
	}

	::closesocket(socket);

	return (this->initialized = true);
}
int winSock::getRetryInterval() const
{
	return this->retryInterval;
}
bool winSock::isOnline() const
{
	return this->initialized;
}