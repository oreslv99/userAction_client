#include "featureAFK.h"

//
// public
//
featureAFK::featureAFK()
	:rule(nullptr), event(INVALID_HANDLE_VALUE)
{
}
featureAFK::~featureAFK()
{
	safeCloseHandle(this->event);
}
bool featureAFK::initialize(void *rule, DWORD size)
{
	if (size != sizeof(ruleAFK))
	{
		help->writeLog(logId::error, L"[%s:%03d] Invalid parameter.", __FUNCTIONW__, __LINE__);
		return false;
	}

	this->rule = reinterpret_cast<ruleAFK*>(rule);
	this->event = ::CreateEventW(nullptr, FALSE, FALSE, nullptr);
	if (this->event == INVALID_HANDLE_VALUE)
	{
		help->writeLog(logId::warning, L"[%s:%03d] code[%d] CreateEventW is failed.", __FUNCTIONW__, __LINE__, ::GetLastError());
		return false;
	}

	return true;
}
bool featureAFK::watch(void* parameters)
{
	::Sleep(1);

	static bool result = false;
	if (this->rule->enabled == false)
	{
		return result;
	}

	// 마지막 입력시간 확인
	std::function<DWORD()> getLastInputTime = []() -> DWORD
	{
		LASTINPUTINFO lastInputInfo;
		::memset(&lastInputInfo, 0x00, sizeof(LASTINPUTINFO));
		lastInputInfo.cbSize = sizeof(LASTINPUTINFO);

		if (::GetLastInputInfo(&lastInputInfo) == FALSE)
		{
			help->writeLog(logId::error, L"[%s:%03d] err[%05d] GetLastInputInfo is failed.", __FUNCTIONW__, __LINE__, ::GetLastError());
			return 0;
		}

		return lastInputInfo.dwTime;
	};

	static DWORD startAfkTime = ::GetTickCount();

	SYSTEMTIME localTime;
	::memset(&localTime, 0x00, sizeof(SYSTEMTIME));
	::GetLocalTime(&localTime);

	if (result == true)
	{
		DWORD lastInputTime = getLastInputTime();

		// 자리비움에 빠진 뒤 바로 풀리지 않고 5초 후에 빠져나옴

		// non-blocking
		//	: event signal 을 통해 non-blocking 으로 만들어서
		//	: 스케쥴에 의해 동작하는 로그전송이 방해받지 않게 변경함
		if (lastInputTime > startAfkTime + this->rule->awake)
		{
			::SetEvent(this->event);
		}

		if (::WaitForSingleObject(this->event, 1) == WAIT_OBJECT_0)
		{
			help->writeUserAction(featureId::afkAwake, L"%02d:%02d:%02d (%dms)", 
				localTime.wHour, localTime.wMinute, localTime.wSecond, lastInputTime - (startAfkTime - this->rule->awake));

			// 자리비움 해제
			startAfkTime = ::GetTickCount();	// 시간 초기화
			::ResetEvent(this->event);
			result = false;
		}
	}
	else
	{
		if ((::GetTickCount() - getLastInputTime()) >= this->rule->in)
		{
			startAfkTime = ::GetTickCount();

			// 자리비움 상태
			result = true;
			help->writeUserAction(featureId::afkIn, L"%02d:%02d:%02d", localTime.wHour, localTime.wMinute, localTime.wSecond);
		}
	}

	return result;
}
//featureType featureAFK::getFeatureType()
//{
//	return featureType::afk;
//}