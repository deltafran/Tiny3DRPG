#pragma once

#include "CoreMath2.h"

struct Vector3;

struct IntVector
{
public:
	int32_t x;
	int32_t y;
	int32_t z;

public:
	static const IntVector ZeroValue;
	static const IntVector NoneValue;

public:

	IntVector();

	IntVector(int32_t inX, int32_t inY, int32_t inZ);

	explicit IntVector(int32_t inValue);

	explicit IntVector(Vector3 v);

	FORCEINLINE const int32_t& operator()(int32_t index) const;

	FORCEINLINE int32_t& operator()(int32_t index);

	FORCEINLINE const int32_t& operator[](int32_t index) const;

	FORCEINLINE int32_t& operator[](int32_t index);

	FORCEINLINE bool operator==(const IntVector& other) const;

	FORCEINLINE bool operator!=(const IntVector& other) const;

	FORCEINLINE IntVector& operator*=(int32_t scale);

	FORCEINLINE IntVector& operator/=(int32_t divisor);

	FORCEINLINE IntVector& operator+=(const IntVector& other);

	FORCEINLINE IntVector& operator-=(const IntVector& other);

	FORCEINLINE IntVector& operator=(const IntVector& other);

	FORCEINLINE IntVector operator*(int32_t scale) const;

	FORCEINLINE IntVector operator/(int32_t divisor) const;

	FORCEINLINE IntVector operator+(const IntVector& other) const;

	FORCEINLINE IntVector operator-(const IntVector& other) const;

	FORCEINLINE bool IsZero() const;

	FORCEINLINE int32_t GetMax() const;

	FORCEINLINE int32_t GetMin() const;

	FORCEINLINE int32_t Size() const;

	FORCEINLINE std::string ToString() const;

	FORCEINLINE static IntVector DivideAndRoundUp(IntVector lhs, int32_t divisor);

	FORCEINLINE static int32_t Num();
};

FORCEINLINE IntVector::IntVector()
	: x(0)
	, y(0)
	, z(0)
{

}

FORCEINLINE IntVector::IntVector(int32_t inX, int32_t inY, int32_t inZ)
	: x(inX)
	, y(inY)
	, z(inZ)
{

}

FORCEINLINE IntVector::IntVector(int32_t inValue)
	: x(inValue)
	, y(inValue)
	, z(inValue)
{

}

FORCEINLINE const int32_t& IntVector::operator()(int32_t index) const
{
	return (&x)[index];
}

FORCEINLINE int32_t& IntVector::operator()(int32_t index)
{
	return (&x)[index];
}

FORCEINLINE const int32_t& IntVector::operator[](int32_t index) const
{
	return (&x)[index];
}

FORCEINLINE int32_t& IntVector::operator[](int32_t index)
{
	return (&x)[index];
}

FORCEINLINE bool IntVector::operator==(const IntVector& other) const
{
	return x == other.x && y == other.y && z == other.z;
}

FORCEINLINE bool IntVector::operator!=(const IntVector& other) const
{
	return x != other.x || y != other.y || z != other.z;
}

FORCEINLINE IntVector& IntVector::operator*=(int32_t scale)
{
	x *= scale;
	y *= scale;
	z *= scale;
	return *this;
}

FORCEINLINE IntVector& IntVector::operator/=(int32_t divisor)
{
	x /= divisor;
	y /= divisor;
	z /= divisor;
	return *this;
}

FORCEINLINE IntVector& IntVector::operator+=(const IntVector& other)
{
	x += other.x;
	y += other.y;
	z += other.z;
	return *this;
}

FORCEINLINE IntVector& IntVector::operator-=(const IntVector& other)
{
	x -= other.x;
	y -= other.y;
	z -= other.z;
	return *this;
}

FORCEINLINE IntVector& IntVector::operator=(const IntVector& other)
{
	x = other.x;
	y = other.y;
	z = other.z;
	return *this;
}

