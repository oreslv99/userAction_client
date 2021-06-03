#pragma once
#include "stdafx.h"
#include "feature.h"
#include "rulePrint.h"
#include "tinyXml.h"
#include <winevt.h>						// �̺�Ʈ ���
#pragma comment(lib, "wevtapi.lib")		// �̺�Ʈ ���
#include <functional>

class featurePrint : public feature
{
public:
	featurePrint();
	~featurePrint();
	bool initialize(rules *rule) final;
	bool watch() final;
	bool isHighPriority() final;

private:
	rulePrint *rule;

	// ������Ʈ��
	void setRegistryKey(HKEY hive, std::wstring path);
	void getRegistryValue(HKEY hive, std::wstring subKey, std::wstring name, DWORD valueType, void* value, DWORD valueSize);
	
	// �̺�Ʈ ������� �� ������ Ȯ��
	void parseDocument(tinyxml2::XMLDocument *document);
	void renderEvent(EVT_HANDLE fragment);
	void seekEvent(EVT_HANDLE queryResult);

};