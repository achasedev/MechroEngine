///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: December 8th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector4.h"
#include "Engine/Math/Quaternion.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Matrix3;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma warning(disable : 4201) // Keep the structs anonymous

//-------------------------------------------------------------------------------------------------
class Matrix4
{
public:
	//-----Public Methods-----

	Matrix4();
	explicit Matrix4(const float* sixteenValuesBasisMajor); // float[16] array in order Ix, Iy...
	explicit Matrix4(const Vector3& iBasis, const Vector3& jBasis, const Vector3& kBasis, const Vector3& translation = Vector3::ZERO);
	explicit Matrix4(const Vector4& iBasis, const Vector4& jBasis, const Vector4& kBasis, const Vector4& translation = Vector4::ZERO);
	explicit Matrix4(const Matrix3& mat3);
	explicit Matrix4(const Matrix3& mat3, const Vector3& translation);

	Matrix4(const Matrix4& other);

	const Matrix4	operator*(const Matrix4& rightMat) const;
	const Matrix4	operator*(float uniformScaler) const;
	const Vector4	operator*(const Vector4& rightVector) const;
	bool			operator==(const Matrix4& other) const;
	void			operator=(const Matrix4& other);

	void SetIdentity();
	void SetValues(const float* sixteenValuesBasisMajor);	// float[16] array in order Ix, Iy...
	void Append(const Matrix4& matrixToAppend);				// Concatenate on the right	
	void Transpose();
	void Invert();
	void FastInverse();

	// Accessors
	Vector4 GetIVector() const;
	Vector4 GetJVector() const;
	Vector4 GetKVector() const;
	Vector4 GetTVector() const;
	Vector4 GetXVector() const;
	Vector4 GetYVector() const;
	Vector4 GetZVector() const;
	Vector4 GetWVector() const;
	Matrix4 GetInverse() const;
	Matrix3 GetMatrix3Part() const;
	float	GetDeterminant() const;

	// Producers
	Vector3 TransformPosition(const Vector3& position) const;
	Vector3 TransformDirection(const Vector3& direction) const;

	// Static producers
	static Matrix4 MakeTranslation(const Vector3& translation);
	static Matrix4 MakeRotationFromEulerAnglesDegrees(const Vector3& anglesDegrees);
	static Matrix4 MakeRotationFromEulerAnglesRadians(const Vector3& anglesRadians);
	static Matrix4 MakeRotation(const Quaternion& rotation);
	static Matrix4 MakeScale(const Vector3& scale);
	static Matrix4 MakeScaleUniform(float uniformScale);
	static Matrix4 MakeModelMatrix(const Vector3& translation, const Vector3& eulerAngleDegrees, const Vector3& scale);
	static Matrix4 MakeModelMatrix(const Vector3& translation, const Quaternion& rotation, const Vector3& scale);

	static Matrix4 MakeOrtho(float leftX, float rightX, float bottomY, float topY, float nearZ, float farZ);
	static Matrix4 MakeOrtho(const Vector2& bottomLeft, const Vector2& topRight, float nearZ = 0.f, float farZ = 1.0f);
	static Matrix4 MakePerspective(float fovDegrees, float aspect, float nearZ, float farZ);
	static Matrix4 MakeLookAt(const Vector3& position, const Vector3& target, const Vector3& up = Vector3::Y_AXIS);
	static Matrix4 GetInverse(const Matrix4& matrix);

	static Vector3 ExtractTranslation(const Matrix4& translationMatrix);
	static Vector3 ExtractRotationAsEulerAnglesDegrees(const Matrix4& rotationMatrix);
	static Vector3 ExtractRotationAsEulerAnglesRadians(const Matrix4& rotationMatrix);
	static Vector3 ExtractScale(const Matrix4& scaleMatrix);


public:
	//-----Public data----- 
	// 16 floats to represent the 4x4 Basis-major ordered matrix
	// Initialized to identity

	union
	{
		struct  
		{
			// I basis vector
			float Ix;
			float Iy;
			float Iz;
			float Iw;

			// J basis vector
			float Jx;
			float Jy;
			float Jz;
			float Jw;

			// K basis vector
			float Kx;
			float Ky;
			float Kz;
			float Kw;

			// T (translation) vector
			float Tx;
			float Ty;
			float Tz;
			float Tw;
		};

		struct
		{
			Vector4 iBasis;
			Vector4 jBasis;
			Vector4 kBasis;
			Vector4 translation;
		};

		float data[16];
	};
	

	const static Matrix4 IDENTITY;
	const static Matrix4 ZERO;

};

#pragma warning(default : 4201)

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
