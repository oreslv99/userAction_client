#pragma once
#include "stdafx.h"
#include "IFeature.h"

class process : public IFeature
{
public:
	process();
	~process();
	bool initialize();
	bool watch();
	featureType getType();

private:

};