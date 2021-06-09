#pragma once
#include "stdafx.h"
#include "rapidJson.h"
#include "tinyXml.h"
#include "winSock.h"
#include "rules.h"
#include "featureAFK.h"
#include "featureFileIo.h"
#include "featurePrint.h"
#include "featureProcess.h"

class context
{
public:
	context();
	~context();
	WNDPROC getWndProc();
	bool initialize(HWND window, std::wstring ip, std::wstring port, int retryInterval);
	int tickTock();

private:

	// window
	WNDPROC callback;
	HWND window;
	void watch(HANDLE timer);

	// socket
	winSock *socket;
	void retryConnect(HANDLE timer);

	// watch
	std::list<feature*> features;
	std::list<feature*>::iterator iter;

};