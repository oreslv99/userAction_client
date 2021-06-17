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
		// IShellItem2 �ν��Ͻ� (riid, ppv) �� �޼����� GetDisplayName + SIGDN_FILESYSPATH ȣ���� ����
		//	>> ��ü��� (������� + display �̸�) �����ϱ� ����
		//	>> SHCreateItemWithParent �� �̿��� pidl �� ������ο� display �̸��� ���� shell item �� ����

		*ppv = nullptr;
		
		// ���� pidl
		PUIDLIST_RELATIVE next = ILNext(*pidl);
		
		// pidl child �����ϱ� ���� ���� �� ��� ���� �� �ʱ�ȭ
		USHORT latest = next->mkid.cb;
		next->mkid.cb = 0;             

		// pidl child ������ ���� �� �ο�
		HRESULT result = ::SHCreateItemWithParent(nullptr, current, reinterpret_cast<PCUITEMID_CHILD>(*pidl), riid, ppv);
		next->mkid.cb = latest;

		return result;
	}
	bool moveNext(PUIDLIST_RELATIVE *pidl, IShellFolder **current, IShellItem2 *item)
	{
		bool result = false;
		if (*pidl == nullptr)
		{
			// item ���κ��� pidl Ȯ��
			PIDLIST_ABSOLUTE pidlAbs;
			if (SUCCEEDED(::SHGetIDListFromObject(item, &pidlAbs)))
			{
				*pidl = pidlAbs;
				result = true;
			}
		}
		else if (ILIsEmpty(*pidl) == false)
		{
			// pidl �� ������� ����

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