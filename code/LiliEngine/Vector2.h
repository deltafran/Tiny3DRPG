#pragma once

#include "IntPoint.h"
#include "StringUtils.h"
#include "FloatConversion.h"
#include "MathPodTypes.h"

class Vector2 final
{
public:
	static const Vector2 Zero;
	static const Vector2 Unit;

	FORCEINLINE Vector2() noexcept;
	FORCEINLINE Vector2(float inX, float inY) noexcept;
	FORCEINLINE Vector2(const Vector2& inV) noexcept;
	FORCEINLINE Vector2(const IntPoint& inPos) noexcept;
	FORCEINLINE Vector2(const v2& inV) noexcept;

	FORCEINLINE Vector2& operator=(const Vector2&) noexcept;

	FORCEINLINE Vector2 operator+(const Vector2& v) const noexcept;
	FORCEINLINE Vector2 operator-(const Vector2& v) const noexcept;
	FORCEINLINE Vector2 operator*(float scale) const noexcept;
	FORCEINLINE Vector2 operator*(const Vector2& v) const noexcept;
	FORCEINLINE Vector2 operator+(float a) const noexcept;
	FORCEINLINE Vector2 operator-(float a) const noexcept;
	FORCEINLINE Vector2 operator/(float scale) const noexcept;
	FORCEINLINE Vector2 operator/(const Vector2& v) const noexcept;

	FORCEINLINE float operator|(const Vector2& v) const noexcept;
	FORCEINLINE float operator^(const Vector2& v) const noexcept;

	FORCEINLINE bool operator==(const Vector2& v) const noexcept;
	FORCEINLINE bool operator!=(const Vector2& v) const noexcept;
	FORCEINLINE bool operator<(const Vector2& other) const noexcept;
	FORCEINLINE bool operator>(const Vector2& other) const noexcept;
	FORCEINLINE bool operator<=(const Vector2& other) const noexcept;
	FORCEINLINE bool operator>=(const Vector2& other) const noexcept;

	FORCEINLINE Vector2 operator-() const noexcept;

	FORCEINLINE Vector2 operator+=(const Vector2& v) noexcept;
	FORCEINLINE Vector2 operator-=(const Vector2& v) noexcept;
	FORCEINLINE Vector2 operator*=(float scale) noexcept;
	FORCEINLINE Vector2 operator*=(const Vector2& v) noexcept;
	FORCEINLINE Vector2 operator/=(float v) noexcept;
	FORCEINLINE Vector2 operator/=(const Vector2& v) noexcept;

	FORCEINLINE float& operator[](int32_t index) noexcept;
	FORCEINLINE const float& operator[](int32_t index) const noexcept;

	FORCEINLINE static float DotProduct(const Vector2& a, const Vector2& b) noexcept;

	FORCEINLINE static float DistSquared(const Vector2& v1, const Vector2& v2) noexcept;

	FORCEINLINE static float Distance(const Vector2& v1, const Vector2& v2) noexcept;

	FORCEINLINE static float CrossProduct(const Vector2& a, const Vector2& b) noexcept;

	FORCEINLINE bool Equals(const Vector2& v, float tolerance = KINDA_SMALL_NUMBER) const noexcept;

	FORCEINLINE void Set(float inX, float inY) noexcept;

	FORCEINLINE  Vector2& Scale(const Vector2& inV) noexcept;

	FORCEINLINE float GetMax() const noexcept;

	FORCEINLINE float GetAbsMax() const noexcept;

	FORCEINLINE float GetMin() const noexcept;

	FORCEINLINE float Size() const noexcept;

	FORCEINLINE float SizeSquared() const noexcept;

	FORCEINLINE Vector2 GetRotated(float angleDeg) const noexcept;

	FORCEINLINE Vector2 GetSafeNormal(float tolerance = SMALL_NUMBER) const noexcept;

	FORCEINLINE void Normalize(float tolerance = SMALL_NUMBER) noexcept;

	FORCEINLINE bool IsNearlyZero(float tolerance = KINDA_SMALL_NUMBER) const noexcept;

	FORCEINLINE void ToDirectionAndLength(Vector2& outDir, float& outLength) const noexcept;

	FORCEINLINE bool IsZero() const noexcept;

	FORCEINLINE IntPoint GetIntPoint() const noexcept;

	FORCEINLINE Vector2 RoundToVector() const noexcept;

	FORCEINLINE Vector2 ClampAxes(float minAxisVal, float maxAxisVal) const noexcept;

	FORCEINLINE Vector2 GetSignVector() const noexcept;

	FORCEINLINE Vector2 GetAbs() const noexcept;

	FORCEINLINE std::string ToString() const noexcept;

	FORCEINLINE bool ContainsNaN() const noexcept
	{
		return (!math::IsFinite(x) || !math::IsFinite(y));
	}

