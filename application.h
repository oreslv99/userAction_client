#pragma once
#include "stdafx.h"
#include "context.h"

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
	std::wstring programName;
	context *appContext;

	bool isAlreadyRunning(std::wstring programName);
	bool createWindow(HINSTANCE instance, std::wstring programName, context *appContext);
	bool readEnvironmet(context *appContext);

};