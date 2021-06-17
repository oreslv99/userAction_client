#pragma once
#include "stdafx.h"
#include "feature.h"
#include <Mshtml.h>		// IHTMLDocument2
#include <OleAcc.h>		// ObjectFromLresult, IAccessible
#pragma comment (lib, "Oleacc.lib")

#include <atlbase.h>

class featureProcess : public feature
{
public:
	featureProcess();
	~featureProcess();
	bool initialize(void *rule, DWORD size) final;
	bool watch(void* parameters = nullptr) final;
	//featureType getFeatureType() final;

private:
	const ruleProcess *rule;

	void getProcessName(DWORD processId, std::wstring *processName, DWORD length);
	
	static BOOL CALLBACK wndEnumProc(HWND hwnd, LPARAM lParam); 
	// iexplore
	void getUrlFromIHTMLDocument(HWND window, std::wstring &content);
	// chromium
	void getName(IAccessible *childrenAccessible, VARIANT childrentVariant, std::wstring &buffer);
	void getRole(IAccessible *childrenAccessible, VARIANT childrentVariant, long *buffer);
	void getValue(IAccessible *childrenAccessible, VARIANT childrentVariant, std::wstring &buffer);
	void getUrlRecursively(IAccessible *accessible, std::wstring &content);
	void getUrlFromIAccessible(HWND window, std::wstring &content);
	//void getUrlFromUIAutomation(HWND window, std::wstring &content);
	void getContents(bool isBrowser, HWND window, std::wstring processName);

};