#pragma once
#include "stdafx.h"
#include "rapidJson.h"
#include "tinyXml.h"
#include "winSock.h"
#include "rules.h"
#include "featureAFK.h"
#include "featurePrint.h"
#include "featureProcess.h"

class context
{
public:
	context();
	~context();
	WNDPROC getWndProc();
	void setWindow(HWND window);
	void setSocket(std::wstring ip, std::wstring port, int retryInterval);
	bool initialize();
	void tickTock();

private:
	// helper
	rapidJson *json;
	tinyXml *xml;

	// window
	WNDPROC callback;
	HWND window;
	void watch(HANDLE timer);

	// socket
	std::wstring ip;
	std::wstring port;
	int retryInterval;
	winSock *socket;
	bool isOnLine;
	void retryConnect(HANDLE timer);

	// watch
	std::list<feature*> features;
	std::list<feature*>::iterator iter;

};