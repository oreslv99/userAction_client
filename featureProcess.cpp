#include "featureProcess.h"

const UINT WM_GETOBJECT_HTML = ::RegisterWindowMessageW(L"WM_HTML_GETOBJECT");

BOOL CALLBACK featureProcess::wndEnumProc(HWND hwnd, LPARAM lParam)
{
	std::wstring className;
	DWORD length = MAX_PATH;
	className.resize(length);
	::GetClassNameW(hwnd, const_cast<wchar_t*>(className.data()), length);
	help->toLower(className);

	if (::_wcsicmp(className.c_str(), L"Internet Explorer_Server") == 0)
	{
		*(HWND*)lParam = hwnd;
		return FALSE;
	}

	return TRUE;
}

//
// public
//
featureProcess::featureProcess()
	:rule()
{
}
featureProcess::~featureProcess()
{
}
bool featureProcess::initialize(void *rule, DWORD size)
{
	if (size != sizeof(ruleProcess))
	{
		help->writeLog(logId::error, L"[%s:%03d] Invalid parameter.", __FUNCTIONW__, __LINE__);
		return false;
	}

	this->rule = reinterpret_cast<ruleProcess*>(rule);
	return true;
}
bool featureProcess::watch(void* parameters)
{
	while (true)
	{
		::Sleep(1);

		// ���콺 ��ġ Ȯ��
		POINT mousePos;
		::GetCursorPos(&mousePos);

		// ���콺 �Ʒ� ������ �ڵ� Ȯ��
		HWND window = ::WindowFromPoint(mousePos);

		// root owner ������ �ڵ�
		HWND rootOwner = ::GetAncestor(window, GA_ROOTOWNER);

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
			// string �� ũ�⸦ MAX_PATH �� �����߱� ������, string�� �޼������ �������ϴ� ��찡 ����
			// trim �� �ϴ��� �ƴϸ� c �Լ��� ���
			help->writeLog(logId::warning, L"[%s:%03d] Invalid process name.", __FUNCTIONW__, __LINE__);
			break;
		}

		// ���� ���μ������� Ȯ��
		bool isExcluded = false;
		std::list<std::wstring>::const_iterator iter;
		for (iter = this->rule->excludes.begin(); iter != this->rule->excludes.end(); iter++)
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
		for (iter = this->rule->browsers.begin(); iter != this->rule->browsers.end(); iter++)
		{
			if (isMatch(processName.c_str(), (*iter).c_str()) == true)
			{
				isBrowser = true;
				break;
			}
		}
		getContents(isBrowser, window, processName);

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
			help->writeLog(logId::warning, L"[%s:%03d] code[%d] QueryFullProcessImageNameW is failed.", __FUNCTIONW__, __LINE__, ::GetLastError());
		}
		else
		{
			// Ȯ����� ������ ���μ��� �̸���
			*processName = processName->substr(processName->rfind('\\') + 1);
			help->toLower(*processName);
		}
	}

	safeCloseHandle(process);
}
// iexplore
void featureProcess::getUrlFromIHTMLDocument(HWND window, std::wstring &content)
{
	HWND hwndParent = reinterpret_cast<HWND>(::GetWindowLongPtrW(window, GWLP_HWNDPARENT));
	if (hwndParent == nullptr)
	{
		return;
	}

	// Internet Explore_Server Ŭ���� Ȯ��
	HWND ieServer = nullptr;
	::EnumChildWindows(hwndParent, wndEnumProc, reinterpret_cast<LPARAM>(&ieServer));
	if (ieServer == nullptr)
	{
		// ie �� admin ���� ����Ǵ� ���, Alternate Modal Top Most ��� ��ü Ŭ������ ����
		// �� process �� ���� (dialog) �Ǿ� ����Ǿ� owner �� �����
		// �Ʒ��� reversing ���� Ȯ���� ieframe.dll �� �ҽ� �ڵ忡�� original ieframe �� Ȯ���ϴ� ����� ����
		std::wstring className;
		DWORD length = MAX_PATH;
		className.resize(length);
		::GetClassNameW(window, const_cast<wchar_t*>(className.data()), length);
		help->toLower(className);

		help->writeLog(logId::debug, L"[%s:%03d] className: %s", __FUNCTIONW__, __LINE__, className.c_str());
		if (::_wcsicmp(className.c_str(), L"Alternate Modal Top Most") != 0)
		{
			return;
		}

		// Ghidra reversing ��� undocumented internal �Լ����� ������ ���� HWND Ȯ����
		window = reinterpret_cast<HWND>(::GetPropW(window, L"FakeModalPartnerFrame"));

		// �ٽ� Ȯ��
		::EnumChildWindows(window, wndEnumProc, (LPARAM)&ieServer);
	}

	// IHTMLDocument2 ��ü Ȯ��
	LRESULT result = 0;
	result = ::SendMessageW(ieServer, WM_GETOBJECT_HTML, 0, 0);
	//::SendMessageTimeoutW(ieServer, WM_GETOBJECT_HTML, static_cast<WPARAM>(0), static_cast<LPARAM>(0), SMTO_ABORTIFHUNG, 1000, reinterpret_cast<PDWORD_PTR>(&result));

	IHTMLDocument2 *iHtmlDocument2 = nullptr;
	HRESULT hresult = ::ObjectFromLresult(result, IID_IHTMLDocument2, static_cast<WPARAM>(0), reinterpret_cast<void**>(&iHtmlDocument2));
	if (SUCCEEDED(hresult))
	{
		BSTR temp;
		result = iHtmlDocument2->get_URL(&temp);
		if (SUCCEEDED(result))
		{
			content = temp;
		}
	}

	safeRelease(iHtmlDocument2);
}
// chromium
void featureProcess::getName(IAccessible *childrenAccessible, VARIANT childrentVariant, std::wstring &buffer)
{
	BSTR name = nullptr;
	if (SUCCEEDED(childrenAccessible->get_accName(childrentVariant, &name)) && (name != nullptr))
	{
		//buffer.assign(name);
		buffer = name;
	}

	if (name != nullptr)
	{
		::SysFreeString(name);
		name = nullptr;
	}

	// �׽�Ʈ
	buffer;
};
void featureProcess::getRole(IAccessible *childrenAccessible, VARIANT childrentVariant, long *buffer)
{
	VARIANT role;
	if (SUCCEEDED(childrenAccessible->get_accRole(childrentVariant, &role)))
	{
		//*buffer = role.lVal;
		::memcpy_s(buffer, sizeof(long), &role.lVal, sizeof(role.lVal));
	}

	::VariantClear(&role);
};
void featureProcess::getValue(IAccessible *childrenAccessible, VARIANT childrentVariant, std::wstring &buffer)
{
	BSTR value = nullptr;
	if (SUCCEEDED(childrenAccessible->get_accValue(childrentVariant, &value)) && (value != nullptr))
	{
		buffer = value;
	}

	if (value != nullptr)
	{
		::SysFreeString(value);
		value = nullptr;
	}
};
void featureProcess::getUrlRecursively(IAccessible *accessible, std::wstring &content)
{
	long countChildren = 0;
	HRESULT result = accessible->get_accChildCount(&countChildren);
	if ((FAILED(result)) || (countChildren <= 0))
	{
		return;
	}

	VARIANT	*variants = new VARIANT[countChildren];
	LONG obtained = 0;
	result = ::AccessibleChildren(accessible, 0, countChildren, variants, &obtained);
	if (FAILED(result))
	{
		safeDeleteArray(variants);
		help->writeLog(logId::error, L"[%s:%d] code[%08x] AccessibleChildren is failed.", __FUNCTIONW__, __LINE__, result);
		return;
	}

	for (LONG i = 0; i < obtained; i++)
	{
		if (::wcslen(content.c_str()) > 0)
		{
			break;
		}

		VARIANT childrenVariant = variants[i];

		long role = -1;
		std::wstring name;
		std::wstring value;
		IAccessible *accessibleChildren = nullptr;

		if (childrenVariant.vt == VT_DISPATCH)
		{
			IDispatch *dispatch = childrenVariant.pdispVal;
			result = dispatch->QueryInterface(IID_IAccessible, (void**)&accessibleChildren);
			if (FAILED(result))
			{
				help->writeLog(logId::error, L"[%s:%d] code[%08x] AccessibleChildren is failed.", __FUNCTIONW__, __LINE__, result);
				return;
			}

			VARIANT var;
			var.vt = VT_I4;
			var.intVal = CHILDID_SELF;

			getName(accessibleChildren, var, name);
			getRole(accessibleChildren, var, &role);

			// Chrome
			if (((::_wcsicmp(name.c_str(), L"�ּ�â �� �˻�â") == 0) || (::_wcsicmp(name.c_str(), L"address and search bar") == 0)) &&
				(role == ROLE_SYSTEM_TEXT))
			{
				getValue(accessibleChildren, var, value);
				if (::wcslen(value.c_str()) > 0)
				{
					content = value;
				}
			}

			// recursively
			if (::wcslen(content.c_str()) <= 0)
			{
				getUrlRecursively(accessibleChildren, content);
			}
		}

		safeRelease(accessibleChildren);
	}

	// release
	for (long i = 0; i < countChildren; i++)
	{
		::VariantClear(&variants[i]);
	}
	safeDeleteArray(variants);
}
void featureProcess::getUrlFromIAccessible(HWND window, std::wstring &content)
{
	HWND rootOwner = ::GetAncestor(window, GA_ROOTOWNER);
	IAccessible *accessible = nullptr;
	if (FAILED(::AccessibleObjectFromWindow(rootOwner, OBJID_CLIENT, IID_IAccessible, reinterpret_cast<void**>(&accessible))))
	{
		help->writeLog(logId::error, L"[%s:%03d] code[%d] AccessibleObjectFromWindow is failed.", __FUNCTIONW__, __LINE__, ::GetLastError());
		return;
	}

	getUrlRecursively(accessible, content);

	safeRelease(accessible);
}
void featureProcess::getContents(bool isBrowser, HWND window, std::wstring processName)
{
	std::wstring currentContent;
	DWORD length = 1024;
	currentContent.resize(length);

	featureId id;

	if (isBrowser == true)
	{
		id = featureId::browser;

		// iexplore �� �������� accessible �̳� uiautomation ���� 
		if (isMatch(processName.c_str(), L"iexplore.exe") == true)
		{
			// IHTMLDocuments2
			getUrlFromIHTMLDocument(window, currentContent);
		}
		else
		{
			// IAccessible
			getUrlFromIAccessible(window, currentContent);
		}
	}
	else
	{
		id = featureId::program;

		// ���� ���Ȱ �Ǵ� ��������� ���õ� caption �� ���� �� �ִ� ���μ����� ���� ����ó��
		bool isPrivate = false;
		std::list<std::wstring>::const_iterator iter;
		for (iter = this->rule->privates.begin(); iter != this->rule->privates.end(); iter++)
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
				for (iter = this->rule->duplicates.begin(); iter != this->rule->duplicates.begin(); iter++)
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
		help->writeUserAction(id, L"%s\t%s", processName.c_str(), latestContent.c_str());
	}
}