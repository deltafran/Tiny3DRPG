#pragma once

#include "CoreMath2.h"

struct IntPoint
{
public:

	int32_t x;
	int32_t y;

	static const IntPoint ZeroValue;
	static const IntPoint NoneValue;

public:

	IntPoint();

	IntPoint(int32_t inX, int32_t inY);

	FORCEINLINE const int32_t& operator()(int32_t pointIndex) const;

	FORCEINLINE int32_t& operator()(int32_t pointIndex);

	FORCEINLINE int32_t& operator[](int32_t index);

	FORCEINLINE int32_t operator[](int32_t index) const;

	FORCEINLINE IntPoint& operator=(const IntPoint& other);

	FORCEINLINE bool operator==(const IntPoint& other) const;

	FORCEINLINE bool operator!=(const IntPoint& other) const;

	FORCEINLINE IntPoint& operator*=(int32_t scale);

	FORCEINLINE IntPoint& operator/=(int32_t divisor);

	FORCEINLINE IntPoint& operator+=(const IntPoint& other);

	FORCEINLINE IntPoint& operator-=(const IntPoint& other);

	FORCEINLINE IntPoint& operator/=(const IntPoint& other);

	FORCEINLINE IntPoint operator*(int32_t scale) const;

	FORCEINLINE IntPoint operator/(int32_t divisor) const;

	FORCEINLINE IntPoint operator+(const IntPoint& other) const;

	FORCEINLINE IntPoint operator-(const IntPoint& other) const;

	FORCEINLINE IntPoint operator/(const IntPoint& other) const;

	FORCEINLINE IntPoint ComponentMin(const IntPoint& other) const;

	FORCEINLINE IntPoint ComponentMax(const IntPoint& other) const;

	FORCEINLINE int32_t GetMax() const;

	FORCEINLINE int32_t GetMin() const;

	FORCEINLINE int32_t Size() const;

	FORCEINLINE int32_t SizeSquared() const;

	FORCEINLINE std::string ToString() const;

	static FORCEINLINE int32_t Num();

	static FORCEINLINE IntPoint DivideAndRoundUp(IntPoint lhs, int32_t divisor);

	static FORCEINLINE IntPoint DivideAndRoundUp(IntPoint lhs, IntPoint divisor);

	static FORCEINLINE IntPoint DivideAndRoundDown(IntPoint lhs, int32_t divisor);
};

FORCEINLINE IntPoint::IntPoint()
	: x(0)
	, y(0)
{

}

FORCEINLINE IntPoint::IntPoint(int32_t inX, int32_t inY)
	: x(inX)
	, y(inY)
{

}

FORCEINLINE const int32_t& IntPoint::operator()(int32_t pointIndex) const
{
	return (&x)[pointIndex];
}

FORCEINLINE int32_t& IntPoint::operator()(int32_t pointIndex)
{
	return (&x)[pointIndex];
}

FORCEINLINE int32_t IntPoint::Num()
{
	return 2;
}

FORCEINLINE bool IntPoint::operator==(const IntPoint& other) const
{
	return x == other.x && y == other.y;
}

FORCEINLINE bool IntPoint::operator!=(const IntPoint& other) const
{
	return x != other.x || y != other.y;
}

FORCEINLINE IntPoint& IntPoint::operator*=(int32_t scale)
{
	x *= scale;
	y *= scale;
	return *this;
}

FORCEINLINE IntPoint& IntPoint::operator/=(int32_t divisor)
{
	x /= divisor;
	y /= divisor;
	return *this;
}

FORCEINLINE IntPoint& IntPoint::operator+=(const IntPoint& other)
{
	x += other.x;
	y += other.y;
	return *this;
}

FORCEINLINE IntPoint& IntPoint::operator-=(const IntPoint& other)
{
	x -= other.x;
	y -= other.y;
	return *this;
}

FORCEINLINE IntPoint& IntPoint::operator/=(const IntPoint& other)
{
	x /= other.x;
	y /= other.y;
	return *this;
}

FORCEINLINE IntPoint& IntPoint::operator=(const IntPoint& other)
{
	x = other.x;
	y = other.y;
	return *this;
}

FORCEINLINE IntPoint IntPoint::operator*(int32_t scale) const
{
	return IntPoint(*this) *= scale;
}

FORCEINLINE IntPoint IntPoint::operator/(int32_t divisor) const
{
	return IntPoint(*this) /= divisor;
}

FORCEINLINE int32_t& IntPoint::operator[](int32_t index)
{
	return ((index == 0) ? x : y);
}

FORCEINLINE int32_t IntPoint::operator[](int32_t index) const
{
	return ((index == 0) ? x : y);
}

FORCEINLINE IntPoint IntPoint::ComponentMin(const IntPoint& other) const
{
	return IntPoint(std::min(x, other.x), std::min(y, other.y));
}

FORCEINLINE IntPoint IntPoint::ComponentMax(const IntPoint& other) const
{
	return IntPoint(std::max(x, other.x), std::max(y, other.y));
}

FORCEINLINE IntPoint IntPoint::DivideAndRoundUp(IntPoint lhs, int32_t divisor)
{
	return IntPoint(math::DivideAndRoundUp(lhs.x, divisor), math::DivideAndRoundUp(lhs.y, divisor));
}

FORCEINLINE IntPoint IntPoint::DivideAndRoundUp(IntPoint lhs, IntPoint divisor)
{
	return IntPoint(math::DivideAndRoundUp(lhs.x, divisor.x), math::DivideAndRoundUp(lhs.y, divisor.y));
}

FORCEINLINE IntPoint IntPoint::DivideAndRoundDown(IntPoint lhs, int32_t divisor)
{
	return IntPoint(math::DivideAndRoundDown(lhs.x, divisor), math::DivideAndRoundDown(lhs.y, divisor));
}

FORCEINLINE IntPoint IntPoint::operator+(const IntPoint& other) const
{
	return IntPoint(*this) += other;
}

FORCEINLINE IntPoint IntPoint::operator-(const IntPoint& other) const
{
	return IntPoint(*this) -= other;
}

FORCEINLINE IntPoint IntPoint::operator/(const IntPoint& other) const
{
	return IntPoint(*this) /= other;
}

FORCEINLINE int32_t IntPoint::GetMax() const
{
	return std::max(x, y);
}

FORCEINLINE int32_t IntPoint::GetMin() const
{
	return std::min(x, y);
}

FORCEINLINE int32_t IntPoint::Size() const
{
	int64_t x64 = (int64_t)x;
	int64_t y64 = (int64_t)x;
	return int32_t(std::sqrt(float(x64 * x64 + y64 * y64)));
}

FORCEINLINE int32_t IntPoint::SizeSquared() const
{
	return x * x + y * y;
}

FORCEINLINE std::string IntPoint::ToString() const
{
	return "X=" + std::to_string(x) + " Y=" + std::to_string(y);
}