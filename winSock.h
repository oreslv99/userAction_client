#pragma once
#include "stdafx.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

enum requestId
{
	rule,
	uploadUserData
};

class winSock
{
public:
	winSock(std::wstring ip, std::wstring port);
	~winSock();
	bool initialize();
	bool isOnline() const;
	bool request(requestId id, std::wstring *buffer);

private:
	std::wstring ip;
	std::wstring port;
	bool initialized;
	addrinfoW *addrInfo;

	struct headerData
	{
		int id;			// request id
		bool moreData;	// 다음 데이터가 존재
	};

	bool generateHeader(headerData header, bool newLine, std::string *buffer);
	bool requestRule(std::wstring *buffer);


};