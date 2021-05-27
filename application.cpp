#include "application.h"

//
// public
//
application::application(HINSTANCE instance, std::wstring programName)
	:instance(instance), programName(programName)
{
}
application::~application()
{
}
bool application::initialize()
{
	bool result = false;

	// 현재 실행중이라면 종료
	if (isAlreadyRunning() == true) {
		return false;
	}

	return true; 
}
void application::run()
{
	log->write(errId::debug, L"write log test #%d", 0);
	log->write(errId::debug, L"write log test #%d", 1);
	log->write(errId::debug, L"write log test #%d", 2);
}
int application::release()
{
	return 0;
}

//
// private
//
bool application::isAlreadyRunning()
{
	HANDLE mutex = ::CreateMutexW(nullptr, FALSE, this->programName.c_str());
	return ((::GetLastError() == ERROR_ALREADY_EXISTS) ? true : false);
}