FORCEINLINE IntVector IntVector::operator*(int32_t scale) const
{
	return IntVector(*this) *= scale;
}

FORCEINLINE IntVector IntVector::operator/(int32_t divisor) const
{
	return IntVector(*this) /= divisor;
}

FORCEINLINE IntVector IntVector::operator+(const IntVector& other) const
{
	return IntVector(*this) += other;
}

FORCEINLINE IntVector IntVector::operator-(const IntVector& other) const
{
	return IntVector(*this) -= other;
}

FORCEINLINE IntVector IntVector::DivideAndRoundUp(IntVector lhs, int32_t divisor)
{
	return IntVector(math::DivideAndRoundUp(lhs.x, divisor), math::DivideAndRoundUp(lhs.y, divisor), math::DivideAndRoundUp(lhs.z, divisor));
}

FORCEINLINE int32_t IntVector::GetMax() const
{
	return math::Max<int32_t>(math::Max<int32_t>(x, y), z);
}

FORCEINLINE int32_t IntVector::GetMin() const
{
	return math::Min<int32_t>(math::Min<int32_t>(x, y), z);
}

FORCEINLINE int32_t IntVector::Num()
{
	return 3;
}

FORCEINLINE int32_t IntVector::Size() const
{
	int64_t x64 = (int64_t)x;
	int64_t y64 = (int64_t)y;
	int64_t z64 = (int64_t)z;
	return int32_t(math::Sqrt(x64 * x64 + y64 * y64 + z64 * z64));
}

FORCEINLINE bool IntVector::IsZero() const
{
	return *this == ZeroValue;
}

FORCEINLINE std::string IntVector::ToString() const
{
	return StringUtils::Printf("x=%d y=%d z=%d", x, y, z);
}

struct IntVector4
{
public:
	int32_t x;
	int32_t y;
	int32_t z;
	int32_t w;

public:
	FORCEINLINE IntVector4()
	{

	}

	FORCEINLINE IntVector4(int32_t inX, int32_t inY, int32_t inZ, int32_t InW)
		: x(inX)
		, y(inY)
		, z(inZ)
		, w(InW)
	{

	}

	FORCEINLINE explicit IntVector4(int32_t inValue)
		: x(inValue)
		, y(inValue)
		, z(inValue)
		, w(inValue)
	{

	}

	FORCEINLINE const int32_t& operator[](int32_t index) const
	{
		return (&x)[index];
	}

	FORCEINLINE int32_t& operator[](int32_t index)
	{
		return (&x)[index];
	}

	FORCEINLINE bool operator==(const IntVector4& other) const
	{
		return x == other.x && y == other.y && z == other.z && w == other.w;
	}

	FORCEINLINE bool operator!=(const IntVector4& other) const
	{
		return x != other.x || y != other.y || z != other.z || w != other.w;
	}
};

struct UintVector4
{
public:
	uint32_t x;
	uint32_t y;
	uint32_t z;
	uint32_t w;

public:
	FORCEINLINE UintVector4()
	{

	}

	FORCEINLINE UintVector4(uint32_t inX, uint32_t inY, uint32_t inZ, uint32_t InW)
		: x(inX)
		, y(inY)
		, z(inZ)
		, w(InW)
	{

	}

	FORCEINLINE explicit UintVector4(uint32_t inValue)
		: x(inValue)
		, y(inValue)
		, z(inValue)
		, w(inValue)
	{

	}

	FORCEINLINE const uint32_t& operator[](int32_t index) const
	{
		return (&x)[index];
	}

	FORCEINLINE uint32_t& operator[](int32_t index)
	{
		return (&x)[index];
	}

	FORCEINLINE bool operator==(const UintVector4& other) const
	{
		return x == other.x && y == other.y && z == other.z && w == other.w;
	}

	FORCEINLINE bool operator!=(const UintVector4& other) const
	{
		return x != other.x || y != other.y || z != other.z || w != other.w;
	}
};