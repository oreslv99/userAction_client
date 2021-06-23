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
	// writeLog ���ϰ��
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

	// SHGetKnownFolderPath �� Ȯ���� wchar_t buffer �� CoTaskMemFree �� release
	safeCoTaskMemFree(profile);

	// critical section �ʱ�ȭ
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

	// local �ð�Ȯ��
	SYSTEMTIME localTime;
	::memset(&localTime, 0x00, sizeof(SYSTEMTIME));
	::GetLocalTime(&localTime);

	// ���ϸ� (path + \\%04d%02d%02d.writeLog)
	std::wstring filePath;
	filePath.resize(path.length() + FORMAT_LOG.length());
	::wsprintfW(const_cast<wchar_t*>(filePath.data()), FORMAT_LOG.c_str(), path.c_str(), localTime.wYear, localTime.wMonth, localTime.wDay);

	// ���� ���� (utf-8 ���ڵ�)
	std::wofstream stream;
	stream.imbue(std::locale(stream.getloc(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::consume_header>));
	stream.open(filePath, std::ios::app);
	if (stream.is_open() == true)
	{
		std::map<int, std::wstring>::iterator iter = logIds.find(id);
		if (iter != logIds.end())
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

	// local �ð�Ȯ��
	SYSTEMTIME localTime;
	::memset(&localTime, 0x00, sizeof(SYSTEMTIME));
	::GetLocalTime(&localTime);

	// ���ϸ� (path + \\%04d%02d%02d.writeLog)
	std::wstring filePath;
	filePath.resize(path.length() + FORMAT_USER.length());
	::wsprintfW(const_cast<wchar_t*>(filePath.data()), FORMAT_USER.c_str(), path.c_str(), localTime.wYear, localTime.wMonth, localTime.wDay);

	// ���� ���� (utf-8 ���ڵ�)
	std::wofstream stream;
	stream.imbue(std::locale(stream.getloc(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::consume_header>));
	stream.open(filePath, std::ios::app);
	if (stream.is_open() == true)
	{
		std::map<int, std::wstring>::iterator iter = featureIds.find(id);
		if (iter != featureIds.end())
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

	// ���� �ڵ�
	std::wifstream fileStream(filePath);
	if (fileStream.is_open() == false)
	{
		help->writeLog(logId::error, L"[%s:%03d] Can not open: %s", __FUNCTIONW__, __LINE__, filePath.c_str());
		return false;
	}

	// ������ ���ڵ� Ÿ���� utf8 �� ����
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

	// TODO: json �� �ƴѰ�� ��� �Ǵ��� Ȯ��
	buffer->Parse(jsonString.c_str());
	return buffer->IsObject();
}