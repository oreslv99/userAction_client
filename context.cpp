#include "context.h"
feature *context::fileIo = nullptr;

LRESULT CALLBACK context::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// file io watch �� ����
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
void context::setSocket(std::wstring ip, std::wstring port)
{
	if (this->socket == nullptr)
	{
		this->socket = new winSock(ip, port);
	}
}
bool context::initialize()
{
	// ���� Ȯ��
	if (this->socket->initialize() == false)
	{
		log->write(logId::warning, L"[%s:%03d] Failed to initialize winSock.", __FUNCTIONW__, __LINE__);
	}

	// ��å Ȯ��
	this->rule.initialize(this->socket, this->window);
	
	// ���ñ�� - �ڸ���� (** �ݵ�� ó�� ����Ʈ�� ���� **)
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

	// ���ñ�� - ���μ���
	feature *proc = new featureProcess();
	if (proc != nullptr)
	{
		if (proc->initialize(this->rule) == true)
		{
			this->features.push_back(proc);
		}
		else
		{
			safeDelete(proc);
		}
	}

	// ���ñ�� - ����Ʈ ���
	feature *prn = new featurePrint();
	if (prn != nullptr)
	{
		if (prn->initialize(this->rule) == true)
		{
			this->features.push_back(prn);
		}
		else
		{
			safeDelete(prn);
		}
	}

	// ���ñ�� - ����io
	feature *fileIo = new featureFileIo();
	if (fileIo != nullptr)
	{
		if (fileIo->initialize(this->rule) == true)
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

	// ���� watch Ÿ�̸� ����
	HANDLE watchTimer = ::CreateWaitableTimerW(nullptr, FALSE, nullptr);
	if (::SetWaitableTimer(watchTimer, &dueTime, this->rule.getTimerInterval(), nullptr, nullptr, FALSE) == FALSE)
	{
		log->write(logId::error, L"[%s:%03d] code[%d] SetWaitableTimer is failed.", __FUNCTIONW__, __LINE__, ::GetLastError());
		return -1;
	}

	// ���� �翬���� ��å�� �ִ°�쿡��
	HANDLE retryTimer = INVALID_HANDLE_VALUE;
	int serverRetryInterval = this->rule.getServerRetryInterval();
	if (serverRetryInterval > 0)
	{
		retryTimer = ::CreateWaitableTimerW(nullptr, FALSE, nullptr);
		if (::SetWaitableTimer(retryTimer, &dueTime, serverRetryInterval, nullptr, nullptr, FALSE) == FALSE)
		{
			log->write(logId::error, L"[%s:%03d] code[%d] SetWaitableTimer is failed.", __FUNCTIONW__, __LINE__, ::GetLastError());
			return -1;
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
	for (int i = 0; i < this->features.size(); i++)
	{
		safeDelete(this->features[i]);
	}
	this->features.clear();

	this->rule.release();

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
		bool inAFK = false;
		for (int i = 0; i < this->features.size(); i++)
		{
			if (i == 0)
			{
				inAFK = this->features[i]->watch();
			}
			else
			{
				if (inAFK == false)
				{
					this->features[i]->watch();
				}
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