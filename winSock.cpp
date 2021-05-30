#include "winSock.h"

winSock::winSock(std::wstring ip, std::wstring port)
	: addrInfo(nullptr), ip(ip), port(port)
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
	if ((this->ip.length() <= 0) || (this->port.length() <= 0))
	{
		log->write(errId::error, L"[%s:%03d] Invalid ip or port.", __FUNCTIONW__, __LINE__);
		return false;
	}

	int errorNo = -1;

	// 초기화
	WSADATA wsaData;
	errorNo = ::WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (errorNo != 0)
	{
		log->write(errId::error, L"[%s:%03d] code[%d] WSAStartup is failed.", __FUNCTIONW__, __LINE__, errorNo);
		return false;
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
		return false;
	}

	// 소켓
	SOCKET socket = ::WSASocketW(
		this->addrInfo->ai_family,
		this->addrInfo->ai_socktype,
		this->addrInfo->ai_protocol, nullptr, 0, WSA_FLAG_OVERLAPPED);
	if (socket == INVALID_SOCKET)
	{
		log->write(errId::error, L"[%s:%03d] code[%d] WSASocketW is failed.", __FUNCTIONW__, __LINE__, ::WSAGetLastError());
		return false;
	}

	// 연결
	if (::connect(socket, addrInfo->ai_addr, static_cast<int>(addrInfo->ai_addrlen)) == SOCKET_ERROR)
	{
		log->write(errId::error, L"[%s:%03d] code[%d] connect is failed.", __FUNCTIONW__, __LINE__, ::WSAGetLastError());
		return false;
	}

	::closesocket(socket);

	return true;
}