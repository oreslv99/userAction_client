#include "stdafx.h"
#include "application.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(pCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	application *app = new application(hInstance);
	if (app->initialize() == false) 
	{
		log->write(errId::error, L"Failed to initialize application.");
		return -1;
	}

	log->write(errId::debug, L"Start application.");
	app->run();

	return app->release();
}