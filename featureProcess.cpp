#include "featureProcess.h"

featureProcess::featureProcess()
{}
featureProcess::~featureProcess()
{}
bool featureProcess::initialize(rule *featureRule)
{
	return true;
}
bool featureProcess::watch()
{
	while (true)
	{
		::Sleep(1);

		// 마우스 위치 확인
		POINT mousePos;
		::GetCursorPos(&mousePos);

		// 마우스 아래 윈도우 핸들 확인
		HWND original = ::WindowFromPoint(mousePos);

		// root owner 윈도우 핸들
		HWND rootOwner = ::GetAncestor(original, GA_ROOTOWNER);

		// pid 확인
		DWORD processId;
		::GetWindowThreadProcessId(rootOwner, &processId);

		// 현재 focus 중인지 확인
		DWORD foregroundProcessId;
		HWND foreground = ::GetForegroundWindow();
		::GetWindowThreadProcessId(foreground, &foregroundProcessId);
		if (processId != foregroundProcessId)
		{
			break;
		}
	}

	return true;
}
bool featureProcess::isHighPriority()
{
	return false;
}