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
	safeDelete(this->rule);
}
bool featureAFK::initialize(void *rule, void *extra, DWORD extraSize)
{
	this->rule = reinterpret_cast<rules*>(rule)->getAFKRule();
	if (this->rule->enabled == false)
	{
		// 해당 기능사용 안함
		log->write(errId::warning, L"[%s:%03d] Feature afk is disabled.", __FUNCTIONW__, __LINE__);
		return true;
	}

	this->event = ::CreateEventW(nullptr, FALSE, FALSE, nullptr);
	if (this->event == INVALID_HANDLE_VALUE)
	{
		log->write(errId::warning, L"[%s:%03d] code[%d] CreateEventW is failed.", __FUNCTIONW__, __LINE__, ::GetLastError());
		return false;
	}

	return true;
}
bool featureAFK::watch()
{
	::Sleep(1);

	// 마지막 입력시간 확인
	std::function<DWORD()> getLastInputTime = []() -> DWORD
	{
		LASTINPUTINFO lastInputInfo;
		::memset(&lastInputInfo, 0x00, sizeof(LASTINPUTINFO));
		lastInputInfo.cbSize = sizeof(LASTINPUTINFO);

		if (::GetLastInputInfo(&lastInputInfo) == FALSE)
		{
			//traceW(L"fatal error [%s:%d]\n", __FUNCTIONW__, __LINE__);
			return 0;
		}

		return lastInputInfo.dwTime;
	};

	static bool result = false;
	static DWORD startAfkTime = ::GetTickCount();

	SYSTEMTIME localTime;
	::memset(&localTime, 0x00, sizeof(SYSTEMTIME));

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
			// 자리비움 해제
			startAfkTime = ::GetTickCount();	// 시간 초기화
			::ResetEvent(this->event);
			result = false;
			log->write(errId::user, L"out afk");
		}
	}
	else
	{
		if ((::GetTickCount() - getLastInputTime()) >= this->rule->in /* 임시 3초 */)
		{
			startAfkTime = ::GetTickCount();

			// 자리비움 상태
			result = true;
			log->write(errId::user, L"in afk");
		}
	}

	return result;
}
bool featureAFK::isHighPriority()
{
	// 가장 먼저 호출되어야 함
	return true;
}