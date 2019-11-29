///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 28th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** INCLUDES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** DEFINES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                              *** TYPES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------
class IntVector2;

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** STRUCTS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                        *** GLOBALS AND STATICS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** CLASSES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Vector2
{

public:

	Vector2() {}
	Vector2(const Vector2& copyFrom);
	explicit Vector2(float initialX, float initialY);
	explicit Vector2(int initialX, int initialY);
	explicit Vector2(const IntVector2& intVector);
	explicit Vector2(float initialValue);
	~Vector2() {}

	const	Vector2 operator+(const Vector2& vecToAdd) const;
	const	Vector2 operator-(const Vector2& vecToSubtract) const;
	const	Vector2 operator*(float uniformScale) const;
	const	Vector2 operator/(float inverseScale) const;
	void	operator+=(const Vector2& vecToAdd);
	void	operator-=(const Vector2& vecToSubtract);
	void	operator*=(const float uniformScale);
	void	operator/=(const float uniformDivisor);
	void	operator=(const Vector2& copyFrom);
	bool	operator==(const Vector2& compare) const;
	bool	operator!=(const Vector2& compare) const;
	friend const Vector2 operator*(float uniformScale, const Vector2& vecToScale);

	float	Normalize();
	Vector2 GetNormalized() const;
	float	GetLength() const;
	float	GetLengthSquared() const;
	float	GetOrientationDegrees() const;

	static Vector2 MakeDirectionAtDegrees(float degrees);

public:

	const static Vector2 ZERO;
	const static Vector2 ONES;
	const static Vector2 X_AXIS;
	const static Vector2 Y_AXIS;
	const static Vector2 MINUS_X_AXIS;
	const static Vector2 MINUS_Y_AXIS;

public:

	float x;
	float y;

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                           *** C FUNCTIONS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

float GetDistance(const Vector2& a, const Vector2& b);
float GetDistanceSquared(const Vector2& a, const Vector2& b);
const Vector2 ProjectVector(const Vector2& vectorToProject, const Vector2& projectOnto);
const Vector2 Reflect(const Vector2& vectorToReflect, const Vector2& normal);
