#pragma once

#if PLATFORM_WINDOWS

class GenericPlatformTime final
{
public:
	static double InitTiming();
	static FORCEINLINE double Seconds()
	{
		LARGE_INTEGER cycles;
		QueryPerformanceCounter(&cycles);
		return cycles.QuadPart * GetSecondsPerCycle() + 16777216.0;
	}

	static FORCEINLINE double GetSecondsPerCycle()
	{
		return s_secondsPerCycle;
	}

private:
	static double s_secondsPerCycle;
};

#endif // PLATFORM_WINDOWS