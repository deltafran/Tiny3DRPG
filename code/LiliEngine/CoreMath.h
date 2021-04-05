#pragma once

namespace math
{
	template< class T >
	static FORCEINLINE T Clamp(const T x, const T inMin, const T inMax)
	{
		return x < inMin ? inMin : x < inMax ? x : inMax;
	}
} // namespace math