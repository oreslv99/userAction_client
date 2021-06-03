#pragma once
#include "stdafx.h"

struct ruleProcess
{
	std::list<std::wstring> excludes;
	std::list<std::wstring> privates;
	std::list<std::wstring> browsers;
	std::list<std::wstring> duplicates;
};