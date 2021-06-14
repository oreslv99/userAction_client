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
	bool initialize(const rules &rule) final;
	bool watch() final;
	//featureType getFeatureType() final;

private:
	const ruleProcess *rule;

	void getProcessName(DWORD processId, std::wstring *processName, DWORD length);
	
	static BOOL CALLBACK wndEnumProc(HWND hwnd, LPARAM lParam); 
	void execScript(IHTMLDocument2 *);
	void getUrlFromIHTMLDocument(HWND window, std::wstring &content);
	void getUrlFromIAccessible();
	void getUrlFromUIAutomation();
	void getContents(bool isBrowser, HWND window, std::wstring processName);

};