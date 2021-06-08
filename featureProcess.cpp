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
		// �ش� ��ɻ�� ����
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

		// ���콺 ��ġ Ȯ��
		POINT mousePos;
		::GetCursorPos(&mousePos);

		// ���콺 �Ʒ� ������ �ڵ� Ȯ��
		HWND original = ::WindowFromPoint(mousePos);

		// root owner ������ �ڵ�
		HWND rootOwner = ::GetAncestor(original, GA_ROOTOWNER);

		// pid Ȯ��
		DWORD processId;
		::GetWindowThreadProcessId(rootOwner, &processId);

		// ���� focus ������ Ȯ��
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