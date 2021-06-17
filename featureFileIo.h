#pragma once
#include "stdafx.h"
#include "feature.h"
#include <functional>

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
	std::wstring getFileSize(std::wstring filePath)
	{
		std::ifstream stream(filePath, std::ios::ate | std::ios::binary);
		double fileSize = static_cast<double>(stream.tellg());

#define KB ((ULONGLONG)1024)
#define MB (KB*KB)
#define GB (KB*KB*KB)
#define TB (KB*KB*KB*KB)
#define PB (KB*KB*KB*KB*KB)

		static struct {
			LONGLONG dLimit;
			double dDivisor;
			double dNormaliser;
			int nDecimals;
			const wchar_t *wPrefix;
		} data[] = {
			{ 10 * KB, 10.24, 100.0, 2, L"%s KB" }, /* 10 KB */
			{ 100 * KB, 102.4, 10.0, 1, L"%s KB" }, /* 100 KB */
			{ 1000 * KB, 1024.0, 1.0, 0, L"%s KB" }, /* 1000 KB */
			{ 10 * MB, 10485.76, 100.0, 2, L"%s MB" }, /* 10 MB */
			{ 100 * MB, 104857.6, 10.0, 1, L"%s MB" }, /* 100 MB */
			{ 1000 * MB, 1048576.0, 1.0, 0, L"%s MB" }, /* 1000 MB */
			{ 10 * GB, 10737418.24, 100.0, 2, L"%s GB" }, /* 10 GB */
			{ 100 * GB, 107374182.4, 10.0, 1, L"%s GB" }, /* 100 GB */
			{ 1000 * GB, 1073741824.0, 1.0, 0, L"%s GB" }, /* 1000 GB */
			{ 10 * TB, 10485.76, 100.0, 2, L"%s TB" }, /* 10 TB */
			{ 100 * TB, 104857.6, 10.0, 1, L"%s TB" }, /* 100 TB */
			{ 1000 * TB, 1048576.0, 1.0, 0, L"%s TB" }, /* 1000 TB */
		};

		return L"";
	}

};