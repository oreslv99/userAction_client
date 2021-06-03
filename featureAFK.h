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
	bool initialize(rule *featureRule) final;
	bool watch() final;
	bool isHighPriority() final;

private:
	HANDLE event;
	ruleAFK rules;

};