#pragma once
#include "stdafx.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

class winSock
{
public:
	winSock(std::wstring ip, std::wstring port);
	~winSock();
	bool initialize();

private:
	std::wstring ip;
	std::wstring port;
	addrinfoW *addrInfo;
	bool onLine;

};