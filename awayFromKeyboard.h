#pragma once
#include "stdafx.h"
#include "IFeature.h"
#include <functional>

class awayFromKeyboard : public IFeature
{
public:
	awayFromKeyboard();
	~awayFromKeyboard();
	bool initialize();
	bool watch();
	featureType getType();

private:
	HANDLE event;

};