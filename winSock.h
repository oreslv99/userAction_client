#pragma once
#include "stdafx.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

class winSock
{
public:
	winSock(std::wstring ip, std::wstring port, int retryInterval);
	~winSock();
	bool initialize();
	int getRetryInterval() const;
	bool isOnline() const;

private:
	std::wstring ip;
	std::wstring port;
	int retryInterval;
	bool initialized;
	addrinfoW *addrInfo;

};