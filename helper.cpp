#include "helper.h"

std::wstring helper::path = {};
CRITICAL_SECTION helper::cs = {};
helper *helper::instance = nullptr;
const std::wstring FORMAT_DATE = L"%02d:%02d:%02d.%03d";
const std::wstring FORMAT_LOG = L"%s\\_err_%04d%02d%02d.log";
const std::wstring FORMAT_USER = L"%s\\_usr_%04d%02d%02d.log";

static std::map<int, std::wstring> logIds =
{
	makeString(debug),
	makeString(info),
	makeString(warning),
	makeString(error),
};
static std::map<int, std::wstring> featureIds =
{
	makeString(afkIn),
	makeString(afkAwk),
	makeString(file),
	makeString(devCon),
	makeString(devDcon),
	makeString(print),
	makeString(program),
	makeString(browser),
	makeString(logon),
	makeString(logoff),
};

//
// public
//
helper *helper::getInstance()
{
	// single-ton pattern
	if (instance == nullptr)
	{
		instance = new helper();
		if ((instance == nullptr) || (instance->initialize() == false))
		{
			safeDelete(instance);
			return nullptr;
		}
	}

	return instance;
}
bool helper::initialize()
{
	// writeLog 파일경로
	// https://docs.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shgetknownfolderpath
	wchar_t *profile = nullptr;
	if (FAILED(::SHGetKnownFolderPath(FOLDERID_Profile, 0, nullptr, &profile)))
	{
		return false;
	}

	// 경로 생성
	path += profile;
	path += L"\\.userAction";
	::CreateDirectoryW(path.c_str(), nullptr);	// C:\\users\\윈도우 계정명\\.userAction

	// SHGetKnownFolderPath 로 확인한 wchar_t buffer 는 CoTaskMemFree 로 release
	safeCoTaskMemFree(profile);

	// critical section 초기화
	::InitializeCriticalSection(&cs);

	return true;
}
void helper::release()
{
	::DeleteCriticalSection(&cs);
	safeDelete(instance);
}
void helper::writeLog(logId id, std::wstring message, ...)
{
	::EnterCriticalSection(&cs);

	// local 시간확인
	SYSTEMTIME localTime;
	::memset(&localTime, 0x00, sizeof(SYSTEMTIME));
	::GetLocalTime(&localTime);

	// 파일명 (path + \\%04d%02d%02d.writeLog)
	std::wstring filePath;
	filePath.resize(path.length() + FORMAT_LOG.length());
	::wsprintfW(const_cast<wchar_t*>(filePath.data()), FORMAT_LOG.c_str(), path.c_str(), localTime.wYear, localTime.wMonth, localTime.wDay);

	// 파일 열기 (utf-8 인코딩)
	std::wofstream stream;
	stream.imbue(std::locale(stream.getloc(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::consume_header>));
	stream.open(filePath, std::ios::app);
	if (stream.is_open() == true)
	{
		std::map<int, std::wstring>::iterator iter = logIds.find(id);
		if (iter != logIds.end())
		{

			// 가변인자 포함한 문자열 만들기
			va_list args;
			va_start(args, message);
			size_t length = 0;

			std::wstring buffer;
			length = ::_vscwprintf(message.c_str(), args);
			length++;
			buffer.resize(length);

			::vswprintf_s(const_cast<wchar_t*>(buffer.data()), length, message.c_str(), args);
			va_end(args);

			// 파일 기록
			std::wstring timestamp;
			timestamp.resize(12);
			::wsprintfW(const_cast<wchar_t*>(timestamp.data()), FORMAT_DATE.c_str(), localTime.wHour, localTime.wMinute, localTime.wSecond, localTime.wMilliseconds);
			stream << timestamp + L"\t";
			stream << iter->second + L"\t\t";
			stream << buffer;
			stream << std::endl;
		}

		stream.close();
	}

	::LeaveCriticalSection(&cs);
}
void helper::writeUserAction(featureId id, std::wstring message, ...)
{
	::EnterCriticalSection(&cs);

	// local 시간확인
	SYSTEMTIME localTime;
	::memset(&localTime, 0x00, sizeof(SYSTEMTIME));
	::GetLocalTime(&localTime);

	// 파일명 (path + \\%04d%02d%02d.writeLog)
	std::wstring filePath;
	filePath.resize(path.length() + FORMAT_USER.length());
	::wsprintfW(const_cast<wchar_t*>(filePath.data()), FORMAT_USER.c_str(), path.c_str(), localTime.wYear, localTime.wMonth, localTime.wDay);

	// 파일 열기 (utf-8 인코딩)
	std::wofstream stream;
	stream.imbue(std::locale(stream.getloc(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::consume_header>));
	stream.open(filePath, std::ios::app);
	if (stream.is_open() == true)
	{
		std::map<int, std::wstring>::iterator iter = featureIds.find(id);
		if (iter != featureIds.end())
		{
			// 가변인자 포함한 문자열 만들기
			va_list args;
			va_start(args, message);
			size_t length = 0;

			std::wstring buffer;
			length = ::_vscwprintf(message.c_str(), args);
			length++;
			buffer.resize(length);

			::vswprintf_s(const_cast<wchar_t*>(buffer.data()), length, message.c_str(), args);
			va_end(args);

			// 파일 기록
			std::wstring timestamp;
			timestamp.resize(12);
			::wsprintfW(const_cast<wchar_t*>(timestamp.data()), FORMAT_DATE.c_str(), localTime.wHour, localTime.wMinute, localTime.wSecond, localTime.wMilliseconds);
			stream << timestamp + L"\t";
			stream << iter->second + L"\t\t";
			stream << buffer;
			stream << std::endl;
			stream.close();
		}
	}

	::LeaveCriticalSection(&cs);
}
void helper::toLower(std::wstring &source)
{
	std::transform(source.begin(), source.end(), source.begin(), ::tolower);
}
bool helper::getJsonDocumentFromFile(const std::wstring filePath, jsonDocumentW *buffer)
{
	if ((filePath.length() < 0) || (buffer == nullptr))
	{
		help->writeLog(logId::error, L"[%s:%03d] Invalid parameter.", __FUNCTIONW__, __LINE__);
		return false;
	}

	// 파일 핸들
	std::wifstream fileStream(filePath);
	if (fileStream.is_open() == false)
	{
		help->writeLog(logId::error, L"[%s:%03d] Can not open: %s", __FUNCTIONW__, __LINE__, filePath.c_str());
		return false;
	}

	// 파일의 인코딩 타입을 utf8 로 설정
	fileStream.imbue(std::locale(fileStream.getloc(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::consume_header>));
	rapidjson::WIStreamWrapper streamWrapper(fileStream);
	buffer->ParseStream(streamWrapper);

	return true;
}
bool helper::getJsonDocumentFromString(const std::wstring jsonString, jsonDocumentW *buffer)
{
	if ((jsonString.length() < 0) || (buffer == nullptr))
	{
		help->writeLog(logId::error, L"[%s:%03d] Invalid parameter.", __FUNCTIONW__, __LINE__);
		return false;
	}

	// TODO: json 이 아닌경우 어떻게 되는지 확인
	buffer->Parse(jsonString.c_str());
	return buffer->IsObject();
}