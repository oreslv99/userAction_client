#pragma once
#include "stdafx.h"
#include "tinyXml.h"
#include <winevt.h>						// �̺�Ʈ ���
#pragma comment(lib, "wevtapi.lib")		// �̺�Ʈ ���

class printing {
public:
	printing();
	~printing();
	bool initialize();
	void watch();

private:
	// ������Ʈ��
	void setRegistryKey(HKEY hive, std::wstring path);
	void getRegistryValue(HKEY hive, std::wstring subKey, std::wstring name, DWORD valueType, void* value, DWORD valueSize);
	
	// �̺�Ʈ ������� �� ������ Ȯ��
	void parseDocument(tinyxml2::XMLDocument *document);
	void renderEvent(EVT_HANDLE fragment);
	void seekEvent(EVT_HANDLE queryResult);

};