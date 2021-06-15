#include "featureFileIo.h"

static std::map<int, std::wstring> driveTypeIds =
{
	makeString(DRIVE_UNKNOWN),
	makeString(DRIVE_NO_ROOT_DIR),
	makeString(DRIVE_REMOVABLE),
	makeString(DRIVE_FIXED),
	makeString(DRIVE_REMOTE),
	makeString(DRIVE_CDROM),
	makeString(DRIVE_RAMDISK),
};

//
// public
//
featureFileIo::featureFileIo()
	:rule(nullptr), event(INVALID_HANDLE_VALUE), id(0), watches(), entires(nullptr)
{}
featureFileIo::~featureFileIo()
{
}
bool featureFileIo::initialize(const rules &rule)
{
	// 저장장치 "연결/해제" 에 따라서 초기화가 추가 호출됨
	if (this->rule == nullptr)
	{
		this->rule = rule.getFileIoRule();
	}
	
	// 이전 
	if (::SHChangeNotifyDeregister(this->id) == FALSE)
	{
		help->writeLog(logId::warning, L"[%s:%03d] code[%d] SHChangeNotifyDeregister is failed.", __FUNCTIONW__, __LINE__, ::GetLastError());
	}
	this->id = 0;

	// 기존항목 삭제
	std::list<std::pair<std::wstring, UINT>>::iterator iterWatches;
	for (iterWatches = watches.begin(); iterWatches != watches.end(); iterWatches++)
	{
		iterWatches->first.clear();
	}
	watches.clear();

	DWORD logicalDrives = ::GetLogicalDrives();
	for (int i = 0; i < 26; i++)
	{
		if ((logicalDrives & (1 << i)) != 0)
		{
			std::wstring data = L"";
			data += L'a' + i;
			data += L":\\";
			DWORD driveType = ::GetDriveTypeW(data.c_str());

			watches.push_back(std::make_pair(data, driveType));
			help->writeLog(logId::info, L"watchable driveLetter [%s (%s)]", data.c_str(), driveTypeIds.find(driveType)->second.c_str());
		}
	}

	// 등록
	safeDelete(this->entires);
	size_t count = watches.size();
	this->entires = new SHChangeNotifyEntry[count];

	int flags = SHCNRF_ShellLevel | SHCNRF_InterruptLevel | SHCNRF_NewDelivery;
	std::list<PIDLIST_ABSOLUTE> pidls;
	int i;
	for (i = 0, iterWatches = watches.begin(); iterWatches != watches.end(); i++, iterWatches++)
	{
		IShellItem *shellItem = nullptr;
		HRESULT result = ::SHCreateItemFromParsingName(iterWatches->first.c_str(), nullptr, IID_IShellItem, (void**)&shellItem);
		if (FAILED(result))
		{
			return false;
		}

		PIDLIST_ABSOLUTE pidl;
		result = ::SHGetIDListFromObject(shellItem, &pidl);
		if (FAILED(result))
		{
			return false;
		}

		entires[i].pidl = pidl;
		entires[i].fRecursive = TRUE;
		pidls.push_back(pidl);

		safeRelease(shellItem);
	}

	/*
	#define SHCNE_RENAMEITEM          0x00000001L
	#define SHCNE_CREATE              0x00000002L
	#define SHCNE_DELETE              0x00000004L
	#define SHCNE_MKDIR               0x00000008L
	#define SHCNE_RMDIR               0x00000010L
	#define SHCNE_MEDIAINSERTED       0x00000020L
	#define SHCNE_MEDIAREMOVED        0x00000040L
	#define SHCNE_DRIVEREMOVED        0x00000080L
	#define SHCNE_DRIVEADD            0x00000100L
	#define SHCNE_NETSHARE            0x00000200L
	#define SHCNE_NETUNSHARE          0x00000400L
	#define SHCNE_ATTRIBUTES          0x00000800L
	#define SHCNE_UPDATEDIR           0x00001000L
	#define SHCNE_UPDATEITEM          0x00002000L
	#define SHCNE_SERVERDISCONNECT    0x00004000L
	#define SHCNE_UPDATEIMAGE         0x00008000L
	#define SHCNE_DRIVEADDGUI         0x00010000L
	#define SHCNE_RENAMEFOLDER        0x00020000L
	#define SHCNE_FREESPACE           0x00040000L
	*/
	long events = SHCNE_CREATE |					// 파일 생성
		SHCNE_MEDIAINSERTED | SHCNE_MEDIAREMOVED |	// 외장 저장장치
		SHCNE_DRIVEADD | SHCNE_DRIVEREMOVED |		// 외장 저장장치
		SHCNE_NETSHARE | SHCNE_NETUNSHARE;			// 외장 저장장치
	this->id = ::SHChangeNotifyRegister(this->rule->window, flags, events, WM_FILE_IO, static_cast<int>(count), entires);
	bool result = true;
	if (this->id == 0)
	{
		help->writeLog(logId::warning, L"[%s:%03d] code[%d] SHChangeNotifyRegister is failed.", __FUNCTIONW__, __LINE__, ::GetLastError());
		result = false;
	}

	// release
	std::list<PIDLIST_ABSOLUTE>::iterator iterPidl;
	for (iterPidl = pidls.begin(); iterPidl != pidls.end(); iterPidl++)
	{
		safeCoTaskMemFree(*iterPidl);
	}
	pidls.clear();

	return result;
}
bool featureFileIo::watch(void* parameters)
{
	static DWORD systemCall = 0;

	// 2회 이상 system event 가 발생하기 때문에 한번만 출력하기 위함
	if (::GetTickCount() - systemCall <= 100)
	{
		return true;
	}

	paramsFileIo *params = reinterpret_cast<paramsFileIo*>(parameters);

	PIDLIST_ABSOLUTE *pidl = nullptr;
	long id = 0;
	HANDLE lock = ::SHChangeNotification_Lock(reinterpret_cast<HANDLE>(params->wparam), static_cast<DWORD>(params->lparam), &pidl, &id);
	if (lock != INVALID_HANDLE_VALUE)
	{
		//// https://docs.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shchangenotify
		//if ((id & SHCNE_ASSOCCHANGED | SHCNE_SERVERDISCONNECT) != 0)
		//{
		//	IShellItem2 *shellItem2 = nullptr;
		//	HRESULT result;
		//	if (pidl[0] != nullptr)
		//	{
		//		result = ::SHCreateItemFromIDList(pidl[0], IID_PPV_ARGS(&shellItem2));
		//		if (FAILED(result))
		//		{
		//		}

		//		wchar_t *itemName0 = nullptr;
		//		getIdListName(item0, &itemName0);
		//	}
		//}
	}

	::SHChangeNotification_Unlock(lock);

	return true;
}
//featureType featureFileIo::getFeatureType()
//{
//	return featureType::fileIo;
//}

//
// private
//