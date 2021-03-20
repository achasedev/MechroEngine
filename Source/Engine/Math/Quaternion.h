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
class Matrix44;

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
	Quaternion(float scalar, const Vector3& vector);
	Quaternion(float scalar, float x, float y, float z);
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
	Quaternion			GetNormalized() const;
	Quaternion			GetConjugate() const;
	Quaternion			GetInverse() const;
	Vector3				GetAsEulerAngles() const;

	void				Normalize();
	void				ConvertToUnitNorm();
	
	Vector3				RotatePoint(const Vector3& point) const;

	static float		GetAngleBetweenDegrees(const Quaternion& a, const Quaternion& b);
	static Quaternion	FromEuler(const Vector3& eulerAnglesDegrees);
	static Quaternion	FromMatrix(const Matrix44& rotationMatrix);
	static Quaternion	RotateToward(const Quaternion& start, const Quaternion& end, float maxAngleDegrees);

	static Quaternion	Lerp(const Quaternion& a, const Quaternion& b, float fractionTowardEnd);
	static Quaternion	Slerp(const Quaternion& start, const Quaternion& end, float fractionTowardEnd);


public:
	//-----Public Data-----

	Vector3 v;
	float s;

	// Statics
	static const Quaternion IDENTITY;

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
