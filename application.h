#pragma once
#include "stdafx.h"

class application
{
public:
	application(HINSTANCE instance, std::wstring programName);
	~application();
	bool initialize();
	void run();
	int release();

private:
	HINSTANCE instance;
	std::wstring programName;

	bool isAlreadyRunning();

};