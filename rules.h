#pragma once
#include "stdafx.h"
#include "winSock.h"
#include "ruleAFK.h"
#include "ruleFileIo.h"
#include "rulePrint.h"
#include "ruleProcess.h"

class rules
{
public:
	rules();
	~rules();
	void initialize(bool isOnline, winSock *socket);
	ruleAFK *getAFKRule();
	ruleFileIo *getFileIoRule();
	ruleProcess *getProcessRule();
	rulePrint *getPrintRule();

private:
	ruleAFK *afk;
	ruleFileIo *fileIo;
	ruleProcess *process;
	rulePrint *print;

};