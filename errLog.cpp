#include "errLog.h"

std::wstring errLog::path = {};
CRITICAL_SECTION errLog::cs = {};
errLog *errLog::instance = nullptr;
const std::wstring DATE_FORMAT_ERROR = L"%02d:%02d:%02d.%03d";

#define makeString(x) { x, L#x }
static std::map<int, std::wstring> errIds =
{
	makeString(debug),
	makeString(info),
	makeString(warning),
	makeString(userAction),
	makeString(error),
};

//
// public
//
errLog *errLog::getInstance()
{
	// single-ton pattern
	if (instance == nullptr)
	{
		instance = new errLog();
		if ((instance == nullptr) || (instance->initialize() == false))
		{
			safeDelete(instance);
			return nullptr;
		}
	}

	return instance;
}
bool errLog::initialize()
{
	// errLog ���ϰ��
	// https://docs.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shgetknownfolderpath
	wchar_t *profile = nullptr;
	if (FAILED(::SHGetKnownFolderPath(FOLDERID_Profile, 0, nullptr, &profile)))
	{
		return false;
	}

	// ��� ����
	path += profile;
	path += L"\\.userAction";
	::CreateDirectoryW(path.c_str(), nullptr);	// C:\\users\\������ ������\\.userAction
	path += L"\\err";
	::CreateDirectoryW(path.c_str(), nullptr);	// C:\\users\\������ ������\\.userAction\\err

	// SHGetKnownFolderPath �� Ȯ���� wchar_t buffer �� CoTaskMemFree �� release
	safeCoTaskMemFree(profile);

	// critical section �ʱ�ȭ
	::InitializeCriticalSection(&cs);

	return true;
}
void errLog::release()
{
	::DeleteCriticalSection(&cs);
	safeDelete(instance);
}
void errLog::write(errId id, std::wstring message, ...)
{
	::EnterCriticalSection(&cs);

	// local �ð�Ȯ��
	SYSTEMTIME localTime;
	::memset(&localTime, 0x00, sizeof(SYSTEMTIME));
	::GetLocalTime(&localTime);

	// ���ϸ� (path + \\%04d%02d%02d.errLog)
	std::wstring filePath;
	filePath.resize(path.length() + 12);
	::wsprintfW(const_cast<wchar_t*>(filePath.data()), L"%s\\%04d%02d%02d.log", path.c_str(), localTime.wYear, localTime.wMonth, localTime.wDay);

	// ���� ����
	std::wofstream stream;
	stream.imbue(std::locale(stream.getloc(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::consume_header>));
	stream.open(filePath, std::ios::app);
	if (stream.is_open() == true)
	{
		std::map<int, std::wstring>::iterator iter = errIds.find(id);
		if (iter != errIds.end())
		{

			// �������� ������ ���ڿ� �����
			va_list args;
			va_start(args, message);
			size_t length = 0;

			std::wstring buffer;
			length = ::_vscwprintf(message.c_str(), args);
			length++;
			buffer.resize(length);

			::vswprintf_s(const_cast<wchar_t*>(buffer.data()), length, message.c_str(), args);
			va_end(args);

			// ���� ���
			std::wstring timestamp;
			timestamp.resize(12);
			::wsprintfW(const_cast<wchar_t*>(timestamp.data()), DATE_FORMAT_ERROR.c_str(), localTime.wHour, localTime.wMinute, localTime.wSecond, localTime.wMilliseconds);
			stream << timestamp + L"\t";
			stream << iter->second + L"\t\t";
			stream << buffer;
			stream << std::endl;
		}
	}

	::LeaveCriticalSection(&cs);
}