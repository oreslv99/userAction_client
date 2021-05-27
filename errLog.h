#pragma once
#include "stdafx.h"
#include <shlobj_core.h>	// SHGetKnownFolderPath
#include <map>
#include <fstream>
#include <codecvt>			// utf8 로 변환

enum errId 
{
	debug = 0,
	info,
	userAction,
	error,
};
static class errLog 
{
public:
	static errLog *getInstance();
	static bool initialize();
	static void release();
	static void write(errId id, std::wstring message, ...);

private:
	static std::wstring path;		// 저장경로
	static CRITICAL_SECTION cs;		// sync
	static errLog *instance;

};