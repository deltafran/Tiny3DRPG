#include "stdafx.h"
#include "CoreMath2.h"
#include "IntPoint.h"
#include "Vector2.h"
#include "Vector3.h"
#include "IntVector.h"
#include "Plane.h"
#include "Quat.h"
#include "Rotator.h"

// ---------------------------------------- Globals ----------------------------------------

// IntPoint
const IntPoint IntPoint::ZeroValue(0, 0);
const IntPoint IntPoint::NoneValue(-1, -1);
// IntVector
const IntVector IntVector::ZeroValue(0, 0, 0);
const IntVector IntVector::NoneValue(-1, -1, -1);
// Vector2
const Vector2 Vector2::ZeroVector(0.0f, 0.0f);
const Vector2 Vector2::UnitVector(1.0f, 1.0f);
// Vector3
const Vector3 Vector3::ZeroVector(0.0f, 0.0f, 0.0f);
const Vector3 Vector3::OneVector(1.0f, 1.0f, 1.0f);
const Vector3 Vector3::UpVector(0.0f, 1.0f, 0.0f);
const Vector3 Vector3::ForwardVector(0.0f, 0.0f, -1.0f);
const Vector3 Vector3::RightVector(1.0f, 0.0f, 0.0f);
// rotator
const Rotator Rotator::ZeroRotator(0.f, 0.f, 0.f);
// quat
const Quat Quat::Identity(0, 0, 0, 1);
// matrix
const Matrix4x4 Matrix4x4::Identity(Plane(1, 0, 0, 0), Plane(0, 1, 0, 0), Plane(0, 0, 1, 0), Plane(0, 0, 0, 1));

const uint32_t math::BitFlag[32] =
{
	(1U << 0),	(1U << 1),	(1U << 2),	(1U << 3),
	(1U << 4),	(1U << 5),	(1U << 6),	(1U << 7),
	(1U << 8),	(1U << 9),	(1U << 10),	(1U << 11),
	(1U << 12),	(1U << 13),	(1U << 14),	(1U << 15),
	(1U << 16),	(1U << 17),	(1U << 18),	(1U << 19),
	(1U << 20),	(1U << 21),	(1U << 22),	(1U << 23),
	(1U << 24),	(1U << 25),	(1U << 26),	(1U << 27),
	(1U << 28),	(1U << 29),	(1U << 30),	(1U << 31),
};

// ---------------------------------------- Helper ----------------------------------------
Quat FindBetweenHelper(const Vector3& a, const Vector3& b, float normAB)
{
	float w = normAB + Vector3::DotProduct(a, b);
	Quat result;

	if (w >= 1e-6f * normAB)
	{
		result = Quat(
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x,
			w
		);
	}
	else
	{
		w = 0.f;
		result = math::Abs(a.x) > math::Abs(a.y)
			? Quat(-a.z, 0.f, a.x, w)
			: Quat(0.f, -a.z, a.y, w);
	}

	result.Normalize();
	return result;
}

// ---------------------------------------- Math ----------------------------------------

Vector3 math::LinePlaneIntersection(const Vector3& point1, const Vector3& point2, const Vector3& planeOrigin, const Vector3& planeNormal)
{
	return point1 + (point2 - point1) * (((planeOrigin - point1) | planeNormal) / ((point2 - point1) | planeNormal));
}

Vector3 math::GetReflectionVector(const Vector3& direction, const Vector3& surfaceNormal)
{
	return direction - 2 * (direction | surfaceNormal.GetSafeNormal()) * surfaceNormal.GetSafeNormal();
}

Vector2 math::RandPointInCircle(float circleRadius)
{
	Vector2 point;
	float l;
	do
	{
		point.x = FRand() * 2.f - 1.f;
		point.x = FRand() * 2.f - 1.f;
		l = point.SizeSquared();
	} while (l > 1.0f);

	return point * circleRadius;
}

