#pragma once
#include "rules.h"

class feature
{
public:
	virtual bool initialize(void *rule, DWORD size) = 0;
	virtual bool watch(void* parameters = nullptr) = 0;
	// 2021-06-12 
	//	: list 에 순서대로 넣어서 loop
	//virtual featureType getFeatureType() = 0;
protected:
	bool isMatch(const wchar_t *source, const wchar_t *pattern)
	{
		const wchar_t *tempSource, *tempPattern;
		bool asterisk = false;

start:
		for (tempSource = source, tempPattern = pattern; *tempSource; tempSource++, tempPattern++)
		{
			switch (*tempPattern)
			{
			case '?':
				break;
			case '*':
			{
				asterisk = true;
				source = tempSource;
				pattern = tempPattern + 1;
				if (*pattern == 0)
				{
					return true;
				}

				goto start;
			}

			default:
				if (*tempSource != *tempPattern)
				{
					goto checkAsterisk;
				}

				break;
			}
		}

		while (*tempPattern == '*')
		{
			tempPattern++;
		}

		return (*tempPattern == 0);

checkAsterisk:
		if (asterisk == false)
		{
			return false;
		}

		source++;
		goto start;
	}

};