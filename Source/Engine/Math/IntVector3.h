///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 2nd, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Utility/Swizzle.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class IntVector2;
class Vector3;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma warning(disable : 4201) // Keep the structs anonymous in the union for usability

//-------------------------------------------------------------------------------------------------
class IntVector3
{

public:
	//-----Public Methods-----

	IntVector3(int initialValue);
	IntVector3() {}
	IntVector3(const IntVector3& copyFrom);
	explicit IntVector3(int initialX, int initialY, int initialZ);
	explicit IntVector3(uint32 initialX, uint32 initialY, uint32 initialZ);
	explicit IntVector3(float initialX, float initialY, float initialZ);
	explicit IntVector3(const Vector3& floatVector);
	explicit IntVector3(const IntVector2& xyVector, int initialZ);
	~IntVector3() {}

	const IntVector3		operator+(const IntVector3& addVector) const;
	const IntVector3		operator-(const IntVector3& subVector) const;
	const IntVector3		operator*(int uniformScaler) const;
	const IntVector3		operator/(int uniformDivisor) const;
	void					operator+=(const IntVector3& vecToAdd);
	void					operator-=(const IntVector3& vecToSubtract);
	void					operator*=(const int uniformScale);
	void					operator=(const IntVector3& copyFrom);
	bool					operator==(const IntVector3& compare) const;
	bool					operator!=(const IntVector3& compare) const;
	friend const IntVector3 operator*(int uniformScale, const IntVector3& vecToScale);

public:
	//-----Public Static Data-----

	const static IntVector3 ZERO;
	const static IntVector3 ONES;


public:
	//-----Public Data-----

	union
	{
		// Array access
		int data[3];

		// Coordinate access
		struct
		{
			int x;
			int y;
			int z;
		};

		// Color channel access
		struct
		{
			int r;
			int g;
			int b;
		};

		// Swizzles!
		// Must be unioned to not pad out the length of Vector2
		// Swizzles have no data, so no fear of overwrite
		union
		{
			Swizzle<IntVector3, int, 0, 0, 0> xxx, rrr;
			Swizzle<IntVector3, int, 1, 1, 1> yyy, ggg;
			Swizzle<IntVector3, int, 2, 2, 2> zzz, bbb;

			Swizzle<IntVector2, int, 0, 0> xx;
			Swizzle<IntVector2, int, 0, 1> xy;
			Swizzle<IntVector2, int, 0, 2> xz;
			Swizzle<IntVector2, int, 1, 0> yx;
			Swizzle<IntVector2, int, 1, 1> yy;
			Swizzle<IntVector2, int, 1, 2> yz;
			Swizzle<IntVector2, int, 2, 0> zx;
			Swizzle<IntVector2, int, 2, 1> zy;
			Swizzle<IntVector2, int, 2, 2> zz;
		};
	};
};

#pragma warning(default : 4201)

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
