#pragma once
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>

// safe ÇÔ¼ö
#define safeFree(x)					if (x) { ::free(x); (x) = nullptr; }
#define safeDelete(x)				if (x) { delete (x); (x) = nullptr; }
#define safeDeleteArray(x)			if (x) { delete[] (x); (x) = nullptr; }
#define safeRelease(x)				if (x) { (x)->Release(); (x) = nullptr; }
#define safeCoTaskMemFree(x)		if (x) { ::CoTaskMemFree(x); (x) = nullptr; }
#define safeCloseHandle(x)			if (x) { ::CloseHandle(x); (x) = INVALID_HANDLE_VALUE; }

#include "errLog.h"
#define log errLog::getInstance()