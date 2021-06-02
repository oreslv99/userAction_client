#include "printing.h"

const std::wstring EVENTVIEWR_PATH = L"Microsoft-Windows-PrintService/Operational";

//
// public
//
bool printing::initialize()
{
	return true;
}

//
// private
//
void printing::setRegistryKey(HKEY hive, std::wstring path)
{
	HKEY result = nullptr;
	::RegOpenKeyExW(hive, path.c_str(), 0, KEY_WRITE, &result);

	// 없으면 생성
	if (result == nullptr)
	{
		DWORD disposition = 0;
		::RegCreateKeyExW(hive, path.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &result, &disposition);
		if (result == nullptr)
		{
			return;
		}
	}

	::RegCloseKey(result);
}
void printing::getRegistryValue(HKEY hive, std::wstring subKey, std::wstring name, DWORD valueType, void* value, DWORD valueSize)
{
	HKEY result = nullptr;
	if (::RegOpenKeyExW(hive, subKey.c_str(), 0, KEY_QUERY_VALUE, &result) == ERROR_SUCCESS)
	{
		::RegQueryValueExW(result, name.c_str(), nullptr, &valueType, (BYTE*)value, &valueSize);
	}

	::RegCloseKey(result);
}