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
enum featureId
{
	afkIn = 0,		// 자리비움 시작
	afkAwk,			// 자리비움 해제
	file,			// 파일 생성
	devCon,			// 장치 연결
	devDcon,		// 장치 해제
	print,			// 문서 출력
	program,		// 일반 프로그램
	browser,		// 웹 브라우저 프로그램
	logon,			// 로그온
	logoff,			// 로그오프
};
class helper
{
public:
	static helper *getInstance();
	static bool initialize();
	static void release();
	static void writeLog(logId id, std::wstring message, ...);
	static void writeUserAction(featureId id, std::wstring message, ...);
	static void toLower(std::wstring &source);

private:
	static std::wstring path;		// 저장경로
	static CRITICAL_SECTION cs;		// sync
	static helper *instance;

};