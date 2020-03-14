///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: December 8th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector4.h"
#include <math.h>

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------
const Vector4 Vector4::ZERO = Vector4(0.f, 0.f, 0.f, 1.0f);
const Vector4 Vector4::ONES = Vector4(1.f, 1.f, 1.f, 1.0f);
const Vector4 Vector4::X_AXIS = Vector4(1.0f, 0.f, 0.f, 1.0f);
const Vector4 Vector4::Y_AXIS = Vector4(0.f, 1.0f, 0.f, 1.0f);
const Vector4 Vector4::Z_AXIS = Vector4(0.f, 0.f, 1.f, 1.0f);
const Vector4 Vector4::MINUS_X_AXIS = Vector4(-1.0f, 0.f, 0.f, 1.0f);
const Vector4 Vector4::MINUS_Y_AXIS = Vector4(0.f, -1.0f, 0.f, 1.0f);
const Vector4 Vector4::MINUS_Z_AXIS = Vector4(0.f, 0.f, -1.f, 1.0f);

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
Vector4::Vector4(const Vector4& copy)
	: x(copy.x)
	, y(copy.y)
	, z(copy.z)
	, w(copy.w)
{
}


//-------------------------------------------------------------------------------------------------
Vector4::Vector4(float initialX, float initialY, float initialZ, float initialW)
	: x(initialX)
	, y(initialY)
	, z(initialZ)
	, w(initialW)
{
}


//-------------------------------------------------------------------------------------------------
Vector4::Vector4(const Vector3& xyzVector, float wValue)
	: x(xyzVector.x), y(xyzVector.y), z(xyzVector.z), w(wValue)
{
}


//-------------------------------------------------------------------------------------------------
const Vector4 Vector4::operator+(const Vector4& addVector) const
{
	return Vector4((x + addVector.x), (y + addVector.y), (z + addVector.z), (w + addVector.w));
}


//-------------------------------------------------------------------------------------------------
const Vector4 Vector4::operator-(const Vector4& subVector) const
{
	return Vector4((x - subVector.x), (y - subVector.y), (z - subVector.z), (w - subVector.w));
}


//-------------------------------------------------------------------------------------------------
const Vector4 Vector4::operator*(float uniformScaler) const
{
	return Vector4((x * uniformScaler), (y * uniformScaler), (z * uniformScaler), (w * uniformScaler));
}


//-------------------------------------------------------------------------------------------------
const Vector4 Vector4::operator/(float uniformDivisor) const
{
	float multScaler = (1.f / uniformDivisor);
	return Vector4((x * multScaler), (y * multScaler), (z * multScaler), (w * multScaler));
}


//-------------------------------------------------------------------------------------------------
void Vector4::operator+=(const Vector4& addVector)
{
	x += addVector.x;
	y += addVector.y;
	z += addVector.z;
	w += addVector.w;
}


//-------------------------------------------------------------------------------------------------
void Vector4::operator-=(const Vector4& subVector)
{
	x -= subVector.x;
	y -= subVector.y;
	z -= subVector.z;
	w -= subVector.w;
}


//-------------------------------------------------------------------------------------------------
void Vector4::operator*=(const float uniformScaler)
{
	x *= uniformScaler;
	y *= uniformScaler;
	z *= uniformScaler;
	w *= uniformScaler;
}


//-------------------------------------------------------------------------------------------------
void Vector4::operator/=(const float uniformDivisor)
{
	float multScaler = (1.f / uniformDivisor);

	x *= multScaler;
	y *= multScaler;
	z *= multScaler;
	w *= multScaler;
}


//-------------------------------------------------------------------------------------------------
void Vector4::operator=(const Vector4& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
	w = copyFrom.w;
}


//-------------------------------------------------------------------------------------------------
const Vector4 operator*(float uniformScaler, const Vector4& vecToScale)
{
	return Vector4((vecToScale.x * uniformScaler), (vecToScale.y * uniformScaler), (vecToScale.z * uniformScaler), (vecToScale.w * uniformScaler));
}


//-------------------------------------------------------------------------------------------------
bool Vector4::operator==(const Vector4& compare) const
{
	return (x == compare.x && y == compare.y && z == compare.z && w == compare.w);
}


//-------------------------------------------------------------------------------------------------
bool Vector4::operator!=(const Vector4& compare) const
{
	return (x != compare.x || y != compare.y || z != compare.z || w != compare.w);
}


//-------------------------------------------------------------------------------------------------
float Vector4::GetLength() const
{
	return sqrtf((x * x) + (y * y) + (z * z) + (w * w));
}


//-------------------------------------------------------------------------------------------------
float Vector4::GetLengthSquared() const
{
	return (x * x) + (y * y) + (z * z) + (w * w);
}


//-------------------------------------------------------------------------------------------------
float Vector4::Normalize()
{
	float length = GetLength();
	float oneOverLength = (1.f / length);

	x *= oneOverLength;
	y *= oneOverLength;
	z *= oneOverLength;
	w *= oneOverLength;

	return length;
}


//-------------------------------------------------------------------------------------------------
Vector4 Vector4::GetNormalized() const
{
	Vector4 normalizedForm = *this;
	normalizedForm.Normalize();

	return normalizedForm;
}


//-------------------------------------------------------------------------------------------------
Vector2 Vector4::xz() const
{
	return Vector2(x, z);
}


//-------------------------------------------------------------------------------------------------
Vector3 Vector4::xyz() const
{
	return Vector3(x, y, z);
}