bool math::GetDotDistance(Vector2& outDotDist, const Vector3& direction, const Vector3& axisX, const Vector3& axisY, const Vector3& axisZ)
{
	const Vector3 normalDir = direction.GetSafeNormal();
	const Vector3 noZProjDir = (normalDir - (normalDir | axisZ) * axisZ).GetSafeNormal();

	const float azimuthSign = ((noZProjDir | axisY) < 0.f) ? -1.f : 1.f;
	outDotDist.y = normalDir | axisZ;
	const float dirDotX = noZProjDir | axisX;
	outDotDist.x = azimuthSign * math::Abs(dirDotX);

	return (dirDotX >= 0.f);
}

Vector3 math::VRandCone(Vector3 const& dir, float coneHalfAngleRad)
{
	if (coneHalfAngleRad > 0.f)
	{
		float const randU = math::FRand();
		float const randV = math::FRand();

		float theta = 2.f * PI * randU;
		float phi = math::Acos((2.f * randV) - 1.f);

		phi = math::Fmod(phi, coneHalfAngleRad);

		Matrix4x4 const dirMat(dir.Rotation(), Vector3::ZeroVector);
		Vector3 const dirZ = dirMat.GetScaledAxis(Axis::X);
		Vector3 const dirY = dirMat.GetScaledAxis(Axis::Y);

		Vector3 result = dir.RotateAngleAxis(phi * 180.f / PI, dirY);
		result = result.RotateAngleAxis(theta * 180.f / PI, dirZ);
		result = result.GetSafeNormal();

		return result;
	}
	else
	{
		return dir.GetSafeNormal();
	}
}

Vector3 math::VRandCone(Vector3 const& dir, float HorizontalConeHalfAngleRad, float VerticalConeHalfAngleRad)
{
	if ((VerticalConeHalfAngleRad > 0.f) && (HorizontalConeHalfAngleRad > 0.f))
	{
		float const randU = math::FRand();
		float const randV = math::FRand();

		float theta = 2.f * PI * randU;
		float phi = math::Acos((2.f * randV) - 1.f);

		float coneHalfAngleRad = math::Square(math::Cos(theta) / VerticalConeHalfAngleRad) + math::Square(math::Sin(theta) / HorizontalConeHalfAngleRad);
		coneHalfAngleRad = math::Sqrt(1.f / coneHalfAngleRad);

		phi = math::Fmod(phi, coneHalfAngleRad);

		Matrix4x4 const dirMat(dir.Rotation(), Vector3::ZeroVector);
		Vector3 const dirZ = dirMat.GetScaledAxis(Axis::X);
		Vector3 const dirY = dirMat.GetScaledAxis(Axis::Y);

		Vector3 result = dir.RotateAngleAxis(phi * 180.f / PI, dirY);
		result = result.RotateAngleAxis(theta * 180.f / PI, dirZ);
		result = result.GetSafeNormal();

		return result;
	}
	else
	{
		return dir.GetSafeNormal();
	}
}

Vector2 math::GetAzimuthAndElevation(const Vector3& Direction, const Vector3& AxisX, const Vector3& AxisY, const Vector3& AxisZ)
{
	const Vector3 normalDir = Direction.GetSafeNormal();
	const Vector3 noZProjDir = (normalDir - (normalDir | AxisZ) * AxisZ).GetSafeNormal();

	const float azimuthSign = ((noZProjDir | AxisY) < 0.f) ? -1.f : 1.f;
	const float elevationSin = normalDir | AxisZ;
	const float azimuthCos = noZProjDir | AxisX;

	return Vector2(math::Acos(azimuthCos) * azimuthSign, math::Asin(elevationSin));
}

float math::GetRangePct(Vector2 const& range, float value)
{
	return GetRangePct(range.x, range.y, value);
}

float math::GetRangeValue(Vector2 const& range, float pct)
{
	return Lerp<float>(range.x, range.y, pct);
}

