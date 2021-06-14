#include "stdafx.h"
#include "application.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(pCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	//::_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	//help->writeLog(logId::info, L"Start application.");

	application app;
	if (app.initialize(hInstance) == false)
	{
		help->writeLog(logId::error, L"Failed to initialize application.");
		return -1;
	}

	int result = app.run();
	//::_CrtDumpMemoryLeaks();

	return result;
}