#pragma once
#include "stdafx.h"
#include "feature.h"

class featureProcess : public feature
{
public:
	featureProcess();
	~featureProcess();
	bool initialize(rule *featureRule) final;
	bool watch() final;
	bool isHighPriority() final;

private:
	void getFullProcessName();
	void getPureProcessName();

};