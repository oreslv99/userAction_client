#pragma once
#include "stdafx.h"
#include "winSock.h"
#include "ruleAFK.h"
#include "rulePrint.h"
#include "ruleProcess.h"

class rules
{
public:
	rules();
	~rules();
	void initialize(bool isOnline, winSock *socket);
	ruleAFK *getAFKRule();
	ruleProcess *getProcessRule();
	rulePrint *getPrintRule();

private:
	ruleAFK *afk;
	ruleProcess *process;
	rulePrint *print;

};