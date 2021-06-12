#include "context.h"
feature *context::fileIo = nullptr;

LRESULT CALLBACK context::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// file io watch 를 위함
	//fileIo->watch();
	return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

//
// public
//
context::context()
	:rule(), window(nullptr), socket(nullptr), callback(wndProc), features()
{
}
context::~context()
{
	safeDelete(this->socket);
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
	if (this->socket == nullptr)
	{
		this->socket = new winSock(ip, port, retryInterval);
	}
}
bool context::initialize()
{
	// 소켓 확인
	if (this->socket->initialize() == false)
	{
		log->write(logId::warning, L"[%s:%03d] Failed to initialize winSock.", __FUNCTIONW__, __LINE__);
	}

	// 정책 확인
	this->rule.initialize(this->socket, this->window);
	
	// 감시기능 - 자리비움 (** 반드시 처음 리스트에 포함 **)
	feature *afk = new featureAFK();
	if (afk != nullptr)
	{
		if (afk->initialize(this->rule) == true)
		{
			this->features.push_back(afk);
		}
		else
		{
			safeDelete(afk);
		}
	}

	// 감시기능 - 프로세스
	feature *proc = new featureProcess();
	if (proc != nullptr)
	{
		if (proc->initialize(rule) == true)
		{
			this->features.push_back(proc);
		}
		else
		{
			safeDelete(proc);
		}
	}

	// 감시기능 - 프린트 출력
	feature *prn = new featurePrint();
	if (prn != nullptr)
	{
		if (prn->initialize(rule) == true)
		{
			this->features.push_back(prn);
		}
		else
		{
			safeDelete(prn);
		}
	}

	// 감시기능 - 파일io
	feature *fileIo = new featureFileIo();
	if (fileIo != nullptr)
	{
		if (fileIo->initialize(rule) == true)
		{
			this->features.push_back(fileIo);
		}
		else
		{
			safeDelete(fileIo);
		}
	}

	return true;
}
int context::tickTock()
{
	LARGE_INTEGER dueTime;
	dueTime.QuadPart = 0;

	// 메인 watch 타이머 설정
	HANDLE watchTimer = ::CreateWaitableTimerW(nullptr, FALSE, nullptr);
	if (::SetWaitableTimer(watchTimer, &dueTime, 3000, nullptr, nullptr, FALSE) == FALSE) // 임시 3000 ms
	{
		log->write(logId::error, L"[%s:%03d] code[%d] SetWaitableTimer is failed.", __FUNCTIONW__, __LINE__, ::GetLastError());
		return -1;
	}

	// 소켓 재연결은 정책에 있는경우에만
	HANDLE retryTimer = INVALID_HANDLE_VALUE;
	if (this->socket->getRetryInterval() > 0)
	{
		retryTimer = ::CreateWaitableTimerW(nullptr, FALSE, nullptr);
		if (::SetWaitableTimer(retryTimer, &dueTime, 5000, nullptr, nullptr, FALSE) == FALSE) // 임시 3000 ms
		{
			log->write(logId::error, L"[%s:%03d] code[%d] SetWaitableTimer is failed.", __FUNCTIONW__, __LINE__, ::GetLastError());
			return -1;
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
	for (std::list<feature*>::iterator iter = this->features.begin(); iter != this->features.end(); iter++)
	{
	}

	::CancelWaitableTimer(retryTimer);
	::CancelWaitableTimer(watchTimer);
	safeCloseHandle(retryTimer);
	safeCloseHandle(watchTimer);
}

//
// private
//
void context::watch(HANDLE timer)
{
	if (::WaitForSingleObject(timer, 1) == WAIT_OBJECT_0)
	{
		bool inAfk = false;
		for (std::list<feature*>::iterator iter = this->features.begin(); iter != this->features.end(); iter++)
		{
			// 자리비움 feature 는 high priority
			if ((*iter)->isHighPriority() == true)
			{
				inAfk = (*iter)->watch();
			}
			
			// 자리비움 중이 아닌 경우에만 감시
			if (inAfk == false) 
			{
				(*iter)->watch();
			}
		}
	}
}
void context::retryConnect(HANDLE timer)
{
	if (::WaitForSingleObject(timer, 1) == WAIT_OBJECT_0)
	{
		if (this->socket->isOnline() == false)
		{
			if (this->socket->initialize() == true) 
			{
				log->write(logId::info, L"[%s:%03d] server is on line again.", __FUNCTIONW__, __LINE__);
			}
#if _DEBUG
			else
			{

				log->write(logId::warning, L"[%s:%03d] server is not on line.", __FUNCTIONW__, __LINE__);
			}
#endif
		}
	}
}