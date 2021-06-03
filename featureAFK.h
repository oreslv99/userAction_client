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
	bool initialize(rules *rule) final;
	bool watch() final;
	bool isHighPriority() final;

private:
	ruleAFK *rule;
	HANDLE event;

};