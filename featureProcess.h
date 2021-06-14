#pragma once
#include "stdafx.h"
#include "feature.h"

class featureProcess : public feature
{
public:
	featureProcess();
	~featureProcess();
	bool initialize(const rules &rule) final;
	bool watch() final;
	//featureType getFeatureType() final;

private:
	const ruleProcess *rule;

	void getProcessName(DWORD processId, std::wstring *processName, DWORD length);
	
	static BOOL CALLBACK wndEnumProc(HWND hwnd, LPARAM lParam); 
	void getUrlFromIHTMLDocument(HWND window);
	void getUrlFromIAccessible();
	void getUrlFromUIAutomation();
	void getContents(bool isBrowser, HWND window, std::wstring processName);

};