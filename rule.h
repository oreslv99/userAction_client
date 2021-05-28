#pragma once
#include <Windows.h>
#include <string>

struct idleRule
{
	bool enabled;
	int inIdleTime;			// 자리비움
	int minimumAwakeTime;	// 사용자의 움직임에 곧바로 깨어나는 것을 방지하기 위한 최소한의 대기시간
	HANDLE wakeUp;			// loop escape 용 event
};
	
struct rule
{
	int interval;			//
	idleRule idle;			//

};