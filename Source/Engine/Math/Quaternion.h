/////--------------------------------------------------------------------------------------------------------------------------------------------------
///// Author: Andrew Chase
///// Date Created: December 14th, 2019
///// Description: 
/////--------------------------------------------------------------------------------------------------------------------------------------------------
//#pragma once
//
/////--------------------------------------------------------------------------------------------------------------------------------------------------
///// INCLUDES
/////--------------------------------------------------------------------------------------------------------------------------------------------------
//#include "Engine/Math/Vector3.h"
//
/////--------------------------------------------------------------------------------------------------------------------------------------------------
///// DEFINES
/////--------------------------------------------------------------------------------------------------------------------------------------------------
//
/////--------------------------------------------------------------------------------------------------------------------------------------------------
///// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
/////--------------------------------------------------------------------------------------------------------------------------------------------------
//
/////--------------------------------------------------------------------------------------------------------------------------------------------------
///// GLOBALS AND STATICS
/////--------------------------------------------------------------------------------------------------------------------------------------------------
//
/////--------------------------------------------------------------------------------------------------------------------------------------------------
///// CLASS DECLARATIONS
/////--------------------------------------------------------------------------------------------------------------------------------------------------
//
/////--------------------------------------------------------------------------------------------------------------------------------------------------
///// C FUNCTIONS
/////--------------------------------------------------------------------------------------------------------------------------------------------------
//
////-------------------------------------------------------------------------------------------------
//class Quaternion
//{
//public:
//	//-----Public Methods-----
//
//	Quaternion();
//	Quaternion(const Vector3& axis, float angleRadians);
//	Quaternion(float x, float y, float z, float w);
//	Quaternion(const Quaternion& copy);
//	~Quaternion() {}
//
//	void operator=(const Quaternion& copy);
//	const Quaternion operator+(const Quaternion& other) const;
//	const Quaternion operator-(const Quaternion& other) const;
//	const Quaternion operator*(const Quaternion& other) const;
//
//	const Quaternion operator*(float scalar) const;
//	friend const Quaternion operator*(float scalar, const Quaternion& quat);
//
//	void operator+=(const Quaternion& other);
//	void operator-=(const Quaternion& other);
//	void operator*=(const Quaternion& other);
//	void operator*=(float scalar);
//
//	void		Normalize();
//
//	float		GetMagnitude() const;
//	Quaternion	GetNormalized() const;
//	Quaternion	GetConjugate() const;
//	Quaternion	GetInverse() const;
//	Vector3		GetAsEulerAngles() const;
//
//	static float		GetAngleBetweenDegrees(const Quaternion& a, const Quaternion& b);
//	static Quaternion	FromEuler(const Vector3& eulerAnglesDegrees);
//	static Quaternion	RotateToward(const Quaternion& start, const Quaternion& end, float maxAngleDegrees);
//	static Quaternion	Lerp(const Quaternion& a, const Quaternion& b, float fractionTowardEnd);
//	static Quaternion	Slerp(const Quaternion& start, const Quaternion& end, float fractionTowardEnd);
//
//
//public:
//	//-----Public Data-----
//
//	Vector3 v;
//	float s;
//
//	static const Quaternion IDENTITY;
//
//};
