#pragma once
#include "stdafx.h"
#include <shlobj_core.h>	// SHGetKnownFolderPath
#include <map>
#include <fstream>
#include <algorithm>

enum logId 
{
	debug = 0,
	info,
	warning,
	error,
};
class helper
{
public:
	static helper *getInstance();
	static bool initialize();
	static void release();
	static void writeLog(logId id, std::wstring message, ...);
	static void writeUserAction(std::wstring message, ...);
	static void toLower(std::wstring &source);

private:
	static std::wstring path;		// 저장경로
	static CRITICAL_SECTION cs;		// sync
	static helper *instance;

};