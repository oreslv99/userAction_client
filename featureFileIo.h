#pragma once
#include "stdafx.h"
#include "feature.h"
#include <functional>

class featureFileIo : public feature
{
public:
	featureFileIo();
	~featureFileIo();
	bool initialize(const rules &rule) final;
	bool watch(void* parameters = nullptr) final;
	//featureType getFeatureType() final;

private:
	const ruleFileIo *rule;
	HANDLE event;

};