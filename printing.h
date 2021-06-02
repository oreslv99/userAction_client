#pragma once
#include "stdafx.h"

class printing {
public:
	printing();
	~printing();
	bool initialize();
	void check();

private:
	void setRegistryKey(HKEY hive, std::wstring path);
	void getRegistryValue(HKEY hive, std::wstring subKey, std::wstring name, DWORD valueType, void* value, DWORD valueSize);

};