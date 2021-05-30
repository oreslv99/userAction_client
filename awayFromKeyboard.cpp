#include "awayFromKeyboard.h"

//
// public
//
awayFromKeyboard::awayFromKeyboard()
{
	this->event = ::CreateEventW(nullptr, FALSE, FALSE, nullptr);
}
awayFromKeyboard::~awayFromKeyboard()
{}
bool awayFromKeyboard::inAFK()
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
		if (lastInputTime > startAfkTime + 3000)
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
		if ((::GetTickCount() - getLastInputTime()) >= 3000 /* �ӽ� 3�� */)
		{
			startAfkTime = ::GetTickCount();

			// �ڸ���� ����
			result = true;
			log->write(errId::user, L"in afk");
		}
	}

	return result;
}