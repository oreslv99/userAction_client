#pragma once
#include "stdafx.h"
#include "winSock.h"
#include "helper.h"

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
	void release();
	int getTimerInterval() const;
	int getServerRetryInterval() const;
	ruleAFK *getAFKRule() const;
	ruleFileIo *getFileIoRule() const;
	ruleProcess *getProcessRule() const;
	rulePrint *getPrintRule() const;

private:
	bool deserializeRule(jsonDocumentW &document);

	int timerInterval;
	int serverRetryInterval;
	ruleAFK *afk;
	ruleFileIo *fileIo;
	ruleProcess *process;
	rulePrint *print;

};