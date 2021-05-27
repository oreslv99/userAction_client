#pragma once
#include "stdafx.h"

class application
{
public:
	application(HINSTANCE instance);
	~application();
	bool initialize();
	void run();
	int release();

private:
	HINSTANCE instance;
	HWND window;
	std::wstring programName;
	HANDLE timer;

	bool isAlreadyRunning(std::wstring programName);
	bool createWindow(HINSTANCE instance, std::wstring programName, HWND *window);

};