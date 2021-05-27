#include "application.h"

//
// static
//
LRESULT CALLBACK wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// file io watch 를 위함
	return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

//
// public
//
application::application(HINSTANCE instance)
	:instance(instance), window(nullptr), programName(), timer(INVALID_HANDLE_VALUE)
{
}
application::~application()
{
}
bool application::initialize()
{
	bool result = false;

	// 프로그램 이름
	size_t size = MAX_PATH;
	this->programName.resize(size);
	::GetModuleFileNameW(nullptr, const_cast<wchar_t*>(this->programName.data()), size);
	this->programName = this->programName.substr(this->programName.rfind('\\') + 1);	// userAction_client.exe

	// 현재 실행중이라면 종료
	if (isAlreadyRunning(this->programName) == true)
	{
		log->write(errId::error, L"[%s:%03d] Application is already running now.", __FUNCTIONW__, __LINE__);
		return false;
	}

	// window 생성
	if (createWindow(this->instance, this->programName, &this->window) == false)
	{
		log->write(errId::error, L"[%s:%03d] createWindow is Failed.", __FUNCTIONW__, __LINE__);
		return false;
	}

	// https://docs.microsoft.com/en-us/windows/win32/learnwin32/initializing-the-com-library
	//	: COINIT_DISABLE_OLE1DDE "OLE 1.0" 관련된 오버헤드를 줄일 수 있음 COINIT_APARTMENTTHREADED, COINIT_MULTITHREADED
	if (FAILED(::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
	{
		log->write(errId::error, L"[%s:%03d] err[%05d] CoInitializeEx is failed.", __FUNCTIONW__, __LINE__, ::GetLastError());
		return false;
	}

	// 정책 확인

	return true; 
}
void application::run() 
{
	// 타이머 설정
	LARGE_INTEGER dueTime;
	dueTime.QuadPart = 0;

	this->timer = ::CreateWaitableTimerW(nullptr, FALSE, nullptr);
	if (::SetWaitableTimer(this->timer, &dueTime, 2999, nullptr, nullptr, FALSE) == FALSE) // 임시 3000 ms
	{
		errLog::getInstance()->write(errId::error, L"[%s:%03d] code[%d] SetWaitableTimer is failed.", __FUNCTIONW__, __LINE__, ::GetLastError());
		return;
	}

	// 메시지 루프
	MSG message;
	while (::PeekMessageW(&message, nullptr, 0, 0, PM_NOREMOVE) == TRUE)
	{
		::TranslateMessage(&message);
		::DispatchMessageW(&message);

		if (message.message == WM_QUIT)
		{
			log->write(errId::info, L"[%s:%03d] message == wm_quit.", __FUNCTIONW__, __LINE__);
			break;
		}

		if (::WaitForSingleObject(this->timer, 1) == WAIT_OBJECT_0)
		{
			// do something
		}
	}
	
}
int application::release()
{
	if (this->timer != INVALID_HANDLE_VALUE)
	{
		::CancelWaitableTimer(this->timer);
		safeCloseHandle(this->timer);
	}

	::CoUninitialize();

	return 0;
}

//
// private
//
bool application::isAlreadyRunning(std::wstring programName)
{
	HANDLE mutex = ::CreateMutexW(nullptr, FALSE, programName.c_str());
	return ((::GetLastError() == ERROR_ALREADY_EXISTS) ? true : false);
}
bool application::createWindow(HINSTANCE instance, std::wstring programName, HWND *window)
{
	// 클래스 등록
	WNDCLASSW wndClass;
	::memset(&wndClass, 0x00, sizeof(WNDCLASSW));
	wndClass.hInstance = instance;
	wndClass.lpfnWndProc = wndProc;
	wndClass.lpszClassName = programName.c_str();
	if (::RegisterClassW(&wndClass) == 0)
	{
		log->write(errId::error, L"[%s:%03d] err[%05d] RegisterClassW is failed.", __FUNCTIONW__, __LINE__, ::GetLastError());
		return false;
	}

	// 윈도우 생성
	*window = ::CreateWindowExW(0, wndClass.lpszClassName, wndClass.lpszClassName, 0, 0, 0, 0, 0, nullptr, nullptr, wndClass.hInstance, nullptr);
	if (*window == nullptr)
	{
		log->write(errId::error, L"[%s:%03d] err[%05d] CreateWindowExW is failed.", __FUNCTIONW__, __LINE__, ::GetLastError());
		return false;
	}

	return true;
}