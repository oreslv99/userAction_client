#pragma once
#include "stdafx.h"
#include "feature.h"

class featureProcess : public feature
{
public:
	featureProcess();
	~featureProcess();
	bool initialize(const rules rule) final;
	bool watch() final;
	//featureType getFeatureType() final;

private:
	const ruleProcess *rule;

	void getFullProcessName();
	void getPureProcessName();

};