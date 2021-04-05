#pragma once

struct Rotator;
struct Plane;
struct Vector3;
struct Vector2;
struct Quat;
struct Matrix4x4;

#if PLATFORM_WINDOWS
#	pragma intrinsic(_BitScanReverse)
#endif

namespace math
{
	FORCEINLINE int32_t TruncToInt(float f)
	{
#if PLATFORM_WINDOWS
		return _mm_cvtt_ss2si(_mm_set_ss(f));
#else
		return (int32_t)(f);
#endif
	}

	FORCEINLINE float TruncToFloat(float f)
	{
		return (float)TruncToInt(f);
	}

	FORCEINLINE int32_t FloorToInt(float f)
	{
#if PLATFORM_WINDOWS
		return _mm_cvt_ss2si(_mm_set_ss(f + f - 0.5f)) >> 1;
#else
		return TruncToInt(floorf(f));
#endif
	}

	FORCEINLINE float FloorToFloat(float f)
	{
#if PLATFORM_WINDOWS
		return (float)FloorToInt(f);
#else
		return floorf(f);
#endif
	}

	FORCEINLINE double FloorToDouble(double f)
	{
		return floor(f);
	}

	FORCEINLINE int32_t RoundToInt(float f)
	{
#if PLATFORM_WINDOWS
		return _mm_cvt_ss2si(_mm_set_ss(f + f + 0.5f)) >> 1;
#else
		return FloorToInt(f + 0.5f);
#endif
	}

	FORCEINLINE float RoundToFloat(float f)
	{
#if PLATFORM_WINDOWS
		return (float)RoundToInt(f);
#else
		return FloorToFloat(f + 0.5f);
#endif
	}

	FORCEINLINE double RoundToDouble(double f)
	{
		return FloorToDouble(f + 0.5);
	}

	FORCEINLINE int32_t CeilToInt(float f)
	{
#if PLATFORM_WINDOWS
		return -(_mm_cvt_ss2si(_mm_set_ss(-0.5f - (f + f))) >> 1);
#else
		return TruncToInt(ceilf(f));
#endif
	}

	FORCEINLINE float CeilToFloat(float f)
	{
#if PLATFORM_WINDOWS
		return (float)CeilToInt(f);
#else
		return ceilf(f);
#endif
	}

	FORCEINLINE double CeilToDouble(double f)
	{
		return ceil(f);
	}

	FORCEINLINE float Fractional(float value)
	{
		return value - TruncToFloat(value);
	}

	FORCEINLINE float Frac(float value)
	{
		return value - FloorToFloat(value);
	}

	FORCEINLINE float Modf(const float value, float* outIntPart)
	{
		return modff(value, outIntPart);
	}

	FORCEINLINE double Modf(const double value, double* outIntPart)
	{
		return modf(value, outIntPart);
	}

	FORCEINLINE float Exp(float value)
	{
		return expf(value);
	}

	FORCEINLINE float Exp2(float value)
	{
		return powf(2.f, value);
	}

	FORCEINLINE float Loge(float value)
	{
		return logf(value);
	}

	FORCEINLINE float LogX(float base, float value)
	{
		return Loge(value) / Loge(base);
	}

	FORCEINLINE float Fmod(float x, float y)
	{
		if (fabsf(y) <= 1.e-8f) {
			return 0.f;
		}

		const float quotient = TruncToFloat(x / y);
		float intPortion = y * quotient;

		if (fabsf(intPortion) > fabsf(x)) {
			intPortion = x;
		}

		const float result = x - intPortion;
		return result;
	}

	FORCEINLINE float Sin(float value)
	{
		return sinf(value);
	}

	FORCEINLINE float Asin(float value)
	{
		return asinf((value < -1.f) ? -1.f : ((value < 1.f) ? value : 1.f));
	}

	FORCEINLINE float Sinh(float value)
	{
		return sinhf(value);
	}

	FORCEINLINE float Cos(float value)
	{
		return cosf(value);
	}

