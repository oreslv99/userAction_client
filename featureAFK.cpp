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
bool featureAFK::initialize(const rules &rule)
{
	this->rule = rule.getAFKRule();
	//if (this->rule->enabled == false)
	//{
	//	// �ش� ��ɻ�� ����
	//	log->write(logId::warning, L"[%s:%03d] Feature afk is disabled.", __FUNCTIONW__, __LINE__);
	//	return true;
	//}

	this->event = ::CreateEventW(nullptr, FALSE, FALSE, nullptr);
	if (this->event == INVALID_HANDLE_VALUE)
	{
		log->write(logId::warning, L"[%s:%03d] code[%d] CreateEventW is failed.", __FUNCTIONW__, __LINE__, ::GetLastError());
		return false;
	}

	return true;
}
bool featureAFK::watch()
{
	::Sleep(1);

	// ������ �Է½ð� Ȯ��
	std::function<DWORD()> getLastInputTime = []() -> DWORD
	{
		LASTINPUTINFO lastInputInfo;
		::memset(&lastInputInfo, 0x00, sizeof(LASTINPUTINFO));
		lastInputInfo.cbSize = sizeof(LASTINPUTINFO);

		if (::GetLastInputInfo(&lastInputInfo) == FALSE)
		{
			log->write(logId::error, L"[%s:%03d] err[%05d] GetLastInputInfo is failed.", __FUNCTIONW__, __LINE__, ::GetLastError());
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

		// �ڸ���� ���� �� �ٷ� Ǯ���� �ʰ� 5�� �Ŀ� ��������

		// non-blocking
		//	: event signal �� ���� non-blocking ���� ����
		//	: �����쿡 ���� �����ϴ� �α������� ���ع��� �ʰ� ������
		if (lastInputTime > startAfkTime + this->rule->awake)
		{
			::SetEvent(this->event);
		}

		if (::WaitForSingleObject(this->event, 1) == WAIT_OBJECT_0)
		{
			// �ڸ���� ����
			startAfkTime = ::GetTickCount();	// �ð� �ʱ�ȭ
			::ResetEvent(this->event);
			result = false;
			log->writeUserAction(L"out afk");
		}
	}
	else
	{
		if ((::GetTickCount() - getLastInputTime()) >= this->rule->in)
		{
			startAfkTime = ::GetTickCount();

			// �ڸ���� ����
			result = true;
			log->writeUserAction(L"in afk");
		}
	}

	return result;
}
//featureType featureAFK::getFeatureType()
//{
//	return featureType::afk;
//}