	float x;
	float y;
};

FORCEINLINE Vector2::Vector2() noexcept
	: x(0.0f)
	, y(0.0f)
{
}

FORCEINLINE Vector2::Vector2(float inX, float inY) noexcept
	: x(inX)
	, y(inY)
{
}

FORCEINLINE Vector2::Vector2(const Vector2& inV) noexcept
	: x(inV.x)
	, y(inV.y)
{
}

FORCEINLINE Vector2::Vector2(const IntPoint& inPos) noexcept
	: x((float)inPos.x)
	, y((float)inPos.y)
{
}

FORCEINLINE Vector2::Vector2(const v2& inV) noexcept
	: x(inV.x)
	, y(inV.y)
{
}

FORCEINLINE Vector2& Vector2::operator=(const Vector2 &v) noexcept
{
	x = v.x;
	y = v.y;
	return *this;
}

FORCEINLINE Vector2 operator*(float scale, const Vector2& v) noexcept
{
	return v.operator*(scale);
}

FORCEINLINE Vector2 Vector2::operator+(const Vector2& v) const noexcept
{
	return Vector2(x + v.x, y + v.y);
}

FORCEINLINE Vector2 Vector2::operator-(const Vector2& v) const noexcept
{
	return Vector2(x - v.x, y - v.y);
}

FORCEINLINE Vector2 Vector2::operator*(float scale) const noexcept
{
	return Vector2(x * scale, y * scale);
}

FORCEINLINE Vector2 Vector2::operator/(float scale) const noexcept
{
	assert(math::CompareApproximately(scale, 0.0F));
	const float invScale = 1.f / scale;
	return Vector2(x * invScale, y * invScale);
}

FORCEINLINE Vector2 Vector2::operator+(float a) const noexcept
{
	return Vector2(x + a, y + a);
}

FORCEINLINE Vector2 Vector2::operator-(float a) const noexcept
{
	return Vector2(x - a, y - a);
}

FORCEINLINE Vector2 Vector2::operator*(const Vector2& v) const noexcept
{
	return Vector2(x * v.x, y * v.y);
}

FORCEINLINE Vector2 Vector2::operator/(const Vector2& v) const noexcept
{
	assert(math::CompareApproximately(v.x, 0.0F));
	assert(math::CompareApproximately(v.y, 0.0F));
	return Vector2(x / v.x, y / v.y);
}

FORCEINLINE float Vector2::operator|(const Vector2& v) const noexcept
{
	return x * v.x + y * v.y;
}

FORCEINLINE float Vector2::operator^(const Vector2& v) const noexcept
{
	return x * v.y - y * v.x;
}

FORCEINLINE float Vector2::DotProduct(const Vector2& a, const Vector2& b) noexcept
{
	return a | b;
}

FORCEINLINE float Vector2::DistSquared(const Vector2& v1, const Vector2& v2) noexcept
{
	return math::Square(v2.x - v1.x) + math::Square(v2.y - v1.y);
}

FORCEINLINE float Vector2::Distance(const Vector2& v1, const Vector2& v2) noexcept
{
	return math::Sqrt(Vector2::DistSquared(v1, v2));
}

FORCEINLINE float Vector2::CrossProduct(const Vector2& a, const Vector2& b) noexcept
{
	return a ^ b;
}

FORCEINLINE bool Vector2::operator==(const Vector2& v) const noexcept
{
	return x == v.x && y == v.y;
}

FORCEINLINE bool Vector2::operator!=(const Vector2& v) const noexcept
{
	return x != v.x || y != v.y;
}

FORCEINLINE bool Vector2::operator<(const Vector2& other) const noexcept
{
	return x < other.x&& y < other.y;
}

FORCEINLINE bool Vector2::operator>(const Vector2& other) const noexcept
{
	return x > other.x && y > other.y;
}

FORCEINLINE bool Vector2::operator<=(const Vector2& other) const noexcept
{
	return x <= other.x && y <= other.y;
}

FORCEINLINE bool Vector2::operator>=(const Vector2& other) const noexcept
{
	return x >= other.x && y >= other.y;
}

FORCEINLINE bool Vector2::Equals(const Vector2& v, float tolerance) const noexcept
{
	return math::Abs(x - v.x) <= tolerance && math::Abs(y - v.y) <= tolerance;
}

FORCEINLINE Vector2 Vector2::operator-() const noexcept
{
	return Vector2(-x, -y);
}

FORCEINLINE Vector2 Vector2::operator+=(const Vector2& v) noexcept
{
	x += v.x;
	y += v.y;
	return *this;
}

FORCEINLINE Vector2 Vector2::operator-=(const Vector2& v) noexcept
{
	x -= v.x;
	y -= v.y;
	return *this;
}

