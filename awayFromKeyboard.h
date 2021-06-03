#pragma once
#include "stdafx.h"
#include "feature.h"
#include <functional>

class awayFromKeyboard : public feature
{
public:
	awayFromKeyboard();
	~awayFromKeyboard();
	bool initialize() final;
	bool watch() final;
	bool isHighPriority() final;

private:
	HANDLE event;

};