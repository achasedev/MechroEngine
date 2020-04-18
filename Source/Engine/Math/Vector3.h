///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 29th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once
#pragma warning(disable : 4201) // Keep the structs anonymous so we can still do myVector.x even when x is part of a struct

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
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

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Vector3
{

public:
	//-----Public Methods-----

	Vector3() {}
	Vector3(float value);
	Vector3(const Vector3& copyFrom);
	explicit Vector3(float initialX, float initialY, float initialZ);
	explicit Vector3(int initialX, int initialY, int initialZ);
	explicit Vector3(const Vector2& xyVector, float initialZ);
	~Vector3() {}

	const Vector3			operator+(const Vector3& addVector) const;
	const Vector3			operator-(const Vector3& subVector) const;
	const Vector3			operator*(float uniformScaler) const;
	const Vector3			operator/(float uniformDivisor) const;
	void					operator+=(const Vector3& addVector);
	void					operator-=(const Vector3& subVector);
	void					operator*=(const float uniformScaler);
	void					operator/=(const float uniformDivisor);
	void					operator=(const Vector3& copyFrom);
	bool					operator==(const Vector3& compare) const;
	bool					operator!=(const Vector3& compare) const;
	friend const Vector3	operator*(float uniformScaler, const Vector3& vecToScale);

	float					GetLength() const;
	float					GetLengthSquared() const;
	float					Normalize();							
	Vector3					GetNormalized() const;

	static Vector3			Slerp(const Vector3& start, const Vector3& end, float percent);


public:
	//-----Public Static Data-----

	const static Vector3 ZERO;
	const static Vector3 ONES;
	const static Vector3 X_AXIS;
	const static Vector3 Y_AXIS;
	const static Vector3 Z_AXIS;
	const static Vector3 MINUS_X_AXIS;
	const static Vector3 MINUS_Y_AXIS;
	const static Vector3 MINUS_Z_AXIS;


public:
	//-----Public Member Data-----

	union
	{
		// Array access
		float data[3];

		// Coordinate access
		struct
		{
			float x;
			float y;
			float z;
		};

		// Color channel access
		struct
		{
			float r;
			float g;
			float b;
		};

		// Swizzles!
		// Must be unioned to not pad out the length of Vector2
		// Swizzles have no data, so no fear of overwrite
		union
		{
			Swizzle<Vector3, float, 0, 0, 0> xxx, rrr;
			Swizzle<Vector3, float, 1, 1, 1> yyy, ggg;
			Swizzle<Vector3, float, 2, 2, 2> zzz, bbb;

			Swizzle<Vector2, float, 0, 0> xx;
			Swizzle<Vector2, float, 0, 1> xy;
			Swizzle<Vector2, float, 0, 2> xz;
			Swizzle<Vector2, float, 1, 0> yx;
			Swizzle<Vector2, float, 1, 1> yy;
			Swizzle<Vector2, float, 1, 2> yz;
			Swizzle<Vector2, float, 2, 0> zx;
			Swizzle<Vector2, float, 2, 1> zy;
			Swizzle<Vector2, float, 2, 2> zz;
		};
	};
};
#pragma warning(default : 4201)