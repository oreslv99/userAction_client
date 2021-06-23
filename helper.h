#pragma once
#include "stdafx.h"
#include <shlobj_core.h>	// SHGetKnownFolderPath
#include <map>
#include <fstream>
#include <algorithm> 
// rapid json
#include ".\\extern\\rapidJson\\document.h"
#include ".\\extern\\rapidJson\\writer.h"
#include ".\\extern\\rapidJson\\istreamwrapper.h"

enum logId 
{
	debug = 0,
	info,
	warning,
	error,
};
enum featureId
{
	afkIn = 0,		// �ڸ���� ����
	afkAwk,			// �ڸ���� ����
	file,			// ���� ����
	devCon,			// ��ġ ����
	devDcon,		// ��ġ ����
	print,			// ���� ���
	program,		// �Ϲ� ���α׷�
	browser,		// �� ������ ���α׷�
	logon,			// �α׿�
	logoff,			// �α׿���
};

// multi-byte
typedef rapidjson::GenericDocument<rapidjson::UTF8<>> jsonDocumentForWriteA;						// multibyte-char utf8 ������ Document
typedef rapidjson::GenericValue<rapidjson::UTF8<>> jsonValueForWriteA;								// multibyte-char utf8 ������ GenericValue<UTF8<>>
typedef rapidjson::GenericStringBuffer<rapidjson::UTF8<>> jsonStringBufferA;						// multibyte-char utf8 ������ GenericStringBuffer<UTF8<>>
typedef rapidjson::Writer<jsonStringBufferA, rapidjson::UTF8<>> jsonStringWriterA;

typedef rapidjson::GenericDocument<rapidjson::UTF8<>> jsonDocumentA;								// multibyte-char utf8 ������ Document
typedef rapidjson::GenericArray<false, rapidjson::GenericValue<rapidjson::UTF8<>>> jsonArrayA;		// multibyte-char utf8 ������ Array
typedef rapidjson::GenericObject<false, rapidjson::GenericValue<rapidjson::UTF8<>>> jsonObjectA;	// multibyte-char utf8 ������ Object
typedef rapidjson::GenericValue<rapidjson::UTF8<>> jsonValueA;										// multibyte-char utf8 ������ GenericValue<UTF16<>>
typedef rapidjson::GenericValue<rapidjson::UTF8<>>::MemberIterator jsonMemberIteratorA;				// multibyte-char utf8 ������ MemberIterator
typedef rapidjson::GenericValue<rapidjson::UTF8<>>::ValueIterator jsonValueIteratorA;

// wide-char
typedef rapidjson::GenericDocument<rapidjson::UTF16<>> jsonDocumentForWriteW;						// wide-char utf16 ������ Document
typedef rapidjson::GenericValue<rapidjson::UTF16<>> jsonValueForWriteW;								// wide-char utf16 ������ GenericValue<UTF16<>>
typedef rapidjson::GenericStringBuffer<rapidjson::UTF16<>> jsonStringBufferW;						// wide-char utf16 ������ GenericStringBuffer<UTF16<>>
typedef rapidjson::Writer<jsonStringBufferW, rapidjson::UTF16<>> jsonStringWriterW;					// Document���� wchar_t�� serialize

typedef rapidjson::GenericDocument<rapidjson::UTF16<>> jsonDocumentW;								// wide-char utf16 ������ Document
typedef rapidjson::GenericArray<false, rapidjson::GenericValue<rapidjson::UTF16<>>> jsonArrayW;		// wide-char utf16 ������ Array
typedef rapidjson::GenericObject<false, rapidjson::GenericValue<rapidjson::UTF16<>>> jsonObjectW;	// wide-char utf16 ������ Object
typedef rapidjson::GenericValue<rapidjson::UTF16<>> jsonValueW;										// wide-char utf16 ������ GenericValue<UTF16<>>
typedef rapidjson::GenericValue<rapidjson::UTF16<>>::MemberIterator jsonMemberIteratorW;			// wide-char utf16 ������ MemberIterator
typedef rapidjson::GenericValue<rapidjson::UTF16<>>::ValueIterator jsonValueIteratorW;				// wide-char utf16 ������ ValueIterator

class helper
{
public:
	static helper *getInstance();
	static bool initialize();
	static void release();
	static void writeLog(logId id, std::wstring message, ...);
	static void writeUserAction(featureId id, std::wstring message, ...);
	static void toLower(std::wstring &source);

	static bool getJsonDocumentFromFile(const std::wstring filePath, jsonDocumentW *buffer);
	static bool getJsonDocumentFromString(const std::wstring jsonString, jsonDocumentW *buffer);

private:
	static std::wstring path;		// ������
	static CRITICAL_SECTION cs;		// sync
	static helper *instance;

};