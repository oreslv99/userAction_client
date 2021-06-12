#pragma once
#include "stdafx.h"
#include "feature.h"
#include <functional>

class featureAFK : public feature
{
public:
	featureAFK();
	~featureAFK();
	bool initialize(const rules rule) final;
	bool watch() final;
	//featureType getFeatureType() final;

private:
	const ruleAFK *rule;
	HANDLE event;

};