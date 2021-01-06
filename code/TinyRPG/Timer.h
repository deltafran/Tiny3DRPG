#pragma once

enum ETimeTypes
{
    TIME_SECOND = 0,
    TIME_MINUTE,
    TIME_HOUR,
    TIME_DAY,
    TIME_MONTH,
    TIME_YEAR
};

class Timer
{
public:
	//! Returns current time as a string in the form "DD/MM/YYYY HH:MM:SS" (e.g. "01/01/2021 15:00:00").
	static std::string GetTime();

    //! Returns the specified time value.
    static uint32_t GetTime(const ETimeTypes Type);
};