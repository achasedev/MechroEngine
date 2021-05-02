///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 17th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Math/Vector3.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Matrix4;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class Quaternion
{
public:
	//-----Public Methods-----

	Quaternion();
	Quaternion(float real, const Vector3& vector);
	Quaternion(float real, float vx, float vy, float vz);
	Quaternion(const Quaternion& copy);
	~Quaternion() {}

	void operator=(const Quaternion& copy);
	const Quaternion operator+(const Quaternion& other) const;
	const Quaternion operator-(const Quaternion& other) const;
	const Quaternion operator*(const Quaternion& other) const;

	const Quaternion operator*(float scalar) const;
	friend const Quaternion operator*(float scalar, const Quaternion& quat);

	void operator+=(const Quaternion& other);
	void operator-=(const Quaternion& other);
	void operator*=(const Quaternion& other);
	void operator*=(float scalar);

	float				GetMagnitude() const;
	float				GetMagnitudeSquared() const;
	float				DecomposeIntoAxisAndRadianAngle(Vector3& out_axis) const;
	Quaternion			GetNormalized() const;
	Quaternion			GetConjugate() const;
	Quaternion			GetInverse() const;
	Vector3				GetAsEulerAngles() const;

	void				Normalize();
	void				ConvertToUnitNorm();
	
	Vector3				RotatePoint(const Vector3& point) const;

	// "CreateFromXAngles" are angle representations about the standard x, y, and z axes
	static Quaternion	CreateFromEulerAngles(float xDegrees, float yDegrees, float zDegrees);
	static Quaternion	CreateFromEulerAngles(const Vector3& eulerAnglesDegrees);
	static Quaternion	CreateFromRadianAngles(float xRadians, float yRadians, float zRadians);
	static Quaternion	CreateFromRadianAngles(const Vector3& radianAngles);
	static Quaternion	CreateFromAxisAndRadianAngle(const Vector3& axis, float radians);
	static Quaternion	CreateFromAxisAndDegreeAngle(const Vector3& axis, float degrees);

	static Quaternion	FromMatrix(const Matrix4& rotationMatrix);

	static Quaternion	RotateToward(const Quaternion& start, const Quaternion& end, float maxAngleDegrees);
	static float		GetAngleBetweenDegrees(const Quaternion& a, const Quaternion& b);
	static Quaternion	Lerp(const Quaternion& a, const Quaternion& b, float fractionTowardEnd);
	static Quaternion	Slerp(const Quaternion& start, const Quaternion& end, float fractionTowardEnd);


public:
	//-----Public Data-----

	Vector3 v;
	float real;

	// Statics
	static const Quaternion IDENTITY;

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
