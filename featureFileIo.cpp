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

	// 2ȸ �̻� system event �� �߻��ϱ� ������ �ѹ��� ����ϱ� ����
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
			// source >> destination ó�� �ΰ��� �ʿ��Ѱ�쿡�� (rename)
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

			// Ȯ���� ���� ��ȿ�ϰ� �ֱ� �����Ϳ� �������� ���� ���
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
						// ��� Ȯ������� Ȯ��
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

						// ���� ������� Ȯ��
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
						// �̸� ���ǵ� �̺�Ʈ id Ȯ��
						std::map<long, eventId>::iterator eventIter = watch_file_io_types_2.find(id);
						if (eventIter != watch_file_io_types_2.end())
						{
							// �����ü (usb) �� ����/����
							logData *data = nullptr;
							eventId newId = eventIter->second;
							wchar_t *fileExtension = nullptr;
							wchar_t *fileSize = nullptr;

							switch (newId)
							{
								// TODO
								//	: ��ŷ �����̼� ���� ���� drive letter �� ���ÿ� mount �Ǵ� ���, event �� ó���� �ϳ��� ��. 
								//	: event �� �ϳ��� �����°� ������?
							case dev_con:
							case dev_discon:
							{
								// ������
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
									// �����ü ������ file io
									// ���� : isMatched(itemName0, *container_iter);
									if ((::wcsstr(itemName0, targetIter->first) != nullptr) && (targetIter->second == DRIVE_REMOVABLE))
									{
										newId = dev_file;
										break;
									}
								}

								getFileExtension(itemName0, &fileExtension);
								getFileSize(itemName0, &fileSize);

								// 2020-10-26 orseL 
								//	: �� �����Ǵ� ��� ifstream tellg �� �����Ͽ� -1
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

							// 2ȸ �̻� system event �� �߻��ϱ� ������ �ѹ��� ����ϱ� ����
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