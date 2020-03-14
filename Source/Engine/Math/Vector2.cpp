///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 28th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Math/Vector2.h"

#include "Engine/Framework/EngineCommon.h"
#include "Engine/Math/IntVector2.h"
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
const Vector2 Vector2::ZERO = Vector2(0.f, 0.f);
const Vector2 Vector2::ONES = Vector2(1.f, 1.f);
const Vector2 Vector2::X_AXIS = Vector2(1.0f, 0.f);
const Vector2 Vector2::Y_AXIS = Vector2(0.f, 1.0f);
const Vector2 Vector2::MINUS_X_AXIS = Vector2(-1.0f, 0.f);
const Vector2 Vector2::MINUS_Y_AXIS = Vector2(0.f, -1.0f);

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
Vector2::Vector2(const Vector2& copy)
	: x(copy.x)
	, y(copy.y)
{
}


//-------------------------------------------------------------------------------------------------
Vector2::Vector2(float initialX, float initialY)
	: x(initialX)
	, y(initialY)
{
}


//-----------------------------------------------------------------------------------------------
// IntVector2 constructor
Vector2::Vector2(const IntVector2& copyFrom)
{
	x = static_cast<float>(copyFrom.x);
	y = static_cast<float>(copyFrom.y);
}


//-------------------------------------------------------------------------------------------------
Vector2::Vector2(int initialX, int initialY)
	: x(static_cast<float>(initialX))
	, y(static_cast<float>(initialY))
{
}


//-------------------------------------------------------------------------------------------------
Vector2::Vector2(float initialValue)
	: x(initialValue)
	, y(initialValue)
{
}


//-------------------------------------------------------------------------------------------------
const Vector2 Vector2::operator + (const Vector2& vecToAdd) const
{
	return Vector2((x + vecToAdd.x), (y + vecToAdd.y));
}


//-------------------------------------------------------------------------------------------------
const Vector2 Vector2::operator-(const Vector2& vecToSubtract) const
{
	return Vector2((x - vecToSubtract.x), (y - vecToSubtract.y));
}


//-------------------------------------------------------------------------------------------------
const Vector2 Vector2::operator*(float uniformScale) const
{
	return Vector2((x * uniformScale), (y * uniformScale));
}


//-------------------------------------------------------------------------------------------------
const Vector2 Vector2::operator/(float inverseScale) const
{
	float multScaler = (1.f / inverseScale);
	return Vector2((x * multScaler), (y * multScaler));
}


//-------------------------------------------------------------------------------------------------
void Vector2::operator+=(const Vector2& vecToAdd)
{
	x += vecToAdd.x;
	y += vecToAdd.y;
}


//-------------------------------------------------------------------------------------------------
void Vector2::operator-=(const Vector2& vecToSubtract)
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
}


//-------------------------------------------------------------------------------------------------
void Vector2::operator*=(const float uniformScale)
{
	x *= uniformScale;
	y *= uniformScale;
}


//-------------------------------------------------------------------------------------------------
void Vector2::operator/=(const float uniformDivisor)
{
	float multScaler = (1.f / uniformDivisor);

	x *= multScaler;
	y *= multScaler;
}


//-------------------------------------------------------------------------------------------------
void Vector2::operator=(const Vector2& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
}


//-------------------------------------------------------------------------------------------------
const Vector2 operator*(float uniformScale, const Vector2& vecToScale)
{
	return Vector2((vecToScale.x * uniformScale), (vecToScale.y * uniformScale));
}


//-------------------------------------------------------------------------------------------------
bool Vector2::operator==(const Vector2& compare) const
{
	if (x == compare.x && y == compare.y) {
		return true;
	}
	return false;
}


//-------------------------------------------------------------------------------------------------
bool Vector2::operator!=(const Vector2& compare) const
{
	if (x != compare.x || y != compare.y) {
		return true;
	}
	return false;
}


//-------------------------------------------------------------------------------------------------
float Vector2::GetLength() const
{
	return sqrtf((x * x) + (y * y));
}


//-------------------------------------------------------------------------------------------------
float Vector2::GetLengthSquared() const
{
	return (x * x) + (y * y);
}


//-------------------------------------------------------------------------------------------------
float Vector2::Normalize()
{
	float length = GetLength();
	ASSERT_OR_DIE(length > 0, "Vector2::Normalize() called on a zero vector!");

	float oneOverLength = (1.f / length);

	x *= oneOverLength;
	y *= oneOverLength;

	return length;
}


//-------------------------------------------------------------------------------------------------
Vector2 Vector2::GetNormalized() const
{
	Vector2 normalizedForm = *this;
	normalizedForm.Normalize();
	
	return normalizedForm;
}


//-------------------------------------------------------------------------------------------------
float Vector2::GetOrientationDegrees() const
{
	// Ensure we have a valid vector to calculate on
	ASSERT_OR_DIE((x != 0.f || y != 0.f), "Error: Vector2::GetOrientationDegrees() called on a zero vector!");
	return Atan2Degrees(y, x);
}


//-------------------------------------------------------------------------------------------------
Vector2 Vector2::MakeDirectionAtDegrees(float degrees)
{
	Vector2 direction;

	direction.x = CosDegrees(degrees);
	direction.y = SinDegrees(degrees);

	return direction;
}


//-------------------------------------------------------------------------------------------------
float GetDistance(const Vector2& a, const Vector2& b)
{
	return sqrtf(((a.x - b.x) * (a.x - b.x)) + ((a.y - b.y) * (a.y - b.y)));
}


//-------------------------------------------------------------------------------------------------
float GetDistanceSquared(const Vector2& a, const Vector2& b)
{
	return ((a.x - b.x) * (a.x - b.x)) + ((a.y - b.y) * (a.y - b.y));
}


//-------------------------------------------------------------------------------------------------
const Vector2 ProjectVector(const Vector2& vectorToProject, const Vector2& projectOnto)
{
	// Optimized for efficiency - using distance squared instead of distance
	float projectOntoMagnitudeSquared = projectOnto.GetLengthSquared();

	float dotProdcut = DotProduct(vectorToProject, projectOnto);

	return (dotProdcut / projectOntoMagnitudeSquared) * projectOnto;
}


//-------------------------------------------------------------------------------------------------
const Vector2 Reflect(const Vector2& vectorToReflect, const Vector2& normal)
{
	// Ensure normal is normalized
	Vector2 normalDirection = normal.GetNormalized();

	float magnitudeInNormalDirection = DotProduct(vectorToReflect, normalDirection);
	Vector2 componentInNormalDirection = magnitudeInNormalDirection * normalDirection;

	// Remove the normal, then add its inverse, essentially removing it twice
	Vector2 reflectedResult = vectorToReflect - (2.f * componentInNormalDirection);

	return reflectedResult;
}
