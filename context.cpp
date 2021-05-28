#include "context.h"

static LRESULT CALLBACK wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// file io watch 를 위함
	return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

//
// public
//
context::context()
	:json(nullptr), xml(nullptr), server(nullptr), callback(wndProc)
{
}
context::~context()
{
	safeDelete(server);
	safeDelete(xml);
	safeDelete(json);
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
	this->retryInterval = retryInterval * 60 * 1000;	// 분
}
bool context::initialize()
{
	// parser 확인

	// 소켓 확인
	this->server = new winSock(this->ip, this->port);
	if (this->server->initialize() == false)
	{
		log->write(errId::warning, L"[%s:%03d] server is off-line.", __FUNCTIONW__, __LINE__);
	}
	else
	{

	}
	// 정책 확인

	return true;
}
void context::tickTock()
{
	LARGE_INTEGER dueTime;
	dueTime.QuadPart = 0;

	// 메인 watch 타이머 설정
	HANDLE watchTimer = ::CreateWaitableTimerW(nullptr, FALSE, nullptr);
	if (::SetWaitableTimer(watchTimer, &dueTime, 3000, nullptr, nullptr, FALSE) == FALSE) // 임시 3000 ms
	{
		log->write(errId::error, L"[%s:%03d] code[%d] SetWaitableTimer is failed.", __FUNCTIONW__, __LINE__, ::GetLastError());
		return;
	}

	// 소켓 재연결은 정책에 있는경우에만
	HANDLE retryTimer = INVALID_HANDLE_VALUE;
	if (this->retryInterval > 0)
	{
		retryTimer = ::CreateWaitableTimerW(nullptr, FALSE, nullptr);
		if (::SetWaitableTimer(retryTimer, &dueTime, 5000, nullptr, nullptr, FALSE) == FALSE) // 임시 3000 ms
		{
			log->write(errId::error, L"[%s:%03d] code[%d] SetWaitableTimer is failed.", __FUNCTIONW__, __LINE__, ::GetLastError());
			return;
		}
	}

	// 메시지 루프
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
		



		log->write(errId::info, L"[%s:%03d] do something", __FUNCTIONW__, __LINE__);
	}
}
void context::retryConnect(HANDLE timer)
{
	if (::WaitForSingleObject(timer, 1) == WAIT_OBJECT_0)
	{
		log->write(errId::info, L"[%s:%03d] retry connection", __FUNCTIONW__, __LINE__);
	}
}