	FORCEINLINE float Acos(float value)
	{
		return acosf((value < -1.f) ? -1.f : ((value < 1.f) ? value : 1.f));
	}

	FORCEINLINE float Tan(float value)
	{
		return tanf(value);
	}

	FORCEINLINE float Atan(float value)
	{
		return atanf(value);
	}

	FORCEINLINE float Sqrt(float value)
	{
		return sqrtf(value);
	}

	FORCEINLINE float Pow(float a, float b)
	{
		return powf(a, b);
	}

	FORCEINLINE float InvSqrt(float f)
	{
#if PLATFORM_WINDOWS
		const __m128 fOneHalf = _mm_set_ss(0.5f);
		__m128 y0, x0, x1, x2, fOver2;
		float temp;

		y0 = _mm_set_ss(f);
		x0 = _mm_rsqrt_ss(y0);
		fOver2 = _mm_mul_ss(y0, fOneHalf);

		x1 = _mm_mul_ss(x0, x0);
		x1 = _mm_sub_ss(fOneHalf, _mm_mul_ss(fOver2, x1));
		x1 = _mm_add_ss(x0, _mm_mul_ss(x0, x1));

		x2 = _mm_mul_ss(x1, x1);
		x2 = _mm_sub_ss(fOneHalf, _mm_mul_ss(fOver2, x2));
		x2 = _mm_add_ss(x1, _mm_mul_ss(x1, x2));

		_mm_store_ss(&temp, x2);
		return temp;
#else
		return 1.0f / sqrtf(f);
#endif
	}

	FORCEINLINE float InvSqrtEst(float f)
	{
#if PLATFORM_WINDOWS
		const __m128 fOneHalf = _mm_set_ss(0.5f);
		__m128 y0, x0, x1, fOver2;
		float temp;

		y0 = _mm_set_ss(f);
		x0 = _mm_rsqrt_ss(y0);
		fOver2 = _mm_mul_ss(y0, fOneHalf);

		x1 = _mm_mul_ss(x0, x0);
		x1 = _mm_sub_ss(fOneHalf, _mm_mul_ss(fOver2, x1));
		x1 = _mm_add_ss(x0, _mm_mul_ss(x0, x1));

		_mm_store_ss(&temp, x1);
		return temp;
#else
		return InvSqrt(f);
#endif
	}

	FORCEINLINE bool IsNaN(float f)
	{
#if PLATFORM_WINDOWS
		return _isnan(f) != 0;
#else
		return ((*(uint32_t*)&f) & 0x7FFFFFFF) > 0x7F800000;
#endif
	}

	FORCEINLINE bool IsFinite(float f)
	{
#if PLATFORM_WINDOWS
		return _finite(f) != 0;
#else
		return ((*(uint32_t*)&f) & 0x7F800000) != 0x7F800000;
#endif
	}

	FORCEINLINE bool IsNegativeFloat(const float& f)
	{
		return ((*(uint32_t*)&f) >= (uint32_t)0x80000000);
	}

	FORCEINLINE bool IsNegativeDouble(const double& f)
	{
		return ((*(uint64_t*)&f) >= (uint64_t)0x8000000000000000);
	}

	FORCEINLINE int32_t Rand()
	{
		return rand();
	}

	FORCEINLINE void RandInit(int32_t seed)
	{
		srand(seed);
	}

	FORCEINLINE float FRand()
	{
		return Rand() / (float)RAND_MAX;
	}

	FORCEINLINE uint32_t FloorLog2(uint32_t value)
	{
#if PLATFORM_WINDOWS
		unsigned long log2;
		if (_BitScanReverse(&log2, value) != 0)
		{
			return log2;
		}

		return 0;
#else
		uint32_t pos = 0;

		if (value >= 1 << 16)
		{
			value >>= 16;
			pos += 16;
		}

		if (value >= 1 << 8)
		{
			value >>= 8;
			pos += 8;
		}

		if (value >= 1 << 4)
		{
			value >>= 4;
			pos += 4;
		}

		if (value >= 1 << 2)
		{
			value >>= 2;
			pos += 2;
		}

		if (value >= 1 << 1)
		{
			pos += 1;
		}

		return (value == 0) ? 0 : pos;
#endif
	}

