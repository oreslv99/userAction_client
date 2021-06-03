#include "context.h"

static LRESULT CALLBACK wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// file io watch �� ����
	return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

//
// public
//
context::context()
	:json(nullptr), xml(nullptr), socket(nullptr), isOnLine(false), callback(wndProc), afk(nullptr)
{
}
context::~context()
{
	safeDelete(this->socket);
	safeDelete(this->xml);
	safeDelete(this->json);
}
WNDPROC context::getWndProc()
{
	return this->callback;
}
void context::setWindow(HWND window)
{
	this->window = window;
}
void context::setSocket(std::wstring ip, std::wstring port, int retryInterval)
{
	this->ip = ip;
	this->port = port;
	this->retryInterval = retryInterval * 60 * 1000;	// ��
}
bool context::initialize()
{
	// parser Ȯ��

	// ���� Ȯ��
	this->socket = new winSock(this->ip, this->port);
	this->isOnLine = this->socket->initialize();

	// ��å Ȯ��
	loadRule(this->isOnLine, this->socket);

	//
	this->afk = new awayFromKeyboard();
	this->print = new printing();
	this->print->initialize();

	return true;
}
void context::tickTock()
{
	LARGE_INTEGER dueTime;
	dueTime.QuadPart = 0;

	// ���� watch Ÿ�̸� ����
	HANDLE watchTimer = ::CreateWaitableTimerW(nullptr, FALSE, nullptr);
	if (::SetWaitableTimer(watchTimer, &dueTime, 3000, nullptr, nullptr, FALSE) == FALSE) // �ӽ� 3000 ms
	{
		log->write(errId::error, L"[%s:%03d] code[%d] SetWaitableTimer is failed.", __FUNCTIONW__, __LINE__, ::GetLastError());
		return;
	}

	// ���� �翬���� ��å�� �ִ°�쿡��
	HANDLE retryTimer = INVALID_HANDLE_VALUE;
	if (this->retryInterval > 0)
	{
		retryTimer = ::CreateWaitableTimerW(nullptr, FALSE, nullptr);
		if (::SetWaitableTimer(retryTimer, &dueTime, 5000, nullptr, nullptr, FALSE) == FALSE) // �ӽ� 3000 ms
		{
			log->write(errId::error, L"[%s:%03d] code[%d] SetWaitableTimer is failed.", __FUNCTIONW__, __LINE__, ::GetLastError());
			return;
		}
	}

	// �޽��� ����
	while (true)
	{
		watch(watchTimer);

		if (retryTimer != INVALID_HANDLE_VALUE)
		{
			retryConnect(retryTimer);
		}
	}

	// release
	::CancelWaitableTimer(watchTimer);
	safeCloseHandle(watchTimer);
}

//
// private
//
void context::watch(HANDLE timer)
{
	if (::WaitForSingleObject(timer, 1) == WAIT_OBJECT_0)
	{
		if (this->afk->inAFK() == false) 
		{
			this->print->watch();
		}
	}
}
void context::retryConnect(HANDLE timer)
{
	if (::WaitForSingleObject(timer, 1) == WAIT_OBJECT_0)
	{
		if (this->isOnLine == false)
		{
			log->write(errId::info, L"[%s:%03d] retry connection", __FUNCTIONW__, __LINE__);
			this->isOnLine = this->socket->initialize();
		}
	}
}
void context::loadRule(bool isOnline, winSock *socket)
{
	bool result = false;
	if (isOnLine == true)
	{
		//socket->
	}
	else
	{

		// ini ���ϰ��
		// https://docs.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shgetknownfolderpath
		wchar_t *profile = nullptr;
		if (SUCCEEDED(::SHGetKnownFolderPath(FOLDERID_Profile, 0, nullptr, &profile)))
		{
			std::wstring ruleFilePath;
			ruleFilePath += profile;
			ruleFilePath += L"\\.userAction\\rule.json";

			// SHGetKnownFolderPath �� Ȯ���� wchar_t buffer �� CoTaskMemFree �� release
			safeCoTaskMemFree(profile);

			// ��å���� ������ ����
			FILE *file = nullptr;
			::_wfopen_s(&file, ruleFilePath.c_str(), L"r");
			if (file != nullptr)
			{
				::fclose(file);
			}
		}
	}

	if (result == false)
	{
		// ��� ���н� �⺻�� ����
	}
}