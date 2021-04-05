#pragma once

template <typename T>
FORCEINLINE constexpr T Align(T val, uint64_t alignment) noexcept
{
    return (T)(((uint64_t)val + alignment - 1) & ~(alignment - 1));
}

template <typename T>
FORCEINLINE constexpr T AlignDown(T val, uint64_t alignment) noexcept
{
    return (T)(((uint64_t)val) & ~(alignment - 1));
}

template <typename T>
FORCEINLINE constexpr bool IsAligned(T val, uint64_t alignment) noexcept
{
    return !((uint64_t)val & (alignment - 1));
}

template <typename T>
FORCEINLINE constexpr T AlignArbitrary(T val, uint64_t alignment) noexcept
{
    return (T)((((uint64_t)val + alignment - 1) / alignment) * alignment);
}