void math::CartesianToPolar(const Vector2 inCart, Vector2& outPolar)
{
	outPolar.x = Sqrt(Square(inCart.x) + Square(inCart.y));
	outPolar.y = Atan2(inCart.y, inCart.x);
}

void math::PolarToCartesian(const Vector2 inPolar, Vector2& outCart)
{
	outCart.x = inPolar.x * Cos(inPolar.y);
	outCart.y = inPolar.x * Sin(inPolar.y);
}

Vector3 math::RayPlaneIntersection(const Vector3& rayOrigin, const Vector3& rayDirection, const Plane& plane)
{
	const Vector3 planeNormal = Vector3(plane.x, plane.y, plane.z);
	const Vector3 planeOrigin = planeNormal * plane.w;

	const float distance = Vector3::DotProduct((planeOrigin - rayOrigin), planeNormal) / Vector3::DotProduct(rayDirection, planeNormal);
	return rayOrigin + rayDirection * distance;
}

Vector3 math::LinePlaneIntersection(const Vector3& point1, const Vector3& point2, const Plane& plane)
{
	return point1 + (point2 - point1) * ((plane.w - (point1 | plane)) / ((point2 - point1) | plane));
}

Vector3 math::VRand()
{
	Vector3 result;
	float l;
	do
	{
		result.x = FRand() * 2.f - 1.f;
		result.y = FRand() * 2.f - 1.f;
		result.z = FRand() * 2.f - 1.f;
		l = result.SizeSquared();
	} while (l > 1.0f || l < KINDA_SMALL_NUMBER);
	return result * (1.0f / Sqrt(l));
}

bool math::LineSphereIntersection(const Vector3& start, const Vector3& dir, float length, const Vector3& origin, float radius)
{
	const Vector3 eo = start - origin;
	const float	 v = (dir | (origin - start));
	const float	 disc = radius * radius - ((eo | eo) - v * v);

	if (disc >= 0.0f)
	{
		const float	time = (v - Sqrt(disc)) / length;
		if (time >= 0.0f && time <= 1.0f) {
			return 1;
		}
		else {
			return 0;
		}
	}
	else
	{
		return 0;
	}
}

float math::InterpConstantTo(float current, float target, float deltaTime, float interpSpeed)
{
	const float dist = target - current;
	if (math::Square(dist) < SMALL_NUMBER) {
		return target;
	}
	const float step = interpSpeed * deltaTime;
	return current + math::Clamp<float>(dist, -step, step);
}

float math::InterpTo(float current, float target, float deltaTime, float interpSpeed)
{
	if (interpSpeed <= 0.f) {
		return target;
	}

	const float dist = target - current;
	if (math::Square(dist) < SMALL_NUMBER) {
		return target;
	}

	const float deltaMove = dist * math::Clamp<float>(deltaTime * interpSpeed, 0.f, 1.f);
	return current + deltaMove;
}

void math::WindRelativeAnglesDegrees(float inAngle0, float& inOutAngle1)
{
	const float diff = inAngle0 - inOutAngle1;
	const float absDiff = Abs(diff);
	if (absDiff > 180.0f) {
		inOutAngle1 += 360.0f * Sign(diff) * FloorToFloat((absDiff / 360.0f) + 0.5f);
	}
}

float math::TruncateToHalfIfClose(float value)
{
	float valueToFudgeIntegralPart = 0.0f;
	float valueToFudgeFractionalPart = math::Modf(value, &valueToFudgeIntegralPart);
	if (value < 0.0f) {
		return valueToFudgeIntegralPart + ((math::IsNearlyEqual(valueToFudgeFractionalPart, -0.5f)) ? -0.5f : valueToFudgeFractionalPart);
	}
	else {
		return valueToFudgeIntegralPart + ((math::IsNearlyEqual(valueToFudgeFractionalPart, 0.5f)) ? 0.5f : valueToFudgeFractionalPart);
	}
}

