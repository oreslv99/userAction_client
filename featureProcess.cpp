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
		getProcessName(processId, processName, length);
		if (processName.length() < 0)
		{
			// 대상 프로세스가 응답없음에 빠져있는 경우
			log->write(logId::warning, L"[%s:%03d] Invalid process name.", __FUNCTIONW__, __LINE__);
			break;
		}

		// 예외 프로세스인지 확인
		bool exclude = false;
		std::list<std::wstring>::iterator iter;
		for (iter = const_cast<ruleProcess*>(this->rule)->excludes.begin(); iter != this->rule->excludes.end(); iter++)
		{
			if (isMatch(processName.c_str(), (*iter).c_str()) == true)
			{
				// 예외 프로세스
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
			// 확장명을 포함한 프로세스 이름만
			processName = processName.substr(processName.rfind('\\') + 1);

			//// 소문자로
			//std::transform(processName.begin(), processName.end(), processName.begin(), ::tolower);
		}
	}

	safeCloseHandle(process);
}