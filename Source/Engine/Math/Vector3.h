///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 29th, 2019
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
class Vector2;

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** STRUCTS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                        *** GLOBALS AND STATICS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** CLASSES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                           *** C FUNCTIONS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Vector3
{

public:

	Vector3() {}
	Vector3(float value);
	Vector3(const Vector3& copyFrom);
	explicit Vector3(float initialX, float initialY, float initialZ);
	explicit Vector3(int initialX, int initialY, int initialZ);
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

	float	GetLength() const;
	float	GetLengthSquared() const;
	float	Normalize();							
	Vector3 GetNormalized() const;
	Vector2 XY() const;
	Vector2 XZ() const;

	static Vector3 Slerp(const Vector3& start, const Vector3& end, float percent);

public:

	const static Vector3 ZERO;
	const static Vector3 ONES;
	const static Vector3 X_AXIS;
	const static Vector3 Y_AXIS;
	const static Vector3 Z_AXIS;
	const static Vector3 MINUS_X_AXIS;
	const static Vector3 MINUS_Y_AXIS;
	const static Vector3 MINUS_Z_AXIS;

public:

	float x;
	float y;
	float z;

};