double math::TruncateToHalfIfClose(double value)
{
	double valueToFudgeIntegralPart = 0.0;
	double valueToFudgeFractionalPart = math::Modf(value, &valueToFudgeIntegralPart);
	if (value < 0.0) {
		return valueToFudgeIntegralPart + ((math::IsNearlyEqual(valueToFudgeFractionalPart, -0.5)) ? -0.5 : valueToFudgeFractionalPart);
	}
	else {
		return valueToFudgeIntegralPart + ((math::IsNearlyEqual(valueToFudgeFractionalPart, 0.5)) ? 0.5 : valueToFudgeFractionalPart);
	}
}

float math::RoundHalfToEven(float value)
{
	value = math::TruncateToHalfIfClose(value);

	const bool bIsNegative = value < 0.0f;
	const bool bValueIsEven = static_cast<uint32_t>(FloorToFloat(((bIsNegative) ? -value : value))) % 2 == 0;
	if (bValueIsEven) {
		return (bIsNegative) ? FloorToFloat(value + 0.5f) : CeilToFloat(value - 0.5f);
	}
	else {
		return (bIsNegative) ? CeilToFloat(value - 0.5f) : FloorToFloat(value + 0.5f);
	}
}

double math::RoundHalfToEven(double value)
{
	value = math::TruncateToHalfIfClose(value);

	const bool bIsNegative = value < 0.0f;
	const bool bValueIsEven = static_cast<uint32_t>(FloorToFloat(((bIsNegative) ? -value : value))) % 2 == 0;
	if (bValueIsEven) {
		return (bIsNegative) ? FloorToFloat(value + 0.5f) : CeilToFloat(value - 0.5f);
	}
	else {
		return (bIsNegative) ? CeilToFloat(value - 0.5f) : FloorToFloat(value + 0.5f);
	}
}

float math::RoundHalfFromZero(float value)
{
	value = math::TruncateToHalfIfClose(value);
	return (value < 0.0f) ? CeilToFloat(value - 0.5f) : FloorToFloat(value + 0.5f);
}

double math::RoundHalfFromZero(double value)
{
	value = math::TruncateToHalfIfClose(value);
	return (value < 0.0) ? CeilToDouble(value - 0.5) : FloorToDouble(value + 0.5);
}

float math::RoundHalfToZero(float value)
{
	value = math::TruncateToHalfIfClose(value);
	return (value < 0.0f) ? FloorToFloat(value + 0.5f) : CeilToFloat(value - 0.5f);
}

double math::RoundHalfToZero(double value)
{
	value = math::TruncateToHalfIfClose(value);
	return (value < 0.0) ? FloorToDouble(value + 0.5) : CeilToDouble(value - 0.5);
}

float math::PerlinNoise1D(const float value)
{
	const int32_t b = 256;
	static int32_t p[b + b + 2];
	static float g[b + b + 2];
	const int32_t bm = 255;

	static bool bIsFirstCall = true;
	if (bIsFirstCall)
	{
		bIsFirstCall = false;

		int32_t i;
		for (i = 0; i < b; i++)
		{
			p[i] = i;
			const int32_t Random1 = math::RandRange(0, 0x3fffffff);
			g[i] = (float)((Random1 % (b + b)) - b) / b;
		}

		while (--i)
		{
			const int32_t k = p[i];
			const int32_t Random = math::RandRange(0, 0x3fffffff);
			const int32_t j = Random % b;
			p[i] = p[j];
			p[j] = k;
		}

		for (i = 0; i < b + 2; i++)
		{
			p[b + i] = p[i];
			g[b + i] = g[i];
		}
	}

	const int32_t n = 4096;
	const float t = value + n;
	const int32_t bx0 = ((int32_t)t) & bm;
	const int32_t bx1 = (bx0 + 1) & bm;
	const float rx0 = t - (int32_t)t;
	const float rx1 = rx0 - 1.;
	const float sx = (rx0 * rx0 * (3. - 2. * rx0));
	const float u = rx0 * g[p[bx0]];
	const float v = rx1 * g[p[bx1]];

	return 2.0f * (u + sx * (v - u));
}