FORCEINLINE Vector2 Vector2::operator*=(float scale) noexcept
{
	x *= scale;
	y *= scale;
	return *this;
}

FORCEINLINE Vector2 Vector2::operator/=(float v) noexcept
{
	assert(math::CompareApproximately(v, 0.0F));
	const float invF = 1.f / v;
	x *= invF;
	y *= invF;
	return *this;
}

FORCEINLINE Vector2 Vector2::operator*=(const Vector2& v) noexcept
{
	x *= v.x;
	y *= v.y;
	return *this;
}

FORCEINLINE Vector2 Vector2::operator/=(const Vector2& v) noexcept
{
	assert(math::CompareApproximately(v.x, 0.0F));
	assert(math::CompareApproximately(v.y, 0.0F));
	x /= v.x;
	y /= v.y;
	return *this;
}

FORCEINLINE float& Vector2::operator[](int32_t index) noexcept
{
	assert(index < 0 || index > 1);
	return ((index == 0) ? x : y);
}

FORCEINLINE const float& Vector2::operator[](int32_t index) const noexcept
{
	assert(index < 0 || index > 1);
	return ((index == 0) ? x : y);
}

FORCEINLINE void Vector2::Set(float inX, float inY) noexcept
{
	x = inX;
	y = inY;
}

inline Vector2& Vector2::Scale(const Vector2& inV) noexcept
{
	x *= inV.x; 
	y *= inV.y; 
	return *this;
}

FORCEINLINE float Vector2::GetMax() const noexcept
{
	return math::Max(x, y);
}

FORCEINLINE float Vector2::GetAbsMax() const noexcept
{
	return math::Max(math::Abs(x), math::Abs(y));
}

FORCEINLINE float Vector2::GetMin() const noexcept
{
	return math::Min(x, y);
}

FORCEINLINE float Vector2::Size() const noexcept
{
	return math::Sqrt(x * x + y * y);
}

FORCEINLINE float Vector2::SizeSquared() const noexcept
{
	return x * x + y * y;
}

FORCEINLINE Vector2 Vector2::GetRotated(const float angleDeg) const noexcept
{
	float s, c;
	math::SinCos(&s, &c, math::DegreesToRadians(angleDeg));
	return Vector2(c * x - s * y, s * x + c * y);
}

FORCEINLINE Vector2 Vector2::GetSafeNormal(float tolerance) const noexcept
{
	const float squareSum = x * x + y * y;
	if (squareSum > tolerance)
	{
		const float scale = math::InvSqrt(squareSum);
		return Vector2(x * scale, y * scale);
	}
	return Vector2(0.f, 0.f);
}

FORCEINLINE void Vector2::Normalize(float tolerance) noexcept
{
	const float squareSum = x * x + y * y;
	if (squareSum > tolerance)
	{
		const float scale = math::InvSqrt(squareSum);
		x *= scale;
		y *= scale;
		return;
	}
	x = 0.0f;
	y = 0.0f;
}

FORCEINLINE void Vector2::ToDirectionAndLength(Vector2& outDir, float& outLength) const noexcept
{
	outLength = Size();
	if (outLength > SMALL_NUMBER)
	{
		float oneOverLength = 1.0f / outLength;
		outDir = Vector2(x * oneOverLength, y * oneOverLength);
	}
	else
	{
		outDir = Vector2::Zero;
	}
}

FORCEINLINE bool Vector2::IsNearlyZero(float tolerance) const noexcept
{
	return math::Abs(x) <= tolerance && math::Abs(y) <= tolerance;
}

FORCEINLINE bool Vector2::IsZero() const noexcept
{
	return x == 0.f && y == 0.f;
}

FORCEINLINE IntPoint Vector2::GetIntPoint() const noexcept
{
	return IntPoint(math::RoundToInt(x), math::RoundToInt(y));
}

FORCEINLINE Vector2 Vector2::RoundToVector() const noexcept
{
	return Vector2((float)math::RoundToInt(x), (float)math::RoundToInt(y));
}

FORCEINLINE Vector2 Vector2::ClampAxes(float minAxisVal, float maxAxisVal) const noexcept
{
	return Vector2(math::Clamp(x, minAxisVal, maxAxisVal), math::Clamp(y, minAxisVal, maxAxisVal));
}

FORCEINLINE Vector2 Vector2::GetSignVector() const noexcept
{
	return Vector2
	(
		math::FloatSelect(x, 1.f, -1.f),
		math::FloatSelect(y, 1.f, -1.f)
	);
}

FORCEINLINE Vector2 Vector2::GetAbs() const noexcept
{
	return Vector2(math::Abs(x), math::Abs(y));
}

FORCEINLINE std::string Vector2::ToString() const noexcept
{
	return StringUtils::Printf("x=%3.3f y=%3.3f", x, y);
}