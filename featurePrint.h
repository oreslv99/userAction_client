#pragma once
#include "stdafx.h"
#include "feature.h"
#include ".\\extern\\tinyxml2\\tinyxml2.h"
#include <winevt.h>						// 이벤트 뷰어
#pragma comment(lib, "wevtapi.lib")		// 이벤트 뷰어
#include <functional>

class featurePrint : public feature
{
public:
	featurePrint();
	~featurePrint();
	bool initialize(const rules rule) final;
	bool watch() final;
	//featureType getFeatureType() final;

private:
	const rulePrint *rule;

	// 레지스트리
	void setRegistryKey(HKEY hive, std::wstring path);
	void getRegistryValue(HKEY hive, std::wstring subKey, std::wstring name, DWORD valueType, void* value, DWORD valueSize);
	
	// 이벤트 뷰어접근 및 데이터 확인
	void parseDocument(tinyxml2::XMLDocument *document);
	void renderEvent(EVT_HANDLE fragment);
	void seekEvent(EVT_HANDLE queryResult);

};