#pragma once
#include "stdafx.h"
#include "feature.h"

class process : public feature
{
public:
	process();
	~process();
	bool initialize() final;
	bool watch() final;
	bool isHighPriority() final;

private:
	void getFullProcessName();
	void getPureProcessName();

};