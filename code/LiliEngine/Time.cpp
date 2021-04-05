#include "stdafx.h"
#include "Time.h"
#if PLATFORM_WINDOWS

double GenericPlatformTime::s_secondsPerCycle = 0.0f;

double GenericPlatformTime::InitTiming()
{
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	s_secondsPerCycle = 1.0 / frequency.QuadPart;
	return GenericPlatformTime::Seconds();
}

#endif // PLATFORM_WINDOWS