#pragma once
#include "stdafx.h"
#include "feature.h"
#include "ruleAFK.h"
#include <functional>

class featureAFK : public feature
{
public:
	featureAFK();
	~featureAFK();
	bool initialize(void *rule, void *extra = nullptr, DWORD extraSize = 0) final;
	bool watch() final;
	bool isHighPriority() final;

private:
	ruleAFK *rule;
	HANDLE event;

};