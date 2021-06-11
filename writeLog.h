#pragma once
#include "stdafx.h"
#include <shlobj_core.h>	// SHGetKnownFolderPath
#include <map>
#include <fstream>
#include <codecvt>			// utf8 로 변환

enum logId 
{
	debug = 0,
	info,
	warning,
	user,
	error,
};
class writeLog
{
public:
	static writeLog *getInstance();
	static bool initialize();
	static void release();
	static void write(logId id, std::wstring message, ...);
	static void writeUserAction(std::wstring message, ...);

private:
	static std::wstring path;		// 저장경로
	static CRITICAL_SECTION cs;		// sync
	static writeLog *instance;

};