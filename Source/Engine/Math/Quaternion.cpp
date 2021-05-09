///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 17th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/Matrix3.h"
#include "Engine/Math/Matrix4.h"
#include "Engine/Math/ScaledAxisRotation.h"
#include "Engine/Math/Quaternion.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------
const Quaternion Quaternion::IDENTITY = Quaternion();

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
Quaternion::Quaternion(float real, const Vector3& complexVector)
	: v(complexVector), real(real)
{
}


//-------------------------------------------------------------------------------------------------
Quaternion::Quaternion()
	: v(Vector3::ZERO), real(1.f)
{
}


//-------------------------------------------------------------------------------------------------
Quaternion::Quaternion(const Quaternion& copy)
	: v(copy.v), real(copy.real)
{
}


//-------------------------------------------------------------------------------------------------
Quaternion::Quaternion(float _real, float vx, float vy, float vz)
	: v(Vector3(vx, vy, vz)), real(_real)
{
}


//-------------------------------------------------------------------------------------------------
const Quaternion Quaternion::operator+(const Quaternion& other) const
{
	Quaternion result;
	result.v = v + other.v;
	result.real = real + other.real;

	return result;
}


//-------------------------------------------------------------------------------------------------
const Quaternion Quaternion::operator-(const Quaternion& other) const
{
	Quaternion result;
	result.v = v - other.v;
	result.real = real - other.real;

	return result;
}


//-------------------------------------------------------------------------------------------------
const Quaternion Quaternion::operator*(const Quaternion& other) const
{
	Quaternion result;

	result.real = real * other.real - DotProduct(v, other.v);
	result.v = real * other.v + v * other.real + CrossProduct(v, other.v);

	result.Normalize();

	return result;
}


//-------------------------------------------------------------------------------------------------
const Quaternion Quaternion::operator*(float scalar) const
{
	Quaternion result;

	result.real = real * scalar;
	result.v = v * scalar;

	return result;
}


//-------------------------------------------------------------------------------------------------
const Quaternion operator*(float scalar, const Quaternion& quat)
{
	Quaternion result;

	result.real = (scalar * quat.real);
	result.v = (scalar * quat.v);

	return result;
}


//-------------------------------------------------------------------------------------------------
Quaternion Quaternion::Lerp(const Quaternion& a, const Quaternion& b, float fractionTowardEnd)
{
	float sResult = Interpolate(a.real, b.real, fractionTowardEnd);
	Vector3 vResult = Interpolate(a.v, b.v, fractionTowardEnd);

	return Quaternion(sResult, vResult);
}


//-------------------------------------------------------------------------------------------------
void Quaternion::operator*=(const Quaternion& other)
{
	Quaternion old = (*this);

	real = old.real * other.real - DotProduct(old.v, other.v);
	v = old.real * other.v + old.v * other.real + CrossProduct(old.v, other.v);

	Normalize();
}


//-------------------------------------------------------------------------------------------------
void Quaternion::operator*=(float scalar)
{
	real = real * scalar;
	v = v * scalar;
}



//-------------------------------------------------------------------------------------------------
void Quaternion::operator+=(const Quaternion& other)
{
	v += other.v;
	real += other.real;
}


//-------------------------------------------------------------------------------------------------
void Quaternion::operator-=(const Quaternion& other)
{
	v -= other.v;
	real -= other.real;
}


//-------------------------------------------------------------------------------------------------
void Quaternion::operator=(const Quaternion& copy)
{
	v = copy.v;
	real = copy.real;
}


//-------------------------------------------------------------------------------------------------
float Quaternion::GetMagnitude() const
{
	return sqrtf(GetMagnitudeSquared());
}


//-------------------------------------------------------------------------------------------------
float Quaternion::GetMagnitudeSquared() const
{
	return (real * real) + (v.x * v.x) + (v.y * v.y) + (v.z * v.z);
}


//-------------------------------------------------------------------------------------------------
float Quaternion::DecomposeIntoAxisAndRadianAngle(Vector3& out_axis) const
{
	float magnitude = GetMagnitude();
	ASSERT_RETURN(magnitude > 0.f, 0.f, "Degenerate Quaternion!");
	float invMag = 1.0f / magnitude;

	out_axis = (invMag * v).GetNormalized();
	return 2.f * acosf(real);
}


