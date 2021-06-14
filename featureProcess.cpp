#include "featureProcess.h"


BOOL CALLBACK featureProcess::wndEnumProc(HWND hwnd, LPARAM lParam)
{
	std::wstring className;
	DWORD length = MAX_PATH;
	className.resize(length);

	::GetClassNameW(hwnd, const_cast<wchar_t*>(className.data()), length);
	if (className.compare(L"Internet Explorer_Server") == 0)
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

		// 프로세스 이름
		std::wstring processName;
		DWORD length = MAX_PATH;
		processName.resize(length);
		getProcessName(processId, &processName, length);
		if (::wcslen(processName.c_str()) <= 0)
		{
			// string 의 크기를 MAX_PATH 로 지정했기 때문에, wstring.length() 는 실제 데이터가 없더라도 length 가 되어버림
			// 따라서 wcslen 을 사용

			help->writeLog(logId::warning, L"[%s:%03d] Invalid process name.", __FUNCTIONW__, __LINE__);
			break;
		}

		// 예외 프로세스인지 확인
		bool isExcluded = false;
		std::list<std::wstring>::iterator iter;
		for (iter = const_cast<ruleProcess*>(this->rule)->excludes.begin(); iter != this->rule->excludes.end(); iter++)
		{
			if (isMatch(processName.c_str(), (*iter).c_str()) == true)
			{
				// 예외 프로세스
				isExcluded = true;
				break;
			}
		}
		if (isExcluded == true)
		{
			break;
		}

		// Contents 확인
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
			help->writeLog(logId::warning, L"[%s:%03d] code[%d] QueryFullProcessImageNameW is failed.", __FUNCTIONW__, __LINE__, ::GetLastError());
		}
		else
		{
			// 확장명을 포함한 프로세스 이름만
			*processName = processName->substr(processName->rfind('\\') + 1);

			// 소문자
			std::transform(processName->begin(), processName->end(), processName->begin(), ::tolower);
		}
	}

	safeCloseHandle(process);
}
void featureProcess::getUrlFromIHTMLDocument(HWND window)
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
		return;
	}

	//// get IHTMLDocument2 object
	//LRESULT result = 0;
	//result = ::SendMessageW(ieServer, WM_GETOBJECT_HTML, 0, 0);
	////::SendMessageTimeoutW(ieServer, WM_GETOBJECT_HTML, static_cast<WPARAM>(0), static_cast<LPARAM>(0), SMTO_ABORTIFHUNG, 1000, reinterpret_cast<PDWORD_PTR>(&result));

	////ATL::CComPtr<IHTMLDocument2> iHTMLDocument2;
	//IHTMLDocument2 *iHTMLDocument2;
	//if (SUCCEEDED(::ObjectFromLresult(result, IID_IHTMLDocument2, static_cast<WPARAM>(0), reinterpret_cast<void**>(&iHTMLDocument2))))
	//{
	//	//ATL::CComBSTR url;
	//	BSTR url;
	//	if (SUCCEEDED(iHTMLDocument2->get_URL(&url)))
	//	{
	//		::wcsncpy_s(buffer, length, url, _TRUNCATE);
	//	}

	//	::SysReleaseString(url);
	//	url = nullptr;
	//}

	//iHTMLDocument2->Release();
	//iHTMLDocument2 = nullptr;
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
		// 개인 사생활 또는 비밀유지에 관련된 caption 을 갖을 수 있는 프로세스에 대한 예외처리
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

			// caption 이 실시간으로 변화하는 프로세스인지 확인
			if (isDuplicated == false)
			{
				for (iter = const_cast<ruleProcess*>(this->rule)->duplicates.begin(); iter != this->rule->duplicates.begin(); iter++)
				{
					if (isMatch(processName.c_str(), (*iter).c_str()) == true)
					{
						isDuplicated = true;

						// 현재 프로세스를 저장
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

	// 이전 확인된 content 가 현재 content 와 다른경우에 기록
	static std::wstring latestContent = L"";
	if (latestContent.compare(currentContent) != 0)
	{
		latestContent = currentContent;
		help->writeUserAction(L"process %s", latestContent.c_str());
	}
}