#pragma once

#if PLATFORM_WINDOWS

struct PlatformAtomics
{
	static_assert(sizeof(int8_t) == sizeof(char) && alignof(int8_t) == alignof(char), "int8_t must be compatible with char");
	static_assert(sizeof(int16_t) == sizeof(short) && alignof(int16_t) == alignof(short), "int16_t must be compatible with short");
	static_assert(sizeof(int32_t) == sizeof(long) && alignof(int32_t) == alignof(long), "int32_t must be compatible with long");
	static_assert(sizeof(int64_t) == sizeof(long long) && alignof(int64_t) == alignof(long long), "int64_t must be compatible with long long");

	static FORCEINLINE int8_t InterlockedIncrement(volatile int8_t* value)
	{
		return (int8_t)_InterlockedExchangeAdd8((char*)value, 1) + 1;
	}

	static FORCEINLINE int16_t InterlockedIncrement(volatile int16_t* value)
	{
		return (int16_t)_InterlockedIncrement16((short*)value);
	}

	static FORCEINLINE int32_t InterlockedIncrement(volatile int32_t* value)
	{
		return (int32_t)::_InterlockedIncrement((long*)value);
	}

	static FORCEINLINE int64_t InterlockedIncrement(volatile int64_t* value)
	{
#if PLATFORM_64BITS
		return (int64_t)::_InterlockedIncrement64((long long*)value);
#else
		while (true)
		{
			int64_t oldValue = *value;
			if (_InterlockedCompareExchange64(value, oldValue + 1, oldValue) == oldValue) {
				return oldValue + 1;
			}
		}
#endif
	}

	static FORCEINLINE int8_t InterlockedDecrement(volatile int8_t* value)
	{
		return (int8_t)::_InterlockedExchangeAdd8((char*)value, -1) - 1;
	}

	static FORCEINLINE int16_t InterlockedDecrement(volatile int16_t* value)
	{
		return (int16_t)::_InterlockedDecrement16((short*)value);
	}

	static FORCEINLINE int32_t InterlockedDecrement(volatile int32_t* value)
	{
		return (int32_t)::_InterlockedDecrement((long*)value);
	}

	static FORCEINLINE int64_t InterlockedDecrement(volatile int64_t* value)
	{
#if PLATFORM_64BITS
		return (int64_t)::_InterlockedDecrement64((long long*)value);
#else
		while (true)
		{
			int64_t oldValue = *value;
			if (_InterlockedCompareExchange64(value, oldValue - 1, oldValue) == oldValue) {
				return oldValue - 1;
			}
		}
#endif
	}

	static FORCEINLINE int8_t InterlockedAdd(volatile int8_t* value, int8_t amount)
	{
		return (int8_t)::_InterlockedExchangeAdd8((char*)value, (char)amount);
	}

	static FORCEINLINE int16_t InterlockedAdd(volatile int16_t* value, int16_t amount)
	{
		return (int16_t)::_InterlockedExchangeAdd16((short*)value, (short)amount);
	}

	static FORCEINLINE int32_t InterlockedAdd(volatile int32_t* value, int32_t amount)
	{
		return (int32_t)::_InterlockedExchangeAdd((long*)value, (long)amount);
	}

	static FORCEINLINE int64_t InterlockedAdd(volatile int64_t* value, int64_t amount)
	{
#if PLATFORM_64BITS
		return (int64_t)::_InterlockedExchangeAdd64((int64_t*)value, (int64_t)amount);
#else
		while (true)
		{
			int64_t oldValue = *value;
			if (_InterlockedCompareExchange64(value, oldValue + amount, oldValue) == oldValue) {
				return oldValue + amount;
			}
		}
#endif
	}

	static FORCEINLINE int8_t InterlockedExchange(volatile int8_t* value, int8_t exchange)
	{
		return (int8_t)::_InterlockedExchange8((char*)value, (char)exchange);
	}

	static FORCEINLINE int16_t InterlockedExchange(volatile int16_t* value, int16_t exchange)
	{
		return (int16_t)::_InterlockedExchange16((short*)value, (short)exchange);
	}

	static FORCEINLINE int32_t InterlockedExchange(volatile int32_t* value, int32_t exchange)
	{
		return (int32_t)::_InterlockedExchange((long*)value, (long)exchange);
	}

	static FORCEINLINE int64_t InterlockedExchange(volatile int64_t* value, int64_t exchange)
	{
#if PLATFORM_64BITS
		return (int64_t)::_InterlockedExchange64((long long*)value, (long long)exchange);
#else
		while (true)
		{
			int64_t oldValue = *value;
			if (_InterlockedCompareExchange64(value, exchange, oldValue) == oldValue) {
				return oldValue;
			}
		}
#endif
	}

