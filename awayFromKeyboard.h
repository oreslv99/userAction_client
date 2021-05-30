#pragma once
#include "stdafx.h"
#include <functional>

class awayFromKeyboard
{
public:
	awayFromKeyboard();
	~awayFromKeyboard();
	bool inAFK();

private:
	HANDLE event;

};