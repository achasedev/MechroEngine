///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: May 1st, 2021
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
class Quaternion;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma warning(disable : 4201) // Keep the structs anonymous

//-------------------------------------------------------------------------------------------------
class Matrix3
{
public:
	//-----Public Methods-----

	Matrix3();
	explicit Matrix3(float* nineBasisMajorValues);
	explicit Matrix3(const Vector3& iBasis, const Vector3& jBasis, const Vector3& kBasis);
	explicit Matrix3(const Quaternion& quaternion);
	Matrix3(const Matrix3& other);
	
	void operator=(const Matrix3& other);
	void operator*=(const float scalar);
	void operator+=(const Matrix3& other);
	void operator*=(const Matrix3& other);
	bool operator==(const Matrix3& other) const;
	Matrix3 operator*(const float scalar) const;
	Matrix3 operator*(const Matrix3& other) const;
	Vector3 operator*(const Vector3& v) const;

	void	Transpose();
	void	Invert();
	void	SetFromQuaternion(const Quaternion& quaternion);
	void	SetAsSkewSymmetric(const Vector3& lhsCrossVector);

	Matrix3 GetTranspose() const;
	Matrix3 GetInverse() const;
	float	GetDeterminant() const;
	Vector3 GetXVector() const;
	Vector3 GetYVector() const;
	Vector3 GetZVector() const;

	static Matrix3 MakeRotationFromEulerAnglesDegrees(const Vector3& anglesDegrees);
	static Matrix3 MakeRotationFromEulerAnglesRadians(const Vector3& anglesRadians);
	static Vector3 ExtractRotationAsEulerAnglesDegrees(const Matrix3& matrix);
	static Vector3 ExtractRotationAsEulerAnglesRadians(const Matrix3& matrix);
	static Matrix3 MakeRotation(const Quaternion& quat);


public:
	//-----Public Data-----

	union
	{
		struct
		{
			float Ix;
			float Iy;
			float Iz;
			float Jx;
			float Jy;
			float Jz;
			float Kx;
			float Ky;
			float Kz;
		};

		struct  
		{
			Vector3 iBasis;
			Vector3 jBasis;
			Vector3 kBasis;
		};

		Vector3 columnVectors[3];

		float data[9];
	};

	static const Matrix3 IDENTITY;
	static const Matrix3 ZERO;
};

#pragma warning(default : 4201)
///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