// ---------------------------------------- Quat ----------------------------------------

Quat Quat::MakeFromEuler(const Vector3& euler)
{
	return Rotator::MakeFromEuler(euler).Quaternion();
}

Vector3 Quat::Euler() const
{
	return Rotator().Euler();
}

void Quat::ToSwingTwist(const Vector3& inTwistAxis, Quat& outSwing, Quat& outTwist) const
{

	Vector3 projection = Vector3::DotProduct(inTwistAxis, Vector3(x, y, z)) * inTwistAxis;

	outTwist = Quat(projection.x, projection.y, projection.z, w);

	if (outTwist.SizeSquared() == 0.0f) {
		outTwist = Quat::Identity;
	}
	else {
		outTwist.Normalize();
	}

	outSwing = *this * outTwist.Inverse();
}

Rotator Quat::GetRotator() const
{
	DiagnosticCheckNaN();
	const float singularityTest = z * x - w * y;
	const float yawY = 2.f * (w * z + x * y);
	const float yawX = (1.f - 2.f * (math::Square(y) + math::Square(z)));

	const float SINGULARITY_THRESHOLD = 0.4999995f;
	const float RAD_TO_DEG = (180.f) / PI;
	Rotator rotatorFromQuat;

	if (singularityTest < -SINGULARITY_THRESHOLD)
	{
		rotatorFromQuat.pitch = -90.f;
		rotatorFromQuat.yaw = math::Atan2(yawY, yawX) * RAD_TO_DEG;
		rotatorFromQuat.roll = Rotator::NormalizeAxis(-rotatorFromQuat.yaw - (2.f * math::Atan2(x, w) * RAD_TO_DEG));
	}
	else if (singularityTest > SINGULARITY_THRESHOLD)
	{
		rotatorFromQuat.pitch = 90.f;
		rotatorFromQuat.yaw = math::Atan2(yawY, yawX) * RAD_TO_DEG;
		rotatorFromQuat.roll = Rotator::NormalizeAxis(rotatorFromQuat.yaw - (2.f * math::Atan2(x, w) * RAD_TO_DEG));
	}
	else
	{
		rotatorFromQuat.pitch = math::FastAsin(2.f * (singularityTest)) * RAD_TO_DEG;
		rotatorFromQuat.yaw = math::Atan2(yawY, yawX) * RAD_TO_DEG;
		rotatorFromQuat.roll = math::Atan2(-2.f * (w * x + y * z), (1.f - 2.f * (math::Square(x) + math::Square(y)))) * RAD_TO_DEG;
	}

	return rotatorFromQuat;
}

Quat Quat::FindBetweenNormals(const Vector3& a, const Vector3& b)
{
	const float normAB = 1.f;
	return FindBetweenHelper(a, b, normAB);
}

Quat Quat::FindBetweenVectors(const Vector3& a, const Vector3& b)
{
	const float normAB = math::Sqrt(a.SizeSquared() * b.SizeSquared());
	return FindBetweenHelper(a, b, normAB);
}

Quat Quat::SlerpFullPathNotNormalized(const Quat& quat1, const Quat& quat2, float alpha)
{
	const float cosAngle = math::Clamp(quat1 | quat2, -1.f, 1.f);
	const float angle = math::Acos(cosAngle);

	if (math::Abs(angle) < KINDA_SMALL_NUMBER) {
		return quat1;
	}

	const float sinAngle = math::Sin(angle);
	const float invSinAngle = 1.f / sinAngle;
	const float scale0 = math::Sin((1.0f - alpha) * angle) * invSinAngle;
	const float scale1 = math::Sin(alpha * angle) * invSinAngle;

	return quat1 * scale0 + quat2 * scale1;
}

