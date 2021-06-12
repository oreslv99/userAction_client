#pragma once
#include "stdafx.h"
#include "winSock.h"
#include ".\\extern\\rapidJson\\document.h"
#include ".\\extern\\rapidJson\\writer.h"
#include ".\\extern\\rapidJson\\istreamwrapper.h"

struct ruleAFK
{
	bool enabled;
	int in;
	int awake;
};
struct ruleFileIo
{
	bool enabled;
	HWND window;	// System 으로부터 Event 마다 Broadcast 될 WndProc 를 등록하기 위한 윈도우
	std::list<std::wstring> excludes;
	std::list<std::wstring> extensions;
};
struct ruleProcess
{
	bool enabled;
	std::list<std::wstring> excludes;
	std::list<std::wstring> privates;
	std::list<std::wstring> browsers;
	std::list<std::wstring> duplicates;
};
struct rulePrint
{
	bool enabled;
};

class rules
{
public:
	rules();
	~rules();
	void initialize(winSock *socket, HWND window);
	int getTimerInterval() const;
	ruleAFK *getAFKRule() const;
	ruleFileIo *getFileIoRule() const;
	ruleProcess *getProcessRule() const;
	rulePrint *getPrintRule() const;

private:
	typedef rapidjson::GenericDocument<rapidjson::UTF16<>> jsonDocumentW;								// wide-char utf16 재정의 Document
	typedef rapidjson::GenericArray<true, rapidjson::GenericValue<rapidjson::UTF16<>>> jsonCArrayW;		// wide-char utf16 재정의 Const Array
	typedef rapidjson::GenericArray<false, rapidjson::GenericValue<rapidjson::UTF16<>>> jsonArrayW;		// wide-char utf16 재정의 Array
	typedef rapidjson::GenericObject<true, rapidjson::GenericValue<rapidjson::UTF16<>>> jsonCObjectW;	// wide-char utf16 재정의 Const Object
	typedef rapidjson::GenericObject<false, rapidjson::GenericValue<rapidjson::UTF16<>>> jsonObjectW;	// wide-char utf16 재정의 Object
	typedef rapidjson::GenericValue<rapidjson::UTF16<>> jsonValueW;										// wide-char utf16 재정의 GenericValue<UTF16<>>
	typedef rapidjson::GenericValue<rapidjson::UTF16<>>::ConstMemberIterator jsonCMemberIteratorW;		// wide-char utf16 재정의 Const MemberIterator
	typedef rapidjson::GenericValue<rapidjson::UTF16<>>::ConstValueIterator jsonCValueIteratorW;		// wide-char utf16 재정의 Const ValueIterator
	typedef rapidjson::GenericValue<rapidjson::UTF16<>>::MemberIterator jsonMemberIteratorW;			// wide-char utf16 재정의 MemberIterator
	typedef rapidjson::GenericValue<rapidjson::UTF16<>>::ValueIterator jsonValueIteratorW;				// wide-char utf16 재정의 ValueIterator
	typedef rapidjson::GenericDocument<rapidjson::UTF16<>> jsonDocumentForWriteW;						// wide-char utf16 재정의 Document
	typedef rapidjson::GenericValue<rapidjson::UTF16<>> jsonValueForWriteW;								// wide-char utf16 재정의 GenericValue<UTF16<>>
	typedef rapidjson::GenericStringBuffer<rapidjson::UTF16<>> jsonStringBufferW;						// wide-char utf16 재정의 GenericStringBuffer<UTF16<>>
	typedef rapidjson::Writer<jsonStringBufferW, rapidjson::UTF16<>> jsonStringWriterW;					// Document에서 wchar_t로 serialize

	bool getJsonDocumentFromFile(const std::wstring filePath, jsonDocumentW *buffer);
	bool getJsonDocumentFromString(const std::wstring jsonString, jsonDocumentW *buffer);
	bool deserializeRule(jsonDocumentW document);

	int timerInterval;
	ruleAFK *afk;
	ruleFileIo *fileIo;
	ruleProcess *process;
	rulePrint *print;

};