	FORCEINLINE uint64_t FloorLog2_64(uint64_t value)
	{
		uint64_t pos = 0;
		if (value >= 1ull << 32)
		{
			value >>= 32;
			pos += 32;
		}

		if (value >= 1ull << 16)
		{
			value >>= 16;
			pos += 16;
		}

		if (value >= 1ull << 8)
		{
			value >>= 8;
			pos += 8;
		}

		if (value >= 1ull << 4)
		{
			value >>= 4;
			pos += 4;
		}

		if (value >= 1ull << 2)
		{
			value >>= 2;
			pos += 2;
		}

		if (value >= 1ull << 1)
		{
			pos += 1;
		}

		return (value == 0) ? 0 : pos;
	}

	FORCEINLINE uint32_t CountLeadingZeros(uint32_t value)
	{
#if PLATFORM_WINDOWS
		unsigned long log2;
		if (_BitScanReverse(&log2, value) != 0)
		{
			return 31 - log2;
		}

		return 32;
#else
		if (value == 0) {
			return 32;
		}
		return 31 - FloorLog2(value);
#endif
	}

	FORCEINLINE uint64_t CountLeadingZeros64(uint64_t value)
	{
#if PLATFORM_WINDOWS
		unsigned long log2;
		if (_BitScanReverse64(&log2, value) != 0)
		{
			return 63 - log2;
		}

		return 64;
#else
		if (value == 0) {
			return 64;
		}
		return 63 - FloorLog2_64(value);
#endif
	}

	FORCEINLINE uint32_t CountTrailingZeros(uint32_t value)
	{
#if PLATFORM_WINDOWS
		if (value == 0)
		{
			return 32;
		}

		unsigned long bitIndex;
		_BitScanForward(&bitIndex, value);
		return bitIndex;
#else
		if (value == 0) {
			return 32;
		}

		uint32_t result = 0;

		while ((value & 1) == 0)
		{
			value >>= 1;
			++result;
		}

		return result;
#endif
	}

	FORCEINLINE uint64_t CountTrailingZeros64(uint64_t value)
	{
#if PLATFORM_WINDOWS
		if (value == 0)
			return 64;
		unsigned long bitIndex;
		_BitScanForward64(&bitIndex, value);
		return bitIndex;
#else
		if (value == 0)
			return 64;

		uint64_t result = 0;
		while ((value & 1) == 0)
		{
			value >>= 1;
			++result;
		}

		return result;
#endif
	}


	FORCEINLINE uint32_t CeilLogTwo(uint32_t value)
	{
		int32_t bitmask = ((int32_t)(CountLeadingZeros(value) << 26)) >> 31;
		return (32 - CountLeadingZeros(value - 1)) & (~bitmask);
	}

	FORCEINLINE uint64_t CeilLogTwo64(uint64_t value)
	{
		int64_t bitmask = ((int64_t)(CountLeadingZeros64(value) << 57)) >> 63;
		return (64 - CountLeadingZeros64(value - 1)) & (~bitmask);
	}

	FORCEINLINE uint32_t RoundUpToPowerOfTwo(uint32_t value)
	{
		return 1 << CeilLogTwo(value);
	}

	FORCEINLINE uint64_t RoundUpToPowerOfTwo64(uint64_t value)
	{
		return uint64_t(1) << CeilLogTwo64(value);
	}

	FORCEINLINE uint32_t MortonCode2(uint32_t x)
	{
		x &= 0x0000ffff;
		x = (x ^ (x << 8)) & 0x00ff00ff;
		x = (x ^ (x << 4)) & 0x0f0f0f0f;
		x = (x ^ (x << 2)) & 0x33333333;
		x = (x ^ (x << 1)) & 0x55555555;
		return x;
	}

