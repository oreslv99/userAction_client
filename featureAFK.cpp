#include "featureAFK.h"

const ULONGLONG KB = static_cast<ULONGLONG>(1024);
const ULONGLONG MB = KB * KB;
const ULONGLONG GB = MB * KB;
const ULONGLONG TB = GB * KB;
const ULONGLONG PB = TB * KB;
struct sizeTemplate
{
	ULONGLONG limit;
	double divisor;
	double normalizer;
	std::wstring prefix;
};
const std::vector<sizeTemplate> sizeTemplates = {
	{ KB * 10,		10.24,			100.0,	L"KB" },
	{ KB * 100,		102.4,			10.0,	L"KB" },
	{ KB * 1000,	1024.0,			1.0,	L"KB" },
	{ MB * 10,		10485.76,		100.0,	L"MB" },
	{ MB * 100,		104857.6,		10.0,	L"MB" },
	{ MB * 1000,	1048576.0,		1.0,	L"MB" },
	{ GB * 10,		10737418.24,	100.0,	L"GB" },
	{ GB * 100,		107374182.4,	10.0,	L"GB" },
	{ GB * 1000,	1073741824.0,	1.0,	L"GB" },
};

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

	// ������ �Է½ð� Ȯ��
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
			help->writeUserAction(featureId::afkAwake, L"%02d:%02d:%02d (%dms)", 
				localTime.wHour, localTime.wMinute, localTime.wSecond, lastInputTime - (startAfkTime - this->rule->awake));

			// �ڸ���� ����
			startAfkTime = ::GetTickCount();	// �ð� �ʱ�ȭ
			::ResetEvent(this->event);
			result = false;
		}
	}
	else
	{
		if ((::GetTickCount() - getLastInputTime()) >= this->rule->in)
		{
			startAfkTime = ::GetTickCount();

			// �ڸ���� ����
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