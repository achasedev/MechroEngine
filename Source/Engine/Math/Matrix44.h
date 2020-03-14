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

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class Matrix44
{
public:
	//-----Public Methods-----

	Matrix44();
	explicit Matrix44(const float* sixteenValuesBasisMajor); // float[16] array in order Ix, Iy...
	explicit Matrix44(const Vector3& iBasis, const Vector3& jBasis, const Vector3& kBasis, const Vector3& translation = Vector3::ZERO);
	explicit Matrix44(const Vector4& iBasis, const Vector4& jBasis, const Vector4& kBasis, const Vector4& translation = Vector4::ZERO);

	const Matrix44	operator*(const Matrix44& rightMat) const;
	const Matrix44	operator*(float uniformScaler) const;
	const Vector4	operator*(const Vector4& rightVector) const;
	bool			operator==(const Matrix44& other) const;

	Vector4 TransformPoint(const Vector2& point) const;
	Vector4 TransformPoint(const Vector3& point) const;
	Vector4 TransformVector(const Vector2& vector) const;
	Vector4 TransformVector(const Vector3& vector) const;
	Vector4 Transform(const Vector4& vector) const;

	void SetIdentity();
	void SetValues(const float* sixteenValuesBasisMajor);		// float[16] array in order Ix, Iy...
	void Append(const Matrix44& matrixToAppend);				// Concatenate on the right	
	void Transpose();
	void Invert();

	// Accessors
	Vector4 GetIVector() const;
	Vector4 GetJVector() const;
	Vector4 GetKVector() const;
	Vector4 GetTVector() const;
	Vector4 GetXVector() const;
	Vector4 GetYVector() const;
	Vector4 GetZVector() const;
	Vector4 GetWVector() const;
	Matrix44 GetInverse() const;

	// Static producers
	static Matrix44 MakeTranslation(const Vector3& translation);
	static Matrix44 MakeRotation(const Vector3& rotation);
	static Matrix44 MakeScale(const Vector3& scale);
	static Matrix44 MakeScaleUniform(float uniformScale);
	static Matrix44 MakeModelMatrix(const Vector3& translation, const Vector3& rotation, const Vector3& scale);

	static Matrix44 MakeOrtho(float leftX, float rightX, float bottomY, float topY, float nearZ, float farZ);
	static Matrix44 MakeOrtho(const Vector2& bottomLeft, const Vector2& topRight, float nearZ = 0.f, float farZ = 1.0f);
	static Matrix44 MakePerspective(float fovDegrees, float aspect, float nearZ, float farZ);
	static Matrix44 MakeLookAt(const Vector3& position, const Vector3& target, const Vector3& up = Vector3::Y_AXIS);

	static Vector3 ExtractTranslation(const Matrix44& translationMatrix);
	static Vector3 ExtractRotationDegrees(const Matrix44& rotationMatrix);
	static Vector3 ExtractScale(const Matrix44& scaleMatrix);


public:
	//-----Public data----- 
	// 16 floats to represent the 4x4 Basis-major ordered matrix
	// Initialized to identity

	// I basis vector
	float Ix = 1.0f;
	float Iy = 0.f;
	float Iz = 0.f;
	float Iw = 0.f;

	// J basis vector
	float Jx = 0.f;
	float Jy = 1.f;
	float Jz = 0.f;
	float Jw = 0.f;

	// K basis vector
	float Kx = 0.f;
	float Ky = 0.f;
	float Kz = 1.f;
	float Kw = 0.f;

	// T (translation) vector
	float Tx = 0.f;
	float Ty = 0.f;
	float Tz = 0.f;
	float Tw = 1.f;

	const static Matrix44 IDENTITY;

};
