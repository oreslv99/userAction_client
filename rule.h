#pragma once
#include <Windows.h>
#include <string>

struct idleRule
{
	bool enabled;
	int inIdleTime;			// �ڸ����
	int minimumAwakeTime;	// ������� �����ӿ� ��ٷ� ����� ���� �����ϱ� ���� �ּ����� ���ð�
	HANDLE wakeUp;			// loop escape �� event
};
	
struct rule
{
	int interval;			//
	idleRule idle;			//

};