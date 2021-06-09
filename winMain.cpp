#include "stdafx.h"
#include "application.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(pCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	log->write(errId::debug, L"Start application.");

	application app;
	if (app.initialize(hInstance) == false)
	{
		log->write(errId::error, L"Failed to initialize application.");
		return -1;
	}

	return app.run();
}