	static FORCEINLINE void* InterlockedExchangePtr(void** dest, void* exchange)
	{
#if PLATFORM_64BITS
		return (void*)::_InterlockedExchange64((int64_t*)(dest), (int64_t)(exchange));
#else
		return (void*)::_InterlockedExchange((long*)(dest), (long)(exchange));
#endif
	}

	static FORCEINLINE int8_t InterlockedCompareExchange(volatile int8_t* dest, int8_t exchange, int8_t comparand)
	{
		return (int8_t)::_InterlockedCompareExchange8((char*)dest, (char)exchange, (char)comparand);
	}

	static FORCEINLINE int16_t InterlockedCompareExchange(volatile int16_t* dest, int16_t exchange, int16_t comparand)
	{
		return (int16_t)::_InterlockedCompareExchange16((short*)dest, (short)exchange, (short)comparand);
	}

	static FORCEINLINE int32_t InterlockedCompareExchange(volatile int32_t* dest, int32_t exchange, int32_t comparand)
	{
		return (int32_t)::_InterlockedCompareExchange((long*)dest, (long)exchange, (long)comparand);
	}

	static FORCEINLINE int64_t InterlockedCompareExchange(volatile int64_t* dest, int64_t exchange, int64_t comparand)
	{
		return (int64_t)::_InterlockedCompareExchange64(dest, exchange, comparand);
	}

	static FORCEINLINE int8_t AtomicRead(volatile const int8_t* src)
	{
		return InterlockedCompareExchange((int8_t*)src, 0, 0);
	}

	static FORCEINLINE int16_t AtomicRead(volatile const int16_t* src)
	{
		return InterlockedCompareExchange((int16_t*)src, 0, 0);
	}

	static FORCEINLINE int32_t AtomicRead(volatile const int32_t* src)
	{
		return InterlockedCompareExchange((int32_t*)src, 0, 0);
	}

	static FORCEINLINE int64_t AtomicRead(volatile const int64_t* src)
	{
		return InterlockedCompareExchange((int64_t*)src, 0, 0);
	}

	static FORCEINLINE int8_t AtomicRead_Relaxed(volatile const int8_t* src)
	{
		return *src;
	}

	static FORCEINLINE int16_t AtomicRead_Relaxed(volatile const int16_t* src)
	{
		return *src;
	}

	static FORCEINLINE int32_t AtomicRead_Relaxed(volatile const int32_t* src)
	{
		return *src;
	}

	static FORCEINLINE int64_t AtomicRead_Relaxed(volatile const int64_t* src)
	{
#if PLATFORM_64BITS
		return *src;
#else
		return InterlockedCompareExchange((volatile int64_t*)src, 0, 0);
#endif
	}

	static FORCEINLINE void AtomicStore(volatile int8_t* src, int8_t val)
	{
		InterlockedExchange(src, val);
	}

	static FORCEINLINE void AtomicStore(volatile int16_t* src, int16_t val)
	{
		InterlockedExchange(src, val);
	}

	static FORCEINLINE void AtomicStore(volatile int32_t* src, int32_t val)
	{
		InterlockedExchange(src, val);
	}

	static FORCEINLINE void AtomicStore(volatile int64_t* src, int64_t val)
	{
		InterlockedExchange(src, val);
	}

	static FORCEINLINE void AtomicStore_Relaxed(volatile int8_t* src, int8_t val)
	{
		*src = val;
	}

	static FORCEINLINE void AtomicStore_Relaxed(volatile int16_t* src, int16_t val)
	{
		*src = val;
	}

	static FORCEINLINE void AtomicStore_Relaxed(volatile int32_t* src, int32_t val)
	{
		*src = val;
	}

	static FORCEINLINE void AtomicStore_Relaxed(volatile int64_t* src, int64_t val)
	{
#if PLATFORM_64BITS
		* src = val;
#else
		InterlockedExchange(src, val);
#endif
	}

	static FORCEINLINE void* InterlockedCompareExchangePointer(void** dest, void* exchange, void* comparand)
	{
#if PLATFORM_64BITS
		return (void*)::_InterlockedCompareExchange64((int64_t*)dest, (int64_t)exchange, (int64_t)comparand);
#else
		return (void*)::_InterlockedCompareExchange((long*)dest, (long)exchange, (long)comparand);
#endif
	}
};

#endif // PLATFORM_WINDOWS