//-------------------------------------------------------------------------------------------------
Quaternion Quaternion::GetNormalized() const
{
	float magnitude = GetMagnitude();
	if (magnitude == 0.f)
	{
		return (*this);
	}

	float oneOverMag = (1.f / GetMagnitude());
	Quaternion result = (*this);

	return result * oneOverMag;
}


//-------------------------------------------------------------------------------------------------
Quaternion Quaternion::GetInverse() const
{
	float absoluteValue = GetMagnitude();
	absoluteValue *= absoluteValue;
	absoluteValue = 1.0f / absoluteValue;

	Quaternion conjugate = GetConjugate();

	float scalar = conjugate.real * absoluteValue;
	Vector3 vector = conjugate.v * absoluteValue;

	return Quaternion(scalar, vector);
}


//-------------------------------------------------------------------------------------------------
Vector3 Quaternion::GetAsEulerAnglesDegrees() const
{
	Matrix3 matrix(*this);
	return Matrix3::ExtractRotationAsEulerAnglesDegrees(matrix);
}


//-------------------------------------------------------------------------------------------------
Vector3 Quaternion::GetAsEulerAnglesRadians() const
{
	Matrix3 matrix(*this);
	return Matrix3::ExtractRotationAsEulerAnglesRadians(matrix);
}


//-------------------------------------------------------------------------------------------------
void Quaternion::Normalize()
{
	(*this) = GetNormalized();
}


//-------------------------------------------------------------------------------------------------
void Quaternion::ConvertToUnitNorm()
{
	float angleDegrees = real;

	v.Normalize();
	real = CosDegrees(0.5f * angleDegrees);
	v = (v * SinDegrees(0.5f * angleDegrees));
}


//-------------------------------------------------------------------------------------------------
Vector3 Quaternion::RotatePosition(const Vector3& position) const
{
	Quaternion pointAsQuat = Quaternion(0.f, position);

	Quaternion inverse = GetInverse();
	//Quaternion rotatedResult = inverse * pointAsQuat * (*this);
	Quaternion rotatedResult = (*this) * pointAsQuat * inverse;

	ASSERT_OR_DIE(AreMostlyEqual(rotatedResult.real, 0.f), "This should be zero!");

	return rotatedResult.v;
}


//-------------------------------------------------------------------------------------------------
Vector3 Quaternion::InverseRotatePosition(const Vector3& position) const
{
	Quaternion pointAsQuat = Quaternion(0.f, position);

	Quaternion inverse = GetInverse();
	Quaternion rotatedResult = inverse * pointAsQuat * (*this);

	ASSERT_OR_DIE(AreMostlyEqual(rotatedResult.real, 0.f), "This should be zero!");

	return rotatedResult.v;
}


//-------------------------------------------------------------------------------------------------
float Quaternion::GetAngleBetweenDegrees(const Quaternion& a, const Quaternion& b)
{
	float newReal = a.real * b.real - DotProduct(-1.0f * a.v, b.v);
	float result = 2.0f * ACosDegrees(newReal);

	return result;
}


//-------------------------------------------------------------------------------------------------
Quaternion Quaternion::CreateFromEulerAnglesDegrees(const Vector3& eulerAnglesDegrees)
{
	return CreateFromEulerAnglesRadians(DegreesToRadians(eulerAnglesDegrees));
}


//-------------------------------------------------------------------------------------------------
Quaternion Quaternion::CreateFromEulerAnglesDegrees(float xDegrees, float yDegrees, float zDegrees)
{
	return CreateFromEulerAnglesDegrees(Vector3(xDegrees, yDegrees, zDegrees));
}


//-------------------------------------------------------------------------------------------------
Quaternion Quaternion::CreateFromEulerAnglesRadians(const Vector3& radianAngles)
{
	const Vector3 he = 0.5f * radianAngles;

	float cx = cosf(he.x);
	float sx = sinf(he.x);
	float cy = cosf(he.y);
	float sy = sinf(he.y);
	float cz = cosf(he.z);
	float sz = sinf(he.z);

	float r = cx * cy*cz + sx * sy*sz;
	float ix = sx * cy*cz + cx * sy*sz;
	float iy = cx * sy*cz - sx * cy*sz;
	float iz = cx * cy*sz - sx * sy*cz;


	Quaternion result = Quaternion(r, Vector3(ix, iy, iz));
	result.Normalize();

	return result;
}


