#include "stdafx.h"
#include "Timer.h"
#include "StringUtils.h"

std::string Timer::GetTime()
{
	return
		Str::Number(GetTime(TIME_DAY), 2) + "/" +
		Str::Number(GetTime(TIME_MONTH), 2) + "/" +
		std::to_string(GetTime(TIME_YEAR)) + " " +
		Str::Number(GetTime(TIME_HOUR), 2) + ":" +
		Str::Number(GetTime(TIME_MINUTE), 2) + ":" +
		Str::Number(GetTime(TIME_SECOND), 2);
}

uint32_t Timer::GetTime(const ETimeTypes Type)
{
	time_t RawTime = time(0);
	tm Time;
	localtime_s(&Time, &RawTime);

	uint32_t TimeVal = 0;

	switch (Type)
	{
	case TIME_SECOND:
		TimeVal = Time.tm_sec; break;
	case TIME_MINUTE:
		TimeVal = Time.tm_min; break;
	case TIME_HOUR:
		TimeVal = Time.tm_hour; break;
	case TIME_DAY:
		TimeVal = Time.tm_mday; break;
	case TIME_MONTH:
		TimeVal = Time.tm_mon + 1; break;
	case TIME_YEAR:
		TimeVal = Time.tm_year + 1900; break;
	}

	return static_cast<uint32_t>(TimeVal);
}