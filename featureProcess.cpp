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

		// ���μ��� �̸�
		std::wstring processName;
		DWORD length = MAX_PATH;
		processName.resize(length);
		getProcessName(processId, processName, length);
		if (processName.length() < 0)
		{
			// ��� ���μ����� ��������� �����ִ� ���
			log->write(logId::warning, L"[%s:%03d] Invalid process name.", __FUNCTIONW__, __LINE__);
			break;
		}

		// ���� ���μ������� Ȯ��
		bool exclude = false;
		std::list<std::wstring>::iterator iter;
		for (iter = const_cast<ruleProcess*>(this->rule)->excludes.begin(); iter != this->rule->excludes.end(); iter++)
		{
			if (isMatch(processName.c_str(), (*iter).c_str()) == true)
			{
				// ���� ���μ���
				exclude = true;
				break;
			}
		}
		if (exclude == true)
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

//
// private
//
void featureProcess::getProcessName(DWORD processId, std::wstring *processName, DWORD length)
{
	processName = L"";

	HANDLE process = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
	if (process != INVALID_HANDLE_VALUE)
	{
		//std::wstring buf;
		//buf.resize(length);
		if (::QueryFullProcessImageNameW(process, 0, const_cast<wchar_t*>(processName.data()), &length) == FALSE)
		{
			log->write(logId::warning, L"[%s:%03d] code[%d] QueryFullProcessImageNameW is failed.", __FUNCTIONW__, __LINE__, ::GetLastError());
		}
		else
		{
			//buf = buf.substr(buf.rfind('\\') + 1);
			processName.rfind('\\');
			// Ȯ����� ������ ���μ��� �̸���
			processName = processName.substr(processName.rfind('\\') + 1);

			//// �ҹ��ڷ�
			//std::transform(processName.begin(), processName.end(), processName.begin(), ::tolower);
		}
	}

	safeCloseHandle(process);
}