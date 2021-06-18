#pragma once
#include "stdafx.h"
#include "feature.h"

struct paramsFileIo
{
	HWND window;
	WPARAM wparam;
	LPARAM lparam;
};
const DWORD WM_FILE_IO = WM_USER + 1;

class featureFileIo : public feature
{
public:
	featureFileIo();
	~featureFileIo();
	bool initialize(void *rule, DWORD size) final;
	bool watch(void* parameters = nullptr) final;
	//featureType getFeatureType() final;

private:
	const ruleFileIo *rule;
	HANDLE event;
	ULONG id;
	std::list<std::pair<std::wstring, UINT>> watches;
	SHChangeNotifyEntry *entires;

	HRESULT getCurrentItem(PUIDLIST_RELATIVE *pidl, IShellFolder *current, REFIID riid, void **ppv);
	bool moveNext(PUIDLIST_RELATIVE *pidl, IShellFolder **current, IShellItem2 *item);
	void getItemName(IShellItem2 *item, std::wstring &itemName);
	std::wstring getFileSize(std::wstring filePath);

};