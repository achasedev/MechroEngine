///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 29th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Math/Vector3.h"

#include "Engine/Framework/EngineCommon.h"
#include "Engine/Math/MathUtils.h"
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
const Vector3 Vector3::ZERO = Vector3(0.f, 0.f, 0.f);
const Vector3 Vector3::ONES = Vector3(1.f, 1.f, 1.f);
const Vector3 Vector3::X_AXIS = Vector3(1.0f, 0.f, 0.f);
const Vector3 Vector3::Y_AXIS = Vector3(0.f, 1.0f, 0.f);
const Vector3 Vector3::Z_AXIS = Vector3(0.f, 0.f, 1.f);
const Vector3 Vector3::MINUS_X_AXIS = Vector3(-1.0f, 0.f, 0.f);
const Vector3 Vector3::MINUS_Y_AXIS = Vector3(0.f, -1.0f, 0.f);
const Vector3 Vector3::MINUS_Z_AXIS = Vector3(0.f, 0.f, -1.f);

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
Vector3::Vector3(const Vector3& copyFrom)
	: x(copyFrom.x)
	, y(copyFrom.y)
	, z(copyFrom.z)
{
}


//-------------------------------------------------------------------------------------------------
Vector3::Vector3(float initialX, float initialY, float initialZ)
	: x(initialX)
	, y(initialY)
	, z(initialZ)
{
}


//-------------------------------------------------------------------------------------------------
Vector3::Vector3(float value)
	: x(value), y(value), z(value)
{
}


//-------------------------------------------------------------------------------------------------
Vector3::Vector3(int initialX, int initialY, int initialZ)
	: x(static_cast<float>(initialX))
	, y(static_cast<float>(initialY))
	, z(static_cast<float>(initialZ))
{
}


//-------------------------------------------------------------------------------------------------
Vector3::Vector3(const Vector2& xyVector, float initialZ)
	: x(xyVector.x)
	, y(xyVector.y)
	, z(initialZ)
{
}


//-------------------------------------------------------------------------------------------------
const Vector3 Vector3::operator+(const Vector3& addVector) const
{
	return Vector3((x + addVector.x), (y + addVector.y), (z + addVector.z));
}


//-------------------------------------------------------------------------------------------------
const Vector3 Vector3::operator-(const Vector3& subVector) const
{
	return Vector3((x - subVector.x), (y - subVector.y), (z - subVector.z));
}


//-------------------------------------------------------------------------------------------------
const Vector3 Vector3::operator*(float uniformScaler) const
{
	return Vector3((x * uniformScaler), (y * uniformScaler), (z * uniformScaler));
}


//-------------------------------------------------------------------------------------------------
const Vector3 Vector3::operator/(float uniformDivisor) const
{
	float multScaler = (1.f / uniformDivisor);
	return Vector3((x * multScaler), (y * multScaler), (z * multScaler));
}


//-------------------------------------------------------------------------------------------------
void Vector3::operator+=(const Vector3& addVector)
{
	x += addVector.x;
	y += addVector.y;
	z += addVector.z;
}


//-------------------------------------------------------------------------------------------------
void Vector3::operator-=(const Vector3& subVector)
{
	x -= subVector.x;
	y -= subVector.y;
	z -= subVector.z;
}


//-------------------------------------------------------------------------------------------------
void Vector3::operator*=(const float uniformScaler)
{
	x *= uniformScaler;
	y *= uniformScaler;
	z *= uniformScaler;
}


//-------------------------------------------------------------------------------------------------
void Vector3::operator/=(const float uniformDivisor)
{
	float multScaler = (1.f / uniformDivisor);

	x *= multScaler;
	y *= multScaler;
	z *= multScaler;
}


//-------------------------------------------------------------------------------------------------
void Vector3::operator=(const Vector3& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}


//-------------------------------------------------------------------------------------------------
const Vector3 operator*(float uniformScaler, const Vector3& vecToScale)
{
	return Vector3((vecToScale.x * uniformScaler), (vecToScale.y * uniformScaler), (vecToScale.z * uniformScaler));
}


//-------------------------------------------------------------------------------------------------
bool Vector3::operator==(const Vector3& compare) const
{
	return (x == compare.x && y == compare.y && z == compare.z);
}


//-------------------------------------------------------------------------------------------------
bool Vector3::operator!=(const Vector3& compare) const
{
	return (x != compare.x || y != compare.y || z != compare.z);
}


//-------------------------------------------------------------------------------------------------
float Vector3::GetLength() const
{
	return sqrtf((x * x) + (y * y) + (z * z));
}


//-------------------------------------------------------------------------------------------------
float Vector3::GetLengthSquared() const
{
	return (x * x) + (y * y) + (z * z);
}


//-------------------------------------------------------------------------------------------------
float Vector3::Normalize()
{
	float length = GetLength();
	ASSERT_OR_DIE(length > 0, "Vector2::Normalize() called on a zero vector!");

	float oneOverLength = (1.f / length);

	x *= oneOverLength;
	y *= oneOverLength;
	z *= oneOverLength;

	return length;
}


//-------------------------------------------------------------------------------------------------
Vector3 Vector3::GetNormalized() const
{
	Vector3 normalizedForm = *this;
	normalizedForm.Normalize();

	return normalizedForm;
}


//-------------------------------------------------------------------------------------------------
Vector2 Vector3::XY() const
{
	return Vector2(x, y);
}


//-------------------------------------------------------------------------------------------------
Vector2 Vector3::XZ() const
{
	return Vector2(x, z);
}


//-------------------------------------------------------------------------------------------------
Vector3 Vector3::Slerp(const Vector3& start, const Vector3& end, float percent)
{
	float dot = DotProduct(start, end);

	dot = Clamp(dot, -1.0f, 1.0f);							// Clamp for safety
	float theta = acosf(dot) * percent;						// Angle between start and the result we want
	Vector3 relative = (end - start * dot).GetNormalized(); // Direction we need to move towards result

	Vector3 result = start * cosf(theta) + relative * sinf(theta);
	return result;
}
