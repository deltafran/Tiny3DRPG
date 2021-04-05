#pragma once

#include "Log.h"
#include "CoreMath2.h"
#include "Vector3.h"

struct Color;

enum class GammaSpace
{
	Linear,
	Pow22,
	sRGB,
};

struct LinearColor
{

public:
	float r;
	float g;
	float b;
	float a;

	static const LinearColor White;
	static const LinearColor Gray;
	static const LinearColor Black;
	static const LinearColor Transparent;
	static const LinearColor Red;
	static const LinearColor Green;
	static const LinearColor Blue;
	static const LinearColor Yellow;

	static float pow22OneOver255Table[256];
	static float sRGBToLinearTable[256];

public:

	explicit LinearColor(const Color& Color);

	explicit LinearColor()
		: r(0)
		, g(0)
		, b(0)
		, a(0)
	{

	}

	constexpr LinearColor(float inR, float inG, float inB, float inA = 1.0f)
		: r(inR)
		, g(inG)
		, b(inB)
		, a(inA)
	{

	}

	FORCEINLINE Color ToRGBE() const;

	FORCEINLINE LinearColor LinearRGBToHSV() const;

	FORCEINLINE LinearColor HSVToLinearRGB() const;

	FORCEINLINE Color Quantize() const;

	FORCEINLINE Color QuantizeRound() const;

	FORCEINLINE Color ToFColor(const bool sRGB) const;

	FORCEINLINE LinearColor Desaturate(float desaturation) const;

	FORCEINLINE float& Component(int32_t index)
	{
		return (&r)[index];
	}

	FORCEINLINE const float& Component(int32_t index) const
	{
		return (&r)[index];
	}

	FORCEINLINE LinearColor operator+(const LinearColor& rhs) const
	{
		return LinearColor(
			this->r + rhs.r,
			this->g + rhs.g,
			this->b + rhs.b,
			this->a + rhs.a
		);
	}

	FORCEINLINE LinearColor& operator+=(const LinearColor& rhs)
	{
		r += rhs.r;
		g += rhs.g;
		b += rhs.b;
		a += rhs.a;

		return *this;
	}

	FORCEINLINE LinearColor operator-(const LinearColor& rhs) const
	{
		return LinearColor(
			this->r - rhs.r,
			this->g - rhs.g,
			this->b - rhs.b,
			this->a - rhs.a
		);
	}

	FORCEINLINE LinearColor& operator-=(const LinearColor& rhs)
	{
		r -= rhs.r;
		g -= rhs.g;
		b -= rhs.b;
		a -= rhs.a;

		return *this;
	}

	FORCEINLINE LinearColor operator*(const LinearColor& rhs) const
	{
		return LinearColor(
			this->r * rhs.r,
			this->g * rhs.g,
			this->b * rhs.b,
			this->a * rhs.a
		);
	}

	FORCEINLINE LinearColor& operator*=(const LinearColor& rhs)
	{
		r *= rhs.r;
		g *= rhs.g;
		b *= rhs.b;
		a *= rhs.a;

		return *this;
	}

	FORCEINLINE LinearColor operator*(float scalar) const
	{
		return LinearColor(
			this->r * scalar,
			this->g * scalar,
			this->b * scalar,
			this->a * scalar
		);
	}

	FORCEINLINE LinearColor& operator*=(float scalar)
	{
		r *= scalar;
		g *= scalar;
		b *= scalar;
		a *= scalar;

		return *this;
	}

	FORCEINLINE LinearColor operator/(const LinearColor& rhs) const
	{
		return LinearColor(
			this->r / rhs.r,
			this->g / rhs.g,
			this->b / rhs.b,
			this->a / rhs.a
		);
	}

	FORCEINLINE LinearColor& operator/=(const LinearColor& rhs)
	{
		r /= rhs.r;
		g /= rhs.g;
		b /= rhs.b;
		a /= rhs.a;

		return *this;
	}

	FORCEINLINE LinearColor operator/(float scalar) const
	{
		const float	invScalar = 1.0f / scalar;

		return LinearColor(
			this->r * invScalar,
			this->g * invScalar,
			this->b * invScalar,
			this->a * invScalar
		);
	}

	FORCEINLINE LinearColor& operator/=(float scalar)
	{
		const float	invScalar = 1.0f / scalar;

		r *= invScalar;
		g *= invScalar;
		b *= invScalar;
		a *= invScalar;

		return *this;
	}

	FORCEINLINE LinearColor GetClamped(float inMin = 0.0f, float inMax = 1.0f) const
	{
		LinearColor ret;

		ret.r = math::Clamp(r, inMin, inMax);
		ret.g = math::Clamp(g, inMin, inMax);
		ret.b = math::Clamp(b, inMin, inMax);
		ret.a = math::Clamp(a, inMin, inMax);

		return ret;
	}

	FORCEINLINE bool operator==(const LinearColor& other) const
	{
		return this->r == other.r && this->g == other.g && this->b == other.b && this->a == other.a;
	}

	FORCEINLINE bool operator!=(const LinearColor& Other) const
	{
		return this->r != Other.r || this->g != Other.g || this->b != Other.b || this->a != Other.a;
	}