	FORCEINLINE uint32_t ReverseMortonCode2(uint32_t x)
	{
		x &= 0x55555555;
		x = (x ^ (x >> 1)) & 0x33333333;
		x = (x ^ (x >> 2)) & 0x0f0f0f0f;
		x = (x ^ (x >> 4)) & 0x00ff00ff;
		x = (x ^ (x >> 8)) & 0x0000ffff;
		return x;
	}

	FORCEINLINE uint32_t MortonCode3(uint32_t x)
	{
		x &= 0x000003ff;
		x = (x ^ (x << 16)) & 0xff0000ff;
		x = (x ^ (x << 8)) & 0x0300f00f;
		x = (x ^ (x << 4)) & 0x030c30c3;
		x = (x ^ (x << 2)) & 0x09249249;
		return x;
	}

	FORCEINLINE uint32_t ReverseMortonCode3(uint32_t x)
	{
		x &= 0x09249249;
		x = (x ^ (x >> 2)) & 0x030c30c3;
		x = (x ^ (x >> 4)) & 0x0300f00f;
		x = (x ^ (x >> 8)) & 0xff0000ff;
		x = (x ^ (x >> 16)) & 0x000003ff;
		return x;
	}

	constexpr FORCEINLINE float FloatSelect(float comparand, float valueGEZero, float valueLTZero)
	{
		return comparand >= 0.f ? valueGEZero : valueLTZero;
	}

	constexpr FORCEINLINE double FloatSelect(double comparand, double valueGEZero, double valueLTZero)
	{
		return comparand >= 0.f ? valueGEZero : valueLTZero;
	}

	template< class T >
	constexpr FORCEINLINE T Abs(const T a)
	{
		return (a >= (T)0) ? a : -a;
	}

	template<>
	FORCEINLINE float Abs(const float a)
	{
		return fabsf(a);
	}

	template< class T >
	constexpr FORCEINLINE T Sign(const T a)
	{
		return (a > (T)0) ? (T)1 : ((a < (T)0) ? (T)-1 : (T)0);
	}

	template< class T >
	constexpr FORCEINLINE T Max(const T a, const T b)
	{
		return (a >= b) ? a : b;
	}

	template< class T >
	constexpr FORCEINLINE T Min(const T a, const T b)
	{
		return (a <= b) ? a : b;
	}

	FORCEINLINE int32_t CountBits(uint64_t bits)
	{
		bits -= (bits >> 1) & 0x5555555555555555ull;
		bits = (bits & 0x3333333333333333ull) + ((bits >> 2) & 0x3333333333333333ull);
		bits = (bits + (bits >> 4)) & 0x0f0f0f0f0f0f0f0full;
		return (bits * 0x0101010101010101) >> 56;
	}

	template< class T >
	FORCEINLINE T Min(const std::vector<T>& values, int32_t* minIndex = NULL)
	{
		if (values.size() == 0)
		{
			if (minIndex) {
				*minIndex = -1;
			}
			return T();
		}

		T curMin = values[0];
		int32_t curMinIndex = 0;
		for (int32_t v = 1; v < values.size(); ++v)
		{
			const T value = values[v];
			if (value < curMin)
			{
				curMin = value;
				curMinIndex = v;
			}
		}

		if (minIndex) {
			*minIndex = curMinIndex;
		}

		return curMin;
	}

	template< class T >
	FORCEINLINE T Max(const std::vector<T>& values, int32_t* maxIndex = NULL)
	{
		if (values.size() == 0)
		{
			if (maxIndex) {
				*maxIndex = -1;
			}
			return T();
		}

		T curMax = values[0];
		int32_t curMaxIndex = 0;
		for (int32_t v = 1; v < values.size(); ++v)
		{
			const T value = values[v];
			if (curMax < value)
			{
				curMax = value;
				curMaxIndex = v;
			}
		}

		if (maxIndex) {
			*maxIndex = curMaxIndex;
		}

		return curMax;
	}

	void SRandInit(int32_t seed);

	int32_t GetRandSeed();

	float SRand();

	float Atan2(float y, float x);

} // namespace math