Quat Quat::Squad(const Quat& quat1, const Quat& tang1, const Quat& quat2, const Quat& tang2, float Alpha)
{
	const Quat q1 = Quat::SlerpNotNormalized(quat1, quat2, Alpha);
	const Quat q2 = Quat::SlerpFullPathNotNormalized(tang1, tang2, Alpha);
	const Quat result = Quat::SlerpFullPath(q1, q2, 2.f * Alpha * (1.f - Alpha));

	return result;
}

Quat Quat::SquadFullPath(const Quat& quat1, const Quat& tang1, const Quat& quat2, const Quat& tang2, float Alpha)
{
	const Quat q1 = Quat::SlerpFullPathNotNormalized(quat1, quat2, Alpha);
	const Quat q2 = Quat::SlerpFullPathNotNormalized(tang1, tang2, Alpha);
	const Quat result = Quat::SlerpFullPath(q1, q2, 2.f * Alpha * (1.f - Alpha));

	return result;
}

void Quat::CalcTangents(const Quat& PrevP, const Quat& P, const Quat& NextP, float Tension, Quat& OutTan)
{
	const Quat InvP = P.Inverse();
	const Quat Part1 = (InvP * PrevP).Log();
	const Quat Part2 = (InvP * NextP).Log();

	const Quat PreExp = (Part1 + Part2) * -0.5f;

	OutTan = P * PreExp.Exp();
}

Quat Quat::SlerpNotNormalized(const Quat& quat1, const Quat& quat2, float slerp)
{
	const float RawCosom =
		quat1.x * quat2.x +
		quat1.y * quat2.y +
		quat1.z * quat2.z +
		quat1.w * quat2.w;

	const float cosom = math::FloatSelect(RawCosom, RawCosom, -RawCosom);

	float scale0, scale1;

	if (cosom < 0.9999f)
	{
		const float omega = math::Acos(cosom);
		const float invSin = 1.f / math::Sin(omega);
		scale0 = math::Sin((1.f - slerp) * omega) * invSin;
		scale1 = math::Sin(slerp * omega) * invSin;
	}
	else
	{
		scale0 = 1.0f - slerp;
		scale1 = slerp;
	}

	scale1 = math::FloatSelect(RawCosom, scale1, -scale1);

	Quat result;
	result.x = scale0 * quat1.x + scale1 * quat2.x;
	result.y = scale0 * quat1.y + scale1 * quat2.y;
	result.z = scale0 * quat1.z + scale1 * quat2.z;
	result.w = scale0 * quat1.w + scale1 * quat2.w;

	return result;
}

// ---------------------------------------- Rotator ----------------------------------------

Vector3 Rotator::RotateVector(const Vector3& v) const
{
	Matrix4x4 matrix4x4(*this, Vector3::ZeroVector);
	return matrix4x4.TransformVector(v);
}

Vector3 Rotator::UnrotateVector(const Vector3& v) const
{
	Matrix4x4 matrix4x4(*this, Vector3::ZeroVector);
	return matrix4x4.GetTransposed().TransformVector(v);
}

Rotator::Rotator(const Quat& quat)
{
	*this = quat.GetRotator();
	DiagnosticCheckNaN();
}

Rotator Rotator::GetInverse() const
{
	return Quaternion().Inverse().GetRotator();
}

Quat Rotator::Quaternion() const
{
	DiagnosticCheckNaN();

	const float DEG_TO_RAD = PI / (180.f);
	const float DIVIDE_BY_2 = DEG_TO_RAD / 2.f;
	float sp, sy, sr;
	float cp, cy, cr;

	math::SinCos(&sp, &cp, pitch * DIVIDE_BY_2);
	math::SinCos(&sy, &cy, yaw * DIVIDE_BY_2);
	math::SinCos(&sr, &cr, roll * DIVIDE_BY_2);

	Quat rotationQuat;
	rotationQuat.x = cr * sp * sy - sr * cp * cy;
	rotationQuat.y = -cr * sp * cy - sr * cp * sy;
	rotationQuat.z = cr * cp * sy - sr * sp * cy;
	rotationQuat.w = cr * cp * cy + sr * sp * sy;

	return rotationQuat;
}

