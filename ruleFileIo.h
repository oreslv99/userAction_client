#pragma once
#include "stdafx.h"

struct ruleFileIo
{
	bool enabled;
	std::list<std::wstring> excludes;
	std::list<std::wstring> extensions;
};