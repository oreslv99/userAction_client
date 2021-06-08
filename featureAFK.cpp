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
		// �ش� ��ɻ�� ����
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

	// ������ �Է½ð� Ȯ��
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
			log->write(errId::user, L"out afk");
		}
	}
	else
	{
		if ((::GetTickCount() - getLastInputTime()) >= this->rule->in /* �ӽ� 3�� */)
		{
			startAfkTime = ::GetTickCount();

			// �ڸ���� ����
			result = true;
			log->write(errId::user, L"in afk");
		}
	}

	return result;
}
bool featureAFK::isHighPriority()
{
	// ���� ���� ȣ��Ǿ�� ��
	return true;
}