	FORCEINLINE bool Equals(const LinearColor& other, float tolerance = KINDA_SMALL_NUMBER) const
	{
		return math::Abs(this->r - other.r) < tolerance && math::Abs(this->g - other.g) < tolerance && math::Abs(this->b - other.b) < tolerance && math::Abs(this->a - other.a) < tolerance;
	}

	FORCEINLINE LinearColor CopyWithNewOpacity(float newOpacicty) const
	{
		LinearColor newCopy = *this;
		newCopy.a = newOpacicty;
		return newCopy;
	}

	FORCEINLINE float ComputeLuminance() const
	{
		return r * 0.3f + g * 0.59f + b * 0.11f;
	}

	FORCEINLINE float GetMax() const
	{
		return math::Max(math::Max(math::Max(r, g), b), a);
	}

	FORCEINLINE bool IsAlmostBlack() const
	{
		return math::Square(r) < DELTA && math::Square(g) < DELTA && math::Square(b) < DELTA;
	}

	FORCEINLINE float GetMin() const
	{
		return math::Min(math::Min(math::Min(r, g), b), a);
	}

	FORCEINLINE float GetLuminance() const
	{
		return r * 0.3f + g * 0.59f + b * 0.11f;
	}

	FORCEINLINE std::string ToString() const
	{
		return StringUtils::Printf("(r=%f,g=%f,b=%f,a=%f)", r, g, b, a);
	}

	static FORCEINLINE float Dist(const LinearColor& v1, const LinearColor& v2)
	{
		return math::Sqrt(math::Square(v2.r - v1.r) + math::Square(v2.g - v1.g) + math::Square(v2.b - v1.b) + math::Square(v2.a - v1.a));
	}

	static LinearColor GetHSV(uint8_t h, uint8_t s, uint8_t v);

	static LinearColor MakeRandomColor();

	static LinearColor MakeFromColorTemperature(float temp);

	static LinearColor FromSRGBColor(const Color& Color);

	static LinearColor FromPow22Color(const Color& Color);

	static LinearColor LerpUsingHSV(const LinearColor& from, const LinearColor& to, const float progress);
};

struct Color
{
public:

	uint8_t b;
	uint8_t g;
	uint8_t r;
	uint8_t a;

	static const Color White;
	static const Color Black;
	static const Color Transparent;
	static const Color Red;
	static const Color Green;
	static const Color Blue;
	static const Color Yellow;
	static const Color Cyan;
	static const Color Magenta;
	static const Color Orange;
	static const Color Purple;
	static const Color Turquoise;
	static const Color Silver;
	static const Color Emerald;

public:

	Color()
	{

	}

	constexpr Color(uint8_t inR, uint8_t inG, uint8_t inB, uint8_t inA = 255)
		: b(inB)
		, g(inG)
		, r(inR)
		, a(inA)
	{

	}

	explicit Color(uint32_t inColor)
	{
		DWColor() = inColor;
	}

	FORCEINLINE uint32_t& DWColor()
	{
		return *((uint32_t*)this);
	}

	FORCEINLINE const uint32_t& DWColor() const
	{
		return *((uint32_t*)this);
	}

	FORCEINLINE bool operator==(const Color& C) const
	{
		return DWColor() == C.DWColor();
	}

	FORCEINLINE bool operator!=(const Color& C) const
	{
		return DWColor() != C.DWColor();
	}

	FORCEINLINE void operator+=(const Color& C)
	{
		r = (uint8_t)math::Min((int32_t)r + (int32_t)C.r, 255);
		g = (uint8_t)math::Min((int32_t)g + (int32_t)C.g, 255);
		b = (uint8_t)math::Min((int32_t)b + (int32_t)C.b, 255);
		a = (uint8_t)math::Min((int32_t)a + (int32_t)C.a, 255);
	}

	FORCEINLINE Color WithAlpha(uint8_t alpha) const
	{
		return Color(r, g, b, alpha);
	}

	FORCEINLINE LinearColor ReinterpretAsLinear() const
	{
		return LinearColor(r / 255.f, g / 255.f, b / 255.f, a / 255.f);
	}

	FORCEINLINE std::string ToHex() const
	{
		return StringUtils::Printf("%02X%02X%02X%02X", r, g, b, a);
	}

	FORCEINLINE std::string ToString() const
	{
		return StringUtils::Printf("(r=%i,g=%i,b=%i,a=%i)", r, g, b, a);
	}

	FORCEINLINE uint32_t ToPackedARGB() const
	{
		return (a << 24) | (r << 16) | (g << 8) | (b << 0);
	}

	FORCEINLINE uint32_t ToPackedABGR() const
	{
		return (a << 24) | (b << 16) | (g << 8) | (r << 0);
	}

	FORCEINLINE uint32_t ToPackedRGBA() const
	{
		return (r << 24) | (g << 16) | (b << 8) | (a << 0);
	}

	FORCEINLINE uint32_t ToPackedBGRA() const
	{
		return (b << 24) | (g << 16) | (r << 8) | (a << 0);
	}

	LinearColor FromRGBE() const;

	static Color MakeRandomColor();

	static Color MakeRedToGreenColorFromScalar(float scalar);

	static Color MakeFromColorTemperature(float temp);

private:
	explicit Color(const LinearColor& linearColor);
};

FORCEINLINE LinearColor operator*(float scalar, const LinearColor& Color)
{
	return Color.operator*(scalar);
}