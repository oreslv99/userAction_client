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
		getProcessName(processId, &processName, length);
		if (::wcslen(processName.c_str()) <= 0)
		{
			// string �� ũ�⸦ MAX_PATH �� �����߱� ������, wstring.length() �� ���� �����Ͱ� ������ length �� �Ǿ����
			// ���� wcslen �� ���

			log->write(logId::warning, L"[%s:%03d] Invalid process name.", __FUNCTIONW__, __LINE__);
			break;
		}

		// ���� ���μ������� Ȯ��
		bool isExcluded = false;
		std::list<std::wstring>::iterator iter;
		for (iter = const_cast<ruleProcess*>(this->rule)->excludes.begin(); iter != this->rule->excludes.end(); iter++)
		{
			if (isMatch(processName.c_str(), (*iter).c_str()) == true)
			{
				// ���� ���μ���
				isExcluded = true;
				break;
			}
		}
		if (isExcluded == true)
		{
			break;
		}

		// Contents Ȯ��
		std::wstring currentContents;
		bool isBrowser = false;
		for (iter = const_cast<ruleProcess*>(this->rule)->browsers.begin(); iter != this->rule->browsers.end(); iter++)
		{
			if (isMatch(processName.c_str(), (*iter).c_str()) == true)
			{
				isBrowser = true;
				break;
			}
		}
		getContents(isBrowser, rootOwner, processName);

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
	HANDLE process = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
	if (process != INVALID_HANDLE_VALUE)
	{
		if (::QueryFullProcessImageNameW(process, 0, const_cast<wchar_t*>(processName->data()), &length) == FALSE)
		{
			log->write(logId::warning, L"[%s:%03d] code[%d] QueryFullProcessImageNameW is failed.", __FUNCTIONW__, __LINE__, ::GetLastError());
		}
		else
		{
			// Ȯ����� ������ ���μ��� �̸���
			*processName = processName->substr(processName->rfind('\\') + 1);

			// �ҹ���
			std::transform(processName->begin(), processName->end(), processName->begin(), ::tolower);
		}
	}

	safeCloseHandle(process);
}
void featureProcess::getContents(bool isBrowser, HWND window, std::wstring processName)
{
	std::wstring currentContent;
	DWORD length = 1024;
	currentContent.resize(length);

	if (isBrowser == true)
	{
		if (processName.compare(L"iexplore.exe") == 0)
		{
			// IHTMLDocuments2
		}
		else
		{
			// IAccessible
			// UIAutomation
		}
	}
	else
	{
		// ���� ���Ȱ �Ǵ� ��������� ���õ� caption �� ���� �� �ִ� ���μ����� ���� ����ó��
		bool isPrivate = false;
		std::list<std::wstring>::iterator iter;
		for (iter = const_cast<ruleProcess*>(this->rule)->privates.begin(); iter != this->rule->privates.end(); iter++)
		{
			if (isMatch(processName.c_str(), (*iter).c_str()) == true)
			{
				isPrivate = true;
				break;
			}
		}

		if (isPrivate == true)
		{
			currentContent = L"<private>";
		}
		else
		{
			::GetWindowTextW(window, const_cast<wchar_t*>(currentContent.data()), length);

			static bool isDuplicated = false;
			static std::wstring latestProcessName = L"";

			// caption �� �ǽð����� ��ȭ�ϴ� ���μ������� Ȯ��
			if (isDuplicated == false)
			{
				for (iter = const_cast<ruleProcess*>(this->rule)->duplicates.begin(); iter != this->rule->duplicates.begin(); iter++)
				{
					if (isMatch(processName.c_str(), (*iter).c_str()) == true)
					{
						isDuplicated = true;

						// ���� ���μ����� ����
						latestProcessName = processName;
						break;
					}
				}
			}
			else
			{
				if (latestProcessName.compare(processName) != 0)
				{
					isDuplicated = false;
				}
			}
		}
	}

	// ���� Ȯ�ε� content �� ���� content �� �ٸ���쿡 ���
	static std::wstring latestContent = L"";
	if (latestContent.compare(currentContent) != 0)
	{
		latestContent = currentContent;
		log->writeUserAction(L"process %s", latestContent.c_str());
	}
}