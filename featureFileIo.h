#pragma once
#include "stdafx.h"
#include "feature.h"
#include <functional>

class featureFileIo : public feature
{
public:
	featureFileIo();
	~featureFileIo();
	bool initialize(const rules rule) final;
	bool watch() final;
	//featureType getFeatureType() final;

private:
	const ruleFileIo *rule;
	HANDLE event;

};