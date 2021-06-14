#include "stdafx.h"
#include "application.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(pCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	help->writeLog(logId::debug, L"Start application.");

	application app;
	if (app.initialize(hInstance) == false)
	{
		help->writeLog(logId::error, L"Failed to initialize application.");
		return -1;
	}

	return app.run();
}