// ---------------------------------------- Vector3 ----------------------------------------

Vector3::Vector3(const Vector4& v)
	: x(v.x)
	, y(v.y)
	, z(v.z)
{
	DiagnosticCheckNaN();
}

Rotator Vector3::Rotation() const
{
	return ToOrientationRotator();
}

Rotator Vector3::ToOrientationRotator() const
{
	Rotator r;

	r.yaw = math::Atan2(y, x) * (180.f / PI);
	r.pitch = math::Atan2(z, math::Sqrt(x * x + y * y)) * (180.f / PI);
	r.roll = 0;

	return r;
}

Quat Vector3::ToOrientationQuat() const
{
	const float yawRad = math::Atan2(y, x);
	const float pitchRad = math::Atan2(z, math::Sqrt(x * x + y * y));
	const float DIVIDE_BY_2 = 0.5f;

	float sp, sy;
	float cp, cy;

	math::SinCos(&sp, &cp, pitchRad * DIVIDE_BY_2);
	math::SinCos(&sy, &cy, yawRad * DIVIDE_BY_2);

	Quat rotationQuat;
	rotationQuat.x = sp * sy;
	rotationQuat.y = -sp * cy;
	rotationQuat.z = cp * sy;
	rotationQuat.w = cp * cy;

	return rotationQuat;
}

Vector3 Vector3::MirrorByPlane(const Plane& plane) const
{
	return *this - plane * (2.f * plane.PlaneDot(*this));
}

Vector3 Vector3::PointPlaneProject(const Vector3& point, const Plane& plane)
{
	return point - plane.PlaneDot(point) * plane;
}

Vector3 Vector3::PointPlaneProject(const Vector3& point, const Vector3& a, const Vector3& b, const Vector3& c)
{
	Plane plane(a, b, c);
	return point - plane.PlaneDot(point) * plane;
}

// ---------------------------------------- IntVector ----------------------------------------

IntVector::IntVector(Vector3 inVector)
	: x(math::TruncToInt(inVector.x))
	, y(math::TruncToInt(inVector.y))
	, z(math::TruncToInt(inVector.z))
{

}

// ---------------------------------------- Matrix4x4 ----------------------------------------

Rotator Matrix4x4::ToRotator() const
{
	const Vector3 xAxis = GetScaledAxis(Axis::X);
	const Vector3 yAxis = GetScaledAxis(Axis::Y);
	const Vector3 zAxis = GetScaledAxis(Axis::Z);

	Rotator	rotator = Rotator(
		math::Atan2(xAxis.z, math::Sqrt(math::Square(xAxis.x) + math::Square(xAxis.y))) * 180.f / PI,
		math::Atan2(xAxis.y, xAxis.x) * 180.f / PI,
		0
	);

	const Vector3 syAxis = Matrix4x4(rotator, Vector3::ZeroVector).GetScaledAxis(Axis::Y);
	rotator.roll = math::Atan2(zAxis | syAxis, yAxis | syAxis) * 180.f / PI;
	rotator.DiagnosticCheckNaN();
	return rotator;
}

Quat Matrix4x4::ToQuat() const
{
	Quat result(*this);
	return result;
}

// ---------------------------------------- Plane ----------------------------------------

Plane Plane::TransformBy(const Matrix4x4& m) const
{
	const Matrix4x4 tmpTA = m.TransposeAdjoint();
	const float detM = m.Determinant();
	return this->TransformByUsingAdjointT(m, detM, tmpTA);
}

Plane Plane::TransformByUsingAdjointT(const Matrix4x4& m, float detM, const Matrix4x4& ta) const
{
	Vector3 newNorm = ta.TransformVector(*this).GetSafeNormal();

	if (detM < 0.f) {
		newNorm *= -1.0f;
	}

	return Plane(m.TransformPosition(*this * w), newNorm);
}