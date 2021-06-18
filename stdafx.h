#pragma once
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <list>
#include <vector>
#include <codecvt>					// utf8 로 변환
#include <algorithm>

// safe 함수
#define safeFree(x)					if (x) { ::free(x); (x) = nullptr; }
#define safeDelete(x)				if (x) { delete (x); (x) = nullptr; }
#define safeDeleteArray(x)			if (x) { delete[] (x); (x) = nullptr; }
#define safeRelease(x)				if (x) { (x)->Release(); (x) = nullptr; }
#define safeCoTaskMemFree(x)		if (x) { ::CoTaskMemFree(x); (x) = nullptr; }
#define safeCloseHandle(x)			if (x) { ::CloseHandle(x); (x) = INVALID_HANDLE_VALUE; }
#define makeString(x)				{ x, L#x }


#include "helper.h"
#define help helper::getInstance()