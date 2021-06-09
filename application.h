#pragma once
#include "stdafx.h"
#include "context.h"

class application
{
public:
	application();
	~application();
	bool initialize(HINSTANCE instance);
	int run();

private:
	context appContext;

	bool isAlreadyRunning(std::wstring programName);
	bool createWindow(HINSTANCE instance, std::wstring programName, context *appContext);
	bool readEnvironmet(context *appContext);

};