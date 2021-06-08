#pragma once
#include "stdafx.h"
#include "feature.h"
#include "ruleFileIo.h"
#include <functional>

class featureFileIo : public feature
{
public:
	featureFileIo();
	~featureFileIo();
	bool initialize(void *rule, void *extra, DWORD extraSize) final;
	bool watch() final;
	bool isHighPriority() final;

private:
	ruleFileIo *rule;
	HANDLE event;

};