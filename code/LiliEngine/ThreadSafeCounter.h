#pragma once

#include "PlatformAtomics.h"

class ThreadSafeCounter
{
public:
	typedef int32_t IntegerType;

	ThreadSafeCounter()
	{
		m_Counter = 0;
	}

	ThreadSafeCounter(const ThreadSafeCounter& other)
	{
		m_Counter = other.GetValue();
	}

	ThreadSafeCounter(int32_t value)
	{
		m_Counter = value;
	}

	int32_t Increment()
	{
		return PlatformAtomics::InterlockedIncrement(&m_Counter);
	}

	int32_t Add(int32_t amount)
	{
		return PlatformAtomics::InterlockedAdd(&m_Counter, amount);
	}

	int32_t Decrement()
	{
		return PlatformAtomics::InterlockedDecrement(&m_Counter);
	}

	int32_t Subtract(int32_t amount)
	{
		return PlatformAtomics::InterlockedAdd(&m_Counter, -amount);
	}

	int32_t Set(int32_t value)
	{
		return PlatformAtomics::InterlockedExchange(&m_Counter, value);
	}

	int32_t Reset()
	{
		return PlatformAtomics::InterlockedExchange(&m_Counter, 0);
	}

	int32_t GetValue() const
	{
		return PlatformAtomics::AtomicRead(&(const_cast<ThreadSafeCounter*>(this)->m_Counter));
	}

private:
	void operator=(const ThreadSafeCounter&) = delete;

	volatile int32_t m_Counter;
};