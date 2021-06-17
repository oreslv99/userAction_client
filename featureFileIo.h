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
	bool initialize(const rules &rule) final;
	bool watch(void* parameters = nullptr) final;
	//featureType getFeatureType() final;

private:
	const ruleFileIo *rule;
	HANDLE event;
	ULONG id;
	std::list<std::pair<std::wstring, UINT>> watches;
	SHChangeNotifyEntry *entires;

	HRESULT getCurrentItem(PUIDLIST_RELATIVE *pidl, IShellFolder *current, REFIID riid, void **ppv)
	{
		// IShellItem2 인스턴스 (riid, ppv) 의 메서드인 GetDisplayName + SIGDN_FILESYSPATH 호출할 예정
		//	>> 전체경로 (상위경로 + display 이름) 생성하기 위함
		//	>> SHCreateItemWithParent 을 이용해 pidl 에 상위경로와 display 이름을 갖는 shell item 을 생성

		*ppv = nullptr;
		
		// 다음 pidl
		PUIDLIST_RELATIVE next = ILNext(*pidl);
		
		// pidl child 생성하기 위해 이전 값 잠시 저장 및 초기화
		USHORT latest = next->mkid.cb;
		next->mkid.cb = 0;             

		// pidl child 생성과 기존 값 부여
		HRESULT result = ::SHCreateItemWithParent(nullptr, current, reinterpret_cast<PCUITEMID_CHILD>(*pidl), riid, ppv);
		next->mkid.cb = latest;

		return result;
	}
	bool moveNext(PUIDLIST_RELATIVE *pidl, IShellFolder **current, IShellItem2 *item)
	{
		bool result = false;
		if (*pidl == nullptr)
		{
			// item 으로부터 pidl 확인
			PIDLIST_ABSOLUTE pidlAbs;
			if (SUCCEEDED(::SHGetIDListFromObject(item, &pidlAbs)))
			{
				*pidl = pidlAbs;
				result = true;
			}
		}
		else if (ILIsEmpty(*pidl) == false)
		{
			// pidl 이 비어있지 않음

			PCUITEMID_CHILD child = reinterpret_cast<PCUITEMID_CHILD>(*pidl);  // save the current segment for binding
			*pidl = ILNext(*pidl);

			// if we are not at the end setup for the next itteration
			if (ILIsEmpty(*pidl) == false)
			{
				USHORT latest = (*pidl)->mkid.cb;  // avoid cloning for the child by truncating temporarily
				(*pidl)->mkid.cb = 0;                  // make this a child

				IShellFolder *folder;
				if (SUCCEEDED((*current)->BindToObject(child, nullptr, IID_PPV_ARGS(&folder))))
				{
					(*current)->Release();
					(*current) = folder;   // transfer ownership
					result = true;
				}

				(*pidl)->mkid.cb = latest; // restore previous ID size
			}
		}

		return result;
	}
	void getItemName(IShellItem2 *item, std::wstring &itemName)
	{
		// desktop
		IShellFolder *currentFolder = nullptr;
		if (SUCCEEDED(::SHGetDesktopFolder(&currentFolder)))
		{
			PUIDLIST_RELATIVE next = nullptr;
			while (moveNext(&next, &currentFolder, item) == true)
			{
				IShellItem2 *currentItem = nullptr;
				if (SUCCEEDED(getCurrentItem(&next, currentFolder, IID_PPV_ARGS(&currentItem))))
				{
					wchar_t *temp = nullptr;
					if (SUCCEEDED(currentItem->GetDisplayName(SIGDN_FILESYSPATH, &temp)))
					{
						itemName = temp;
						safeCoTaskMemFree(temp);
					}
				}
			}
		}
	}
};