#include "featureFileIo.h"

//
// public
//
featureFileIo::featureFileIo()
	:rule(nullptr), event(INVALID_HANDLE_VALUE)
{}
featureFileIo::~featureFileIo()
{
}
bool featureFileIo::initialize(const rules &rule)
{
	this->rule = rule.getFileIoRule();
	return true;
}
bool featureFileIo::watch()
{
	//(HWND hWnd, WPARAM wParam, LPARAM lParam)

	// 2회 이상 system event 가 발생하기 때문에 한번만 출력하기 위함
	if (::GetTickCount() - watch_file_io_latest_dev_tick <= 100)
	{
		return;
	}

	PIDLIST_ABSOLUTE *pidList = nullptr;
	long id = 0;
	HANDLE lock = ::SHChangeNotification_Lock((HANDLE)wParam, (DWORD)lParam, &pidList, &id);
	if (lock != INVALID_HANDLE_VALUE)
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shchangenotify
		if ((id & SHCNE_ASSOCCHANGED | SHCNE_SERVERDISCONNECT) != 0)
		{
			IShellItem2 *item0 = nullptr;
			//ATL::CComPtr<IShellItem2> item1 = nullptr;

			HRESULT result;
			if (pidList[0] != nullptr)
			{
				result = ::SHCreateItemFromIDList(pidList[0], IID_PPV_ARGS(&item0));
				if (FAILED(result))
				{
				}
			}
			// source >> destination 처럼 두개가 필요한경우에만 (rename)
			//if (pidList[1] != nullptr)
			//{
			//	result = ::SHCreateItemFromIDList(pidList[1], IID_PPV_ARGS(&item1));
			//	if (FAILED(result))
			//	{
			//	}
			//}

			wchar_t *itemName0 = nullptr;
			getIdListName(item0, &itemName0);

			//wchar_t *itemName1 = nullptr;
			//getIdListName(item1, &itemName1);

			// 확인한 값이 유효하고 최근 데이터와 동일하지 않은 경우
			if ((itemName0 != nullptr) && (::wcslen(itemName0) > 0))
			{
				while (true)
				{
					std::list<const wchar_t*>::iterator container_iter; // exclude_path_iter;

					bool device = ((::wcslen(itemName0) == 3) && (itemName0[1] == ':') && (itemName0[2] == '\\'));
					bool includeExtension = false;
					bool excludePath = false;

					if (device == false)
					{
						// 대상 확장명인지 확인
						wchar_t *extension = nullptr;
						getFileExtension(itemName0, &extension);
						for (container_iter = stalking_rule.watch_file_io_include_extensions.begin(); container_iter != stalking_rule.watch_file_io_include_extensions.end(); container_iter++)
						{
							//includeExtension = (::_wcsicmp(extension, *container_iter) == 0);
							includeExtension = isMatched(extension, *container_iter);
							//traceW(L"warning [%s:%d] excluded extension. [%s][%s]\n", __FUNCTIONW__, __LINE__, extension, *container_iter);
							if (includeExtension == true)
							{
								break;
							}
						}
						safeFree(extension);
						if (includeExtension == false)
						{
							break;
						}

						// 예외 경로인지 확인
						for (container_iter = stalking_rule.watch_file_io_exclude_paths.begin(); container_iter != stalking_rule.watch_file_io_exclude_paths.end(); container_iter++)
						{
							//excludePath = ((::wcsstr(itemName0, *container_iter) != nullptr) ? true : false);
							excludePath = isMatched(itemName0, *container_iter);
							if (excludePath == true)
							{
								//traceW(L"warning [%s:%d] excluded path. [%s][%s]\n", __FUNCTIONW__, __LINE__, itemName0, *container_iter);
								break;
							}
						}
					}

					//if (exclude == false)
					if ((device == true) ||
						((includeExtension == true) && (excludePath == false)))
					{
						// 미리 정의된 이벤트 id 확인
						std::map<long, eventId>::iterator eventIter = watch_file_io_types_2.find(id);
						if (eventIter != watch_file_io_types_2.end())
						{
							// 외장매체 (usb) 가 연결/해제
							logData *data = nullptr;
							eventId newId = eventIter->second;
							wchar_t *fileExtension = nullptr;
							wchar_t *fileSize = nullptr;

							switch (newId)
							{
								// TODO
								//	: 도킹 스테이션 같이 여러 drive letter 가 동시에 mount 되는 경우, event 가 처리가 하나만 됨. 
								//	: event 가 하나만 들어오는거 같은데?
							case dev_con:
							case dev_discon:
							{
								// 재적용
								data = new logData{ nullptr, nullptr, itemName0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, };
								write_log(newId, data);

								initialize_watch_file_io(hWnd);
								break;
							}
							case file:
							{
								std::list<std::pair<wchar_t*, UINT>>::iterator targetIter;
								for (targetIter = watch_file_io_include_targets.begin(); targetIter != watch_file_io_include_targets.end(); targetIter++)
								{
									// 외장매체 영역의 file io
									// 보류 : isMatched(itemName0, *container_iter);
									if ((::wcsstr(itemName0, targetIter->first) != nullptr) && (targetIter->second == DRIVE_REMOVABLE))
									{
										newId = dev_file;
										break;
									}
								}

								getFileExtension(itemName0, &fileExtension);
								getFileSize(itemName0, &fileSize);

								// 2020-10-26 orseL 
								//	: 막 생성되는 경우 ifstream tellg 가 실패하여 -1
								if (::_wcsicmp(fileSize, L"-1") != 0)
								{
									data = new logData{ nullptr, nullptr, itemName0, fileExtension, fileSize, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, };
									write_log(newId, data);
								}
								break;
							}
							}

							safeFree(fileSize);
							safeFree(fileExtension);
							safeDelete(data);

							// 2회 이상 system event 가 발생하기 때문에 한번만 출력하기 위함
							watch_file_io_latest_dev_tick = ::GetTickCount();
						}
					}

					break;
				}
			}

			safeFree(itemName0);
			safeRelease(item0);
		}

		::SHChangeNotification_Unlock(lock);
	}

	return true;
}
//featureType featureFileIo::getFeatureType()
//{
//	return featureType::fileIo;
//}