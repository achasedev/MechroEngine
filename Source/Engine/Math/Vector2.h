///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 28th, 2019
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
class IntVector2;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Vector2
{

public:
	//-----Public Methods-----

	Vector2() {}
	Vector2(const Vector2& copyFrom);
	explicit Vector2(float initialX, float initialY);
	explicit Vector2(int initialX, int initialY);
	explicit Vector2(const IntVector2& intVector);
	explicit Vector2(float initialValue);
	~Vector2() {}

	const Vector2			operator+(const Vector2& vecToAdd) const;
	const Vector2			operator-(const Vector2& vecToSubtract) const;
	const Vector2			operator*(float uniformScale) const;
	const Vector2			operator/(float inverseScale) const;
	void					operator+=(const Vector2& vecToAdd);
	void					operator-=(const Vector2& vecToSubtract);
	void					operator*=(const float uniformScale);
	void					operator/=(const float uniformDivisor);
	void					operator=(const Vector2& copyFrom);
	bool					operator==(const Vector2& compare) const;
	bool					operator!=(const Vector2& compare) const;
	friend const Vector2	operator*(float uniformScale, const Vector2& vecToScale);

	float					Normalize();
	Vector2					GetNormalized() const;
	float					GetLength() const;
	float					GetLengthSquared() const;
	float					GetOrientationDegrees() const;

	static Vector2			MakeDirectionAtDegrees(float degrees);


public:
	//----- Public Static Data-----

	const static Vector2 ZERO;
	const static Vector2 ONES;
	const static Vector2 X_AXIS;
	const static Vector2 Y_AXIS;
	const static Vector2 MINUS_X_AXIS;
	const static Vector2 MINUS_Y_AXIS;


public:
	//-----Public Member Data-----
	// Various ways of accessing

	union
	{
		// Array access
		float data[2];

		// Coordinate access
		struct			
		{
			float x;
			float y;
		};

		// Texture coordinate access
		struct
		{
			float u;
			float v;
		};

		// Swizzles!
		// Must be unioned to not pad out the length of Vector2
		// Swizzles have no data, so no fear of overwrite
		union
		{
			Swizzle<Vector2, float, 0, 0> xx, uu;
			Swizzle<Vector2, float, 0, 1> xy, uv;
			Swizzle<Vector2, float, 1, 0> yx, vu;
			Swizzle<Vector2, float, 1, 1> yy, vv;
		};
	};
};

#pragma warning(default : 4201)

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

float GetDistance(const Vector2& a, const Vector2& b);
float GetDistanceSquared(const Vector2& a, const Vector2& b);
const Vector2 ProjectVector(const Vector2& vectorToProject, const Vector2& projectOnto);
const Vector2 Reflect(const Vector2& vectorToReflect, const Vector2& normal);
