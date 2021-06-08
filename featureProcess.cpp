#include "featureProcess.h"

featureProcess::featureProcess()
	:rule(nullptr)
{
}
featureProcess::~featureProcess()
{
	safeDelete(this->rule);
}
bool featureProcess::initialize(void *value, DWORD size)
{
	if (size != sizeof(rules))
	{
		return false;
	}

	rules *rule = reinterpret_cast<rules*>(value);
	this->rule = rule->getProcessRule();
	if (this->rule->enabled == false)
	{
		// 해당 기능사용 안함
		log->write(errId::warning, L"[%s:%03d] Feature process is disabled.", __FUNCTIONW__, __LINE__);
		return true;
	}

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