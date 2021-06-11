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
	:window(nullptr), socket(nullptr), callback(wndProc), features(), iter()
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
	// parser Ȯ��

	// ���� Ȯ��
	if (this->socket->initialize() == false)
	{
		log->write(logId::warning, L"[%s:%03d] Failed to initialize winSock.", __FUNCTIONW__, __LINE__);
	}

	//// ��å Ȯ��
	//rules *rule = new rules;
	//rule->initialize(this->isOnLine, this->socket);
	//
	//// �ӽ� (��å����ü�� initialize ���� ���� �� - model)
	//feature *afk = new featureAFK();	// �ݵ�� ó�� ����Ʈ�� ����
	//if ((afk != nullptr) && (afk->initialize(rule, sizeof(rules)) == true))
	//{
	//	this->features.push_back(afk);
	//}
	//feature *proc = new featureProcess();
	//if ((proc != nullptr) && (proc->initialize(rule, sizeof(rules)) == true))
	//{
	//	this->features.push_back(proc);
	//}
	//feature *prn = new featurePrint();
	//if ((prn != nullptr) && (prn->initialize(rule, sizeof(rules)) == true))
	//{
	//	this->features.push_back(prn);
	//}
	//feature *fileIo = new featureFileIo();
	//if ((fileIo != nullptr) && (fileIo->initialize(this->window, sizeof(HWND)) == true))
	//{
	//	this->features.push_back(fileIo);
	//}

	return true;
}
int context::tickTock()
{
	LARGE_INTEGER dueTime;
	dueTime.QuadPart = 0;

	// ���� watch Ÿ�̸� ����
	HANDLE watchTimer = ::CreateWaitableTimerW(nullptr, FALSE, nullptr);
	if (::SetWaitableTimer(watchTimer, &dueTime, 3000, nullptr, nullptr, FALSE) == FALSE) // �ӽ� 3000 ms
	{
		log->write(logId::error, L"[%s:%03d] code[%d] SetWaitableTimer is failed.", __FUNCTIONW__, __LINE__, ::GetLastError());
		return -1;
	}

	// ���� �翬���� ��å�� �ִ°�쿡��
	HANDLE retryTimer = INVALID_HANDLE_VALUE;
	if (this->socket->getRetryInterval() > 0)
	{
		retryTimer = ::CreateWaitableTimerW(nullptr, FALSE, nullptr);
		if (::SetWaitableTimer(retryTimer, &dueTime, 5000, nullptr, nullptr, FALSE) == FALSE) // �ӽ� 3000 ms
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
		for (this->iter = this->features.begin(); this->iter != this->features.end(); this->iter++)
		{
			// �ڸ���� feature �� high priority
			if ((*this->iter)->isHighPriority() == true)
			{
				inAfk = (*this->iter)->watch();
			}
			
			// �ڸ���� ���� �ƴ� ��쿡�� ����
			if (inAfk == false) 
			{
				(*this->iter)->watch();
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