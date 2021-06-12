#pragma once
#include "stdafx.h"
#include "winSock.h"
#include "rules.h"
#include "featureAFK.h"
#include "featureFileIo.h"
#include "featurePrint.h"
#include "featureProcess.h"
#include <algorithm>

class context
{
public:
	context();
	~context();
	WNDPROC getWndProc();
	void setWindow(HWND window);
	void setSocket(std::wstring ip, std::wstring port);
	bool initialize();
	int tickTock();

private:
	// rule
	rules rule;

	// window
	WNDPROC callback;
	HWND window;
	void watch(HANDLE timer);

	// socket
	winSock *socket;
	void retryConnect(HANDLE timer);

	// watch
	static feature *fileIo;
	static LRESULT CALLBACK wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	std::list<feature*> features;

};