///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 28th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/Utility/Swizzle.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Vector2;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma warning(disable : 4201) // Keep the structs anonymous so we can still do myVector.x even when x is part of a struct

//-------------------------------------------------------------------------------------------------
class IntVector2
{

public:
	//-----Public Methods-----

	IntVector2() {}
	IntVector2(const IntVector2& copyFrom);	
	explicit IntVector2(int initialValue);
	explicit IntVector2(int initialX, int initialY);
	explicit IntVector2(uint32 initialX, uint32 initialY);
	explicit IntVector2(float initialX, float initialY);
	explicit IntVector2(const Vector2& floatVector);
	~IntVector2() {}

	const IntVector2		operator+(const IntVector2& addVector) const;
	const IntVector2		operator-(const IntVector2& subVector) const;
	const IntVector2		operator*(int uniformScaler) const;
	const IntVector2		operator/(int uniformDivisor) const;
	void					operator+=(const IntVector2& vecToAdd);							
	void					operator-=(const IntVector2& vecToSubtract);
	void					operator*=(const int uniformScale);
	void					operator=(const IntVector2& copyFrom);		
	bool					operator==(const IntVector2& compare) const;
	bool					operator!=(const IntVector2& compare) const;
	friend const IntVector2 operator*(int uniformScale, const IntVector2& vecToScale);


public:
	//------Public Static Data-----

	const static IntVector2 ZERO;
	const static IntVector2 ONES;


public:
	//-----Public Data-----

	union
	{
		// Array access
		int data[2];

		// Coordinate access
		struct
		{
			int x;
			int y;
		};

		// Texture coordinate access
		struct
		{
			int u;
			int v;
		};

		// Swizzles!
		// Must be unioned to not pad out the length of Vector2
		// Swizzles have no data, so no fear of overwrite
		union
		{
			Swizzle<IntVector2, int, 0, 0> xx, uu;
			Swizzle<IntVector2, int, 0, 1> xy, uv;
			Swizzle<IntVector2, int, 1, 0> yx, vu;
			Swizzle<IntVector2, int, 1, 1> yy, vv;
		};
	};
};

#pragma warning(default : 4201)

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
