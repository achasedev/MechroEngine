///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 28th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Math/IntVector2.h"
#include "Engine/Math/IntVector3.h"
#include "Engine/Math/Vector3.h"
#include <math.h>

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                              *** DEFINES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------
const IntVector3 IntVector3::ZERO = IntVector3(0, 0, 0);
const IntVector3 IntVector3::ONES = IntVector3(1, 1, 1);

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                            *** C FUNCTIONS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                              *** CLASSES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
IntVector3::IntVector3(const IntVector3& copy)
	: x(copy.x)
	, y(copy.y)
	, z(copy.z)
{
}


//-------------------------------------------------------------------------------------------------
IntVector3::IntVector3(int initialX, int initialY, int initialZ)
	: x(initialX)
	, y(initialY)
	, z(initialZ)
{
}


//-------------------------------------------------------------------------------------------------
IntVector3::IntVector3(float initialX, float initialY, float initialZ)
	: x(static_cast<int>(initialX))
	, y(static_cast<int>(initialY))
	, z(static_cast<int>(initialZ))
{
}


//-------------------------------------------------------------------------------------------------
IntVector3::IntVector3(const Vector3& floatVector)
	: x(static_cast<int>(floatVector.x))
	, y(static_cast<int>(floatVector.y))
	, z(static_cast<int>(floatVector.z))
{
}


//-------------------------------------------------------------------------------------------------
IntVector3::IntVector3(int initialValue)
	: x(initialValue)
	, y(initialValue)
	, z(initialValue)
{
}


//-------------------------------------------------------------------------------------------------
IntVector3::IntVector3(uint32 initialX, uint32 initialY, uint32 initialZ)
	: x(static_cast<int>(initialX))
	, y(static_cast<int>(initialY))
	, z(static_cast<int>(initialZ))
{
}


//-------------------------------------------------------------------------------------------------
IntVector3::IntVector3(const IntVector2& xyVector, int initialZ)
	: x(xyVector.x)
	, y(xyVector.y)
	, z(initialZ)
{
}


//-------------------------------------------------------------------------------------------------
const IntVector3 IntVector3::operator+(const IntVector3& addVector) const
{
	return IntVector3((x + addVector.x), (y + addVector.y), (z + addVector.z));
}


//-------------------------------------------------------------------------------------------------
const IntVector3 IntVector3::operator-(const IntVector3& subVector) const
{
	return IntVector3((x - subVector.x), (y - subVector.y), (z - subVector.z));
}


//-------------------------------------------------------------------------------------------------
const IntVector3 IntVector3::operator*(int uniformScale) const
{
	return IntVector3((x * uniformScale), (y * uniformScale), (z * uniformScale));
}


//-------------------------------------------------------------------------------------------------
bool IntVector3::operator<(const IntVector3& other) const
{
	if (x < other.x)
	{
		return true;
	}
	else if (x == other.x)
	{
		if (z < other.z)
		{
			return true;
		}
		else if (z == other.z)
		{
			if (y < other.y)
			{
				return true;
			}
		}
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
const IntVector3 IntVector3::operator/(int divisor) const
{
	return IntVector3((x / divisor), (y / divisor), (z / divisor));
}


//-------------------------------------------------------------------------------------------------
void IntVector3::operator+=(const IntVector3& addVector)
{
	x += addVector.x;
	y += addVector.y;
	z += addVector.z;
}


//-------------------------------------------------------------------------------------------------
void IntVector3::operator-=(const IntVector3& subVector)
{
	x -= subVector.x;
	y -= subVector.y;
	z -= subVector.z;
}


//-------------------------------------------------------------------------------------------------
void IntVector3::operator*=(const int uniformScaler)
{
	x *= uniformScaler;
	y *= uniformScaler;
	z *= uniformScaler;
}


//-------------------------------------------------------------------------------------------------
void IntVector3::operator=(const IntVector3& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}


//-------------------------------------------------------------------------------------------------
const IntVector3 operator*(int uniformScaler, const IntVector3& vecToScale)
{
	return vecToScale * uniformScaler;
}


//-------------------------------------------------------------------------------------------------
bool IntVector3::operator==(const IntVector3& compare) const
{
	return (x == compare.x && y == compare.y && z == compare.z);
}


//-------------------------------------------------------------------------------------------------
bool IntVector3::operator!=(const IntVector3& compare) const
{
	return (x != compare.x || y != compare.y || z != compare.z);
}
