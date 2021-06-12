#include "featureProcess.h"

featureProcess::featureProcess()
	:rule()
{
}
featureProcess::~featureProcess()
{
}
bool featureProcess::initialize(const rules &rule)
{
	this->rule = rule.getProcessRule();
	//if (size != sizeof(rules))
	//{
	//	return false;
	//}

	//rules *rule = reinterpret_cast<rules*>(value);
	//this->rule = rule->getProcessRule();
	//if (this->rule->enabled == false)
	//{
	//	// �ش� ��ɻ�� ����
	//	log->write(logId::warning, L"[%s:%03d] Feature process is disabled.", __FUNCTIONW__, __LINE__);
	//	return true;
	//}

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

		break;
	}

	return true;
}
//featureType featureProcess::getFeatureType()
//{
//	return featureType::process;
//}