/////--------------------------------------------------------------------------------------------------------------------------------------------------
///// Author: Andrew Chase
///// Date Created: December 14th, 2019
///// Description: 
/////--------------------------------------------------------------------------------------------------------------------------------------------------
//
/////--------------------------------------------------------------------------------------------------------------------------------------------------
///// INCLUDES
/////--------------------------------------------------------------------------------------------------------------------------------------------------
//#include <math.h>
//#include "Engine/Math/Matrix44.h"
//#include "Engine/Math/MathUtils.h"
//#include "Engine/Math/Quaternion.h"
//
/////--------------------------------------------------------------------------------------------------------------------------------------------------
///// DEFINES
/////--------------------------------------------------------------------------------------------------------------------------------------------------
//
/////--------------------------------------------------------------------------------------------------------------------------------------------------
///// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
/////--------------------------------------------------------------------------------------------------------------------------------------------------
//
/////--------------------------------------------------------------------------------------------------------------------------------------------------
///// GLOBALS AND STATICS
/////--------------------------------------------------------------------------------------------------------------------------------------------------
//const Quaternion Quaternion::IDENTITY = Quaternion();
//
/////--------------------------------------------------------------------------------------------------------------------------------------------------
///// C FUNCTIONS
/////--------------------------------------------------------------------------------------------------------------------------------------------------
//
/////--------------------------------------------------------------------------------------------------------------------------------------------------
///// CLASS IMPLEMENTATIONS
/////--------------------------------------------------------------------------------------------------------------------------------------------------
//
//
//
////-------------------------------------------------------------------------------------------------
//Quaternion::Quaternion(const Vector3& vector, float sValue)
//	: v(vector)
//	, s(sValue)
//{
//}
//
//
////-------------------------------------------------------------------------------------------------
//Quaternion::Quaternion()
//	: v(Vector3::ZERO)
//	, s(1.0f) 
//{
//}
//
//
////-------------------------------------------------------------------------------------------------
//Quaternion::Quaternion(const Quaternion& copy)
//	: v(copy.v)
//	, s(copy.s)
//{
//}
//
//
////-------------------------------------------------------------------------------------------------
//Quaternion::Quaternion(float xValue, float yValue, float zValue, float sValue)
//	: v(Vector3(xValue, yValue, zValue))
//	, s(sValue)
//{
//}
//
//
////-------------------------------------------------------------------------------------------------
//const Quaternion Quaternion::operator+(const Quaternion& other) const
//{
//	Quaternion result;
//	result.v = v + other.v;
//	result.s = s + other.s;
//
//	return result;
//}
//
//
////-------------------------------------------------------------------------------------------------
//const Quaternion Quaternion::operator-(const Quaternion& other) const
//{
//	Quaternion result;
//	result.v = v - other.v;
//	result.s = s - other.s;
//
//	return result;
//}
//
//
////-------------------------------------------------------------------------------------------------
//const Quaternion Quaternion::operator*(const Quaternion& other) const
//{
//	Quaternion result;
//
//	result.s = s * other.s - DotProduct(v, other.v);
//	result.v = s * other.v + v * other.s + CrossProduct(v, other.v);
//
//	return result;
//}
//
//
////-------------------------------------------------------------------------------------------------
//const Quaternion Quaternion::operator*(float scalar) const
//{
//	Quaternion result;
//
//	result.s = s * scalar;
//	result.v = v * scalar;
//
//	return result;
//}
//
//
////-------------------------------------------------------------------------------------------------
//const Quaternion operator*(float scalar, const Quaternion& quat)
//{
//	Quaternion result;
//
//	result.s = scalar * quat.s;
//	result.v = scalar * quat.v;
//
//	return result;
//}
//
//
////-------------------------------------------------------------------------------------------------
//Quaternion Quaternion::Lerp(const Quaternion& a, const Quaternion& b, float fractionTowardEnd)
//{
//	float sResult = Interpolate(a.s, b.s, fractionTowardEnd);
//	Vector3 vResult = Interpolate(a.v, b.v, fractionTowardEnd);
//
//	return Quaternion(vResult, sResult);
//}
//
//
////-------------------------------------------------------------------------------------------------
//void Quaternion::operator*=(const Quaternion& other)
//{
//	Quaternion old = (*this);
//
//	s = old.s * other.s - DotProduct(old.v, other.v);
//	v = old.s * other.v + old.v * other.s + CrossProduct(old.v, other.v);
//}
//
//
////-------------------------------------------------------------------------------------------------
//void Quaternion::operator*=(float scalar)
//{
//	s = s * scalar;
//	v = v * scalar;
//}
//
//
//
////-------------------------------------------------------------------------------------------------
//void Quaternion::operator+=(const Quaternion& other)
//{
//	v += other.v;
//	s += other.s;
//}
//
//
////-------------------------------------------------------------------------------------------------
//void Quaternion::operator-=(const Quaternion& other)
//{
//	v -= other.v;
//	s -= other.s;
//}
//
//
////-------------------------------------------------------------------------------------------------
//void Quaternion::operator=(const Quaternion& copy)
//{
//	v = copy.v;
//	s = copy.s;
//}
//
//
////-------------------------------------------------------------------------------------------------
//float Quaternion::GetMagnitude() const
//{
//	float magSquared = (s * s) + (v.x * v.x) + (v.y * v.y) + (v.z * v.z);
//	return sqrtf(magSquared);
//}
//
//
////-------------------------------------------------------------------------------------------------
//Quaternion Quaternion::GetNormalized() const
//{
//	Quaternion norm = (*this);
//	norm.Normalize();
//
//	return norm;
//}
//
//
////-------------------------------------------------------------------------------------------------
//// q^-1 = q* / ||q||^2
//Quaternion Quaternion::GetInverse() const
//{
//	float demonimator = GetMagnitude();
//	demonimator *= demonimator;
//	demonimator = 1.0f / demonimator;
//
//	Quaternion conjugate = GetConjugate();
//
//	Vector3 vector = conjugate.v * demonimator;
//	float scalar = conjugate.s * demonimator;
//
//	return Quaternion(vector, scalar);
//}
//
//
////-------------------------------------------------------------------------------------------------
//Vector3 Quaternion::GetAsEulerAngles() const
//{
//	float test = v.x * v.y + v.z * s;
//
//	Vector3 angles;
//
//	if (AreMostlyEqual(test, 0.5f)) // singularity at north pole
//	{
//		angles.x = PI_OVER_TWO;
//		angles.y = 2.f * Atan2Degrees(v.x, s);
//		angles.z = 0.f;
//	}
//	else if (AreMostlyEqual(test, -0.5f))
//	{
//		angles.x = -1.0f * PI_OVER_TWO;
//		angles.y = -2.f * Atan2Degrees(v.x, s);
//		angles.z = 0.f;
//	}
//	else
//	{
//		float xSquared = v.x * v.x;
//		float ySquared = v.y * v.y;
//		float zSquared = v.z * v.z;
//
//		angles.x = asin(2 * test);
//		angles.y = atan2(2 * v.y * s - 2.f * v.x * v.z, 1.0f - 2.0f * ySquared - 2.0f * zSquared);
//		angles.z = atan2(2 * v.x * s - 2.f * v.y * v.z, 1.0f - 2.0f * xSquared - 2.0f * zSquared);
//	}
//
//	return angles;
//}
//
//
////-------------------------------------------------------------------------------------------------
//void Quaternion::Normalize()
//{
//	float magnitude = GetMagnitude();
//	ASSERT_OR_DIE(magnitude > 0.f, "Quaternion::Normalize() called on a zero quaternion!");
//
//	float oneOverMag = (1.f / magnitude);
//
//	v *= oneOverMag;
//	s *= oneOverMag;
//}
//
//
////-------------------------------------------------------------------------------------------------
//float Quaternion::GetAngleBetweenDegrees(const Quaternion& a, const Quaternion& b)
//{
//	float newReal = a.s * b.s - DotProduct(-1.0f * a.v, b.v);
//	float result = 2.0f * ACosDegrees(newReal);
//
//	return result;
//}
//
//
////-------------------------------------------------------------------------------------------------
//Quaternion Quaternion::FromEuler(const Vector3& eulerAnglesDegrees)
//{
//	const Vector3 he = 0.5f * eulerAnglesDegrees;
//
//	float cx = CosDegrees(he.x);
//	float sx = SinDegrees(he.x);
//	float cy = CosDegrees(he.y);
//	float sy = SinDegrees(he.y);
//	float cz = CosDegrees(he.z);
//	float sz = SinDegrees(he.z);
//
//	float sValue = cx * cy * cz + sx * sy * sz;
//	float ix = sx * cy * cz + cx * sy * sz;
//	float iy = cx * sy * cz - sx * cy * sz;
//	float iz = cx * cy * sz - sx * sy * cz;
//
//
//	Quaternion result = Quaternion(Vector3(ix, iy, iz), sValue);
//	result.Normalize();
//
//	return result;
//}
//
//
////-------------------------------------------------------------------------------------------------
//Quaternion Quaternion::RotateToward(const Quaternion& start, const Quaternion& end, float maxAngleDegrees)
//{
//	float angleBetween = GetAngleBetweenDegrees(start, end);
//
//	if (angleBetween < 0.f)
//	{
//		angleBetween = -angleBetween;
//	}
//
//	if (AreMostlyEqual(angleBetween, 0.f))
//	{
//		return end;
//	}
//
//	float t = Clamp(maxAngleDegrees / angleBetween, 0.f, 1.0f);
//	Quaternion result = Slerp(start, end, t);
//
//	return result;
//}
//
//
////-----------------------------------------------------------------------------------------------
//// Spherically interpolates between quaternion a and b by an amount given by fractionTowardEnd
////
//Quaternion Quaternion::Slerp(const Quaternion& a, const Quaternion& b, float fractionTowardEnd)
//{
//	fractionTowardEnd = Clamp(fractionTowardEnd, 0.f, 1.0f);
//	float cosAngle = DotProduct(a, b);
//
//
//	Quaternion start;
//	if (cosAngle < 0.0f) {
//		// If it's negative - it means it's going the long way
//		// flip it.
//		start = -1.0f * a;
//		cosAngle = -cosAngle;
//	}
//	else {
//		start = a;
//	}
//
//	float f0, f1;
//	if (cosAngle >= .9999f) {
//		// very close - just linearly interpolate for speed
//		f0 = 1.0f - fractionTowardEnd;
//		f1 = fractionTowardEnd;
//	}
//	else {
//		float sinAngle = sqrtf(1.0f - cosAngle * cosAngle);
//		float angle = atan2f(sinAngle, cosAngle);
//
//		float den = 1.0f / sinAngle;
//		f0 = sinf((1.0f - fractionTowardEnd) * angle) * den;
//		f1 = sinf(fractionTowardEnd * angle) * den;
//	}
//
//	Quaternion r0 = start * f0;
//	Quaternion r1 = b * f1;
//
//	return Quaternion(r0.s + r1.s, r0.v + r1.v);
//}
//
//
////-----------------------------------------------------------------------------------------------
//// Returns the conjugate of this quaternion
////
//Quaternion Quaternion::GetConjugate() const
//{
//	Quaternion result;
//
//	result.s = s;
//	result.v = -1.0f * v;
//
//	return result;
//}
