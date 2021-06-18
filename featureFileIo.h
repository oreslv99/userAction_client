#pragma once
#include "stdafx.h"
#include "feature.h"
#include <vector>

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

	//
	const ULONGLONG KB = static_cast<ULONGLONG>(1024);
	const ULONGLONG MB = KB * KB;
	const ULONGLONG GB = MB * KB;
	const ULONGLONG TB = GB * KB;
	const ULONGLONG PB = TB * KB;
	struct sizeTemplate
	{
		ULONGLONG limit;
		double divisor;
		double normalizer;
		std::wstring prefix;
	};
	std::vector<sizeTemplate> sizeTemplates = {
		{ KB * 10,		10.24,			100.0,	L"KB" },
		{ KB * 100,		102.4,			10.0,	L"KB" },
		{ KB * 1000,	1024.0,			1.0,	L"KB" },
		{ MB * 10,		10485.76,		100.0,	L"MB" },
		{ MB * 100,		104857.6,		10.0,	L"MB" },
		{ MB * 1000,	1048576.0,		1.0,	L"MB" },
		{ GB * 10,		10737418.24,	100.0,	L"GB" },
		{ GB * 100,		107374182.4,	10.0,	L"GB" },
		{ GB * 1000,	1073741824.0,	1.0,	L"GB" },
	};

	std::wstring getFileSize(std::wstring filePath)
	{
		std::ifstream stream(filePath, std::ios::ate | std::ios::binary);
		ULONGLONG fileSize = static_cast<ULONGLONG>(stream.tellg());

		// 파일이 막 생성되는 경우 ifstream tellg 가 실패하여 -1 리턴함
		if (fileSize < 0)
		{
			fileSize = 0;
		}

		// 1024 byte 가 안되는 경우, 그냥 1kb 로 표시
		if (fileSize < 1024)
		{
			return std::to_wstring(fileSize) + L"Byte";
		}
		
		// template 정보 확인
		int i;
		for (i = 0; i < this->sizeTemplates.size(); i++)
		{
			if (fileSize < this->sizeTemplates[i].limit)
			{
				break;
			}
		}

		// 윈도우처럼 표기하기 위해 template 에 정의한 값에 맞춰 계산
		double temp = static_cast<double>(fileSize);
		temp = ::floor(temp / this->sizeTemplates[i].divisor) / this->sizeTemplates[i].normalizer;

		std::wstring result;
		result = std::to_wstring(temp);
		result = result.substr(0, result.rfind('.') + 3);	// 소수점 두자리 표시
		result += this->sizeTemplates[i].prefix;

		return result;
	}

};