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
class ScaledAxisRotation;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma warning(disable : 4201) // Keep the structs anonymous

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
	Vector3				GetAsEulerAnglesDegrees() const;
	Vector3				GetAsEulerAnglesRadians() const;

	void				Normalize();
	void				ConvertToUnitNorm();
	
	Vector3				RotatePoint(const Vector3& point) const;

	// "CreateFromXAngles" are angle representations about the standard x, y, and z axes
	static Quaternion	CreateFromEulerAnglesDegrees(float xDegrees, float yDegrees, float zDegrees);
	static Quaternion	CreateFromEulerAnglesDegrees(const Vector3& eulerAnglesDegrees);
	static Quaternion	CreateFromEulerAnglesRadians(float xRadians, float yRadians, float zRadians);
	static Quaternion	CreateFromEulerAnglesRadians(const Vector3& radianAngles);
	static Quaternion	CreateFromAxisAndRadianAngle(const Vector3& axis, float radians);
	static Quaternion	CreateFromAxisAndDegreeAngle(const Vector3& axis, float degrees);
	static Quaternion	CreateFromScaledAxisDegrees(const ScaledAxisRotation& scaledAxisDegrees);
	static Quaternion	CreateFromScaledAxisRadians(const ScaledAxisRotation& scaledAxisRadians);

	static Quaternion	FromMatrix(const Matrix4& rotationMatrix);

	static Quaternion	RotateToward(const Quaternion& start, const Quaternion& end, float maxAngleDegrees);
	static float		GetAngleBetweenDegrees(const Quaternion& a, const Quaternion& b);
	static Quaternion	Lerp(const Quaternion& a, const Quaternion& b, float fractionTowardEnd);
	static Quaternion	Slerp(const Quaternion& start, const Quaternion& end, float fractionTowardEnd);


public:
	//-----Public Data-----

	union
	{
		struct  
		{
			float real;
			Vector3 v;
		};

		struct  
		{
			float w;
			float x;
			float y;
			float z;
		};

	};

	// Statics
	static const Quaternion IDENTITY;

};

#pragma warning(default : 4201)
///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
