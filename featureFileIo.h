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

	class CItemIterator
	{
	public:
		CItemIterator(IShellItem *psi)
			: _hr(SHGetIDListFromObject(psi, &_pidlFull)), _psfCur(nullptr)
		{
			_Init();
		}
		CItemIterator(PCIDLIST_ABSOLUTE pidl) : _hr(SHILCloneFull(pidl, &_pidlFull)), _psfCur(nullptr)
		{
			_Init();
		}
		~CItemIterator()
		{
			safeCoTaskMemFree(_pidlFull);
			safeRelease(_psfCur);
		}
		bool MoveNext()
		{
			bool fMoreItems = false;

			if (SUCCEEDED(_hr))
			{
				if (_pidlRel == nullptr)
				{
					fMoreItems = true;
					_pidlRel = _pidlFull;   // first item, might be empty if it is the desktop
				}
				else if (!ILIsEmpty(_pidlRel))
				{
					PCUITEMID_CHILD pidlChild = (PCUITEMID_CHILD)_pidlRel;  // save the current segment for binding
					_pidlRel = ILNext(_pidlRel);

					// if we are not at the end setup for the next itteration
					if (!ILIsEmpty(_pidlRel))
					{
						const WORD cbSave = _pidlRel->mkid.cb;  // avoid cloning for the child by truncating temporarily
						_pidlRel->mkid.cb = 0;                  // make this a child

						IShellFolder *psfNew;
						_hr = _psfCur->BindToObject(pidlChild, nullptr, IID_PPV_ARGS(&psfNew));
						if (SUCCEEDED(_hr))
						{
							_psfCur->Release();
							_psfCur = psfNew;   // transfer ownership
							fMoreItems = true;
						}

						_pidlRel->mkid.cb = cbSave; // restore previous ID size
					}
				}
			}

			return fMoreItems;
		}
		HRESULT GetCurrent(REFIID riid, void **ppv)
		{
			*ppv = nullptr;
			if (SUCCEEDED(_hr))
			{
				// create teh childID by truncating _pidlRel temporarily
				PUIDLIST_RELATIVE pidlNext = ILNext(_pidlRel);
				const WORD cbSave = pidlNext->mkid.cb;  // save old cb
				pidlNext->mkid.cb = 0;                  // make _pidlRel a child

				_hr = ::SHCreateItemWithParent(nullptr, _psfCur, (PCUITEMID_CHILD)_pidlRel, riid, ppv);

				pidlNext->mkid.cb = cbSave;             // restore old cb
			}

			return _hr;
		}
		HRESULT GetResult() const { return _hr; }
		PCUIDLIST_RELATIVE GetRelativeIDList() const { return _pidlRel; }

	private:
		void _Init()
		{
			_pidlRel = nullptr;

			if (SUCCEEDED(_hr))
			{
				_hr = ::SHGetDesktopFolder(&_psfCur);
			}
		}
		HRESULT SHILCloneFull(PCUIDLIST_ABSOLUTE pidl, PIDLIST_ABSOLUTE *ppidl)
		{
			*ppidl = ILCloneFull(pidl);
			return ((*ppidl != nullptr) ? S_OK : E_OUTOFMEMORY);
		}

		HRESULT _hr;
		PIDLIST_ABSOLUTE _pidlFull;
		PUIDLIST_RELATIVE _pidlRel;
		IShellFolder *_psfCur;
	};

};