#pragma once
#include "stdafx.h"
#include "feature.h"
#include "ruleProcess.h"

class featureProcess : public feature
{
public:
	featureProcess();
	~featureProcess();
	bool initialize(rules *rule) final;
	bool watch() final;
	bool isHighPriority() final;

private:
	ruleProcess *rule;

	void getFullProcessName();
	void getPureProcessName();

};