//-------------------------------------------------------------------------------------------------
Quaternion Quaternion::CreateFromEulerAnglesRadians(float xRadians, float yRadians, float zRadians)
{
	return CreateFromEulerAnglesRadians(Vector3(xRadians, yRadians, zRadians));
}


//-------------------------------------------------------------------------------------------------
Quaternion Quaternion::CreateFromAxisAndRadianAngle(const Vector3& axis, float radians)
{
	float he = 0.5f * radians;
	float real = cosf(he);
	Vector3 vector = axis.GetNormalized() * sinf(he);

	return Quaternion(real, vector);
}


//-------------------------------------------------------------------------------------------------
Quaternion Quaternion::CreateFromAxisAndDegreeAngle(const Vector3& axis, float degrees)
{
	return CreateFromAxisAndRadianAngle(axis, DegreesToRadians(degrees));
}


//-------------------------------------------------------------------------------------------------
Quaternion Quaternion::CreateFromScaledAxisDegrees(const ScaledAxisRotation& scaledAxisDegrees)
{
	Vector3 axis = scaledAxisDegrees.data;
	float angleDegrees = axis.Normalize();

	return CreateFromAxisAndDegreeAngle(axis, angleDegrees);
}


//-------------------------------------------------------------------------------------------------
Quaternion Quaternion::CreateFromScaledAxisRadians(const ScaledAxisRotation& scaledAxisRadians)
{
	Vector3 axis = scaledAxisRadians.data;
	float angleRadians = axis.Normalize();

	return CreateFromAxisAndRadianAngle(axis, angleRadians);
}


//-------------------------------------------------------------------------------------------------
Quaternion Quaternion::FromMatrix(const Matrix4& rotationMatrix)
{
	// TODO: Faster way to do this?
	return Quaternion::CreateFromEulerAnglesDegrees(Matrix4::ExtractRotationAsEulerAnglesDegrees(rotationMatrix));
}


//-------------------------------------------------------------------------------------------------
Quaternion Quaternion::RotateToward(const Quaternion& start, const Quaternion& end, float maxAngleDegrees)
{
	float angleBetween = GetAngleBetweenDegrees(start, end);

	if (angleBetween < 0.f)
	{
		angleBetween = -angleBetween;
	}

	if (AreMostlyEqual(angleBetween, 0.f))
	{
		return end;
	}

	float t = Clamp(maxAngleDegrees / angleBetween, 0.f, 1.0f);
	Quaternion result = Slerp(start, end, t);

	return result;
}


//-------------------------------------------------------------------------------------------------
Quaternion Quaternion::Slerp(const Quaternion& a, const Quaternion& b, float fractionTowardEnd)
{
	fractionTowardEnd = Clamp(fractionTowardEnd, 0.f, 1.0f);
	float cosAngle = DotProduct(a, b);


	Quaternion start;
	if (cosAngle < 0.0f) {
		// If it's negative - it means it's going the long way
		// flip it.
		start = -1.0f * a;
		cosAngle = -cosAngle;
	}
	else {
		start = a;
	}

	float f0, f1;
	if (cosAngle >= .9999f) {
		// very close - just linearly interpolate for speed
		f0 = 1.0f - fractionTowardEnd;
		f1 = fractionTowardEnd;
	}
	else {
		float sinAngle = sqrtf(1.0f - cosAngle * cosAngle);
		float angle = atan2f(sinAngle, cosAngle);

		float den = 1.0f / sinAngle;
		f0 = sinf((1.0f - fractionTowardEnd) * angle) * den;
		f1 = sinf(fractionTowardEnd * angle) * den;
	}

	Quaternion r0 = start * f0;
	Quaternion r1 = b * f1;

	return Quaternion(r0.real + r1.real, r0.v + r1.v);
}


//-------------------------------------------------------------------------------------------------
Quaternion Quaternion::GetConjugate() const
{
	Quaternion result;

	result.real = real;
	result.v = -1.0f * v;

	return result;
}
