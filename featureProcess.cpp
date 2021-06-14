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
void featureProcess::execScript(IHTMLDocument2 * iHtmlDocument2)
{
	// internal url Ȯ���ϱ� ���� javascript ����
	IHTMLWindow2 *iHtmlWindow2 = nullptr;
	if (SUCCEEDED(iHtmlDocument2->get_parentWindow(&iHtmlWindow2)))
	{
		CComPtr<IDispatch> spScript;
		iHtmlDocument2->get_Script(&spScript);
		//CComBSTR bstrMember(L"alert(document.getElementById('mainFrame').contentWindow.eval('location').href)");
		CComBSTR bstrMember(L"InternalUrl");
		DISPID dispid = NULL;
		CComVariant vaResult;
		BOOL bRes = FALSE;
		HRESULT res = spScript->GetIDsOfNames(IID_NULL, &bstrMember, 1, LOCALE_USER_DEFAULT, &dispid);
		if (SUCCEEDED(res))
		{
			//Putting parameters  
			DISPPARAMS dispparams;
			memset(&dispparams, 0, sizeof dispparams);
			dispparams.cArgs = 0;
			dispparams.rgvarg = new VARIANT[dispparams.cArgs];
			dispparams.cNamedArgs = 0;

			EXCEPINFO excepInfo;
			memset(&excepInfo, 0, sizeof excepInfo);
			UINT nArgErr = (UINT)-1;  // initialize to invalid arg

			//Call JavaScript function         
			res = spScript->Invoke(dispid, IID_NULL, 0, DISPATCH_METHOD, &dispparams, &vaResult, &excepInfo, &nArgErr);
			if (SUCCEEDED(res))
			{
				//Done!
				bRes = TRUE;
			}

			//Free mem
			delete[] dispparams.rgvarg;


			////CComBSTR script = L"";
			////CComBSTR language = L"javascript";
			////CComVariant var;
			////iHtmlWindow2->execScript(script, language, &var);
			////hresult = ::VariantChangeType(&var, &var, 0, VT_BSTR);
			//////ATL::CComDispatchDriver dispatchDriver = nullptr;
			//////iHtmlDocument2->QueryInterface(&dispatchDriver);
			//////dispatchDriver.Invoke1(L"eval", &CComVariant(script), &var);
			//////var.ChangeType(VT_BSTR);

			//help->writeLog(logId::debug, L"[%s:%03d] internal url: %s", __FUNCTIONW__, __LINE__, var.bstrVal);
		}
	}

}
void featureProcess::getUrlFromIHTMLDocument(HWND window, std::wstring &content)
{
	HWND hwndParent = reinterpret_cast<HWND>(::GetWindowLongPtrW(window, GWLP_HWNDPARENT));
	if (hwndParent == nullptr)
	{
		return;
	}

	// get window handle for "Internet Explore_Server" class name
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

	// get IHTMLDocument2 object
	LRESULT result = 0;
	result = ::SendMessageW(ieServer, WM_GETOBJECT_HTML, 0, 0);
	//::SendMessageTimeoutW(ieServer, WM_GETOBJECT_HTML, static_cast<WPARAM>(0), static_cast<LPARAM>(0), SMTO_ABORTIFHUNG, 1000, reinterpret_cast<PDWORD_PTR>(&result));

	IHTMLDocument2 *iHtmlDocument2 = nullptr;
	HRESULT hresult = ::ObjectFromLresult(result, IID_IHTMLDocument2, static_cast<WPARAM>(0), reinterpret_cast<void**>(&iHtmlDocument2));
	if (SUCCEEDED(hresult))
	{
		CComBSTR temp;

		execScript(iHtmlDocument2);

		//
		result = iHtmlDocument2->get_URL(&temp);
		if (SUCCEEDED(result))
		{
			content = temp;
		}

		//::SysReleaseString(temp);
		//temp = nullptr;
	}

	safeRelease(iHtmlDocument2);
}
void featureProcess::getContents(bool isBrowser, HWND window, std::wstring processName)
{
	std::wstring currentContent;
	DWORD length = 1024;
	currentContent.resize(length);

	if (isBrowser == true)
	{
		//if (processName.compare(std::wstring(L"iexplore.exe")) == 0)
		if (isMatch(processName.c_str(), L"iexplore.exe") == true)
		{
			// IHTMLDocuments2
			getUrlFromIHTMLDocument(window, currentContent);
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
		help->writeUserAction(featureId::process, L"%s\t%s", processName.c_str(), latestContent.c_str());
	}
}