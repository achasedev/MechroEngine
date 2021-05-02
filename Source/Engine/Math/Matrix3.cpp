///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: May 1st, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/Math/Matrix3.h"
#include "Engine/Math/MathUtils.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------
const Matrix3 Matrix3::IDENTITY = Matrix3(Vector3::X_AXIS, Vector3::Y_AXIS, Vector3::Z_AXIS);


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
Matrix3::Matrix3()
{
	*this = IDENTITY;
}


//-------------------------------------------------------------------------------------------------
Matrix3::Matrix3(float* nineBasisMajorValues)
{
	for (int i = 0; i < 9; ++i)
	{
		data[i] = nineBasisMajorValues[i];
	}
}


//-------------------------------------------------------------------------------------------------
Matrix3::Matrix3(const Vector3& _iBasis, const Vector3& _jBasis, const Vector3& _kBasis)
{
	iBasis = _iBasis;
	jBasis = _jBasis;
	kBasis = _kBasis;
}


//--------------------------------------------------------------------------------------------------
Matrix3::Matrix3(const Matrix3& other)
{
	*this = other;
}


//-------------------------------------------------------------------------------------------------
Matrix3::Matrix3(const Quaternion& quaternion)
{
	SetFromQuaternion(quaternion);
}


//-------------------------------------------------------------------------------------------------
bool Matrix3::operator==(const Matrix3& other) const
{
	return (iBasis == other.iBasis) && (jBasis == other.jBasis) && (kBasis == other.kBasis);
}


//-------------------------------------------------------------------------------------------------
void Matrix3::operator*=(const Matrix3& other)
{
	*this = (*this * other);
}


//-------------------------------------------------------------------------------------------------
Matrix3 Matrix3::operator*(const Matrix3& other) const
{
	Matrix3 result;

	result.Ix = DotProduct(GetXVector(), other.iBasis);
	result.Iy = DotProduct(GetYVector(), other.iBasis);
	result.Iz = DotProduct(GetZVector(), other.iBasis);

	result.Jx = DotProduct(GetXVector(), other.jBasis);
	result.Jy = DotProduct(GetYVector(), other.jBasis);
	result.Jz = DotProduct(GetZVector(), other.jBasis);

	result.Kx = DotProduct(GetXVector(), other.kBasis);
	result.Ky = DotProduct(GetYVector(), other.kBasis);
	result.Kz = DotProduct(GetZVector(), other.kBasis);
	
	return result;
}


//-------------------------------------------------------------------------------------------------
void Matrix3::Transpose()
{
	Matrix3 original = *this;

	Iy = original.Jx;
	Jx = original.Iy;

	Iz = original.Kx;
	Kx = original.Iz;

	Jz = original.Ky;
	Ky = original.Jz;
}


//-------------------------------------------------------------------------------------------------
void Matrix3::Invert()
{
	float t1 = data[0] * data[4];
	float t2 = data[0] * data[5];
	float t3 = data[1] * data[3];
	float t4 = data[2] * data[3];
	float t5 = data[1] * data[6];
	float t6 = data[2] * data[6];

	float det = t1 * data[8] - t2 * data[7] - t3 * data[8] + t4 * data[7] + t5 * data[5] - t6 * data[4];
	ASSERT_OR_DIE(det == GetDeterminant(), "Determinant calculations don't match!");
	ASSERT_RETURN(det != 0.f, NO_RETURN_VAL, "Cannot invert, 0.f determinant!");
	
	float invDet = 1.0f / det;

	Matrix3 old = *this;

	data[0] = invDet * (old.data[4] * old.data[8] - old.data[5] * old.data[7]);
	data[1] = invDet * (old.data[2] * old.data[7] - old.data[1] * old.data[8]);
	data[2] = invDet * (old.data[1] * old.data[5] - old.data[2] * old.data[4]);
	data[3] = invDet * (old.data[5] * old.data[6] - old.data[3] * old.data[8]);
	data[4] = invDet * (old.data[0] * old.data[8] - t6);
	data[5] = invDet * (t4 - t2);
	data[6] = invDet * (old.data[3] * old.data[7] - old.data[4] * old.data[6]);
	data[7] = invDet * (t5 - old.data[0] * old.data[7]);
	data[8] = invDet * (t1 - t3);
}


//-------------------------------------------------------------------------------------------------
void Matrix3::SetFromQuaternion(const Quaternion& q)
{
	Ix = 1.f - 2.f * (q.y * q.y + q.z * q.z);
	Iy = 2.f * (q.x * q.y - q.z * q.w);
	Iz = 2.f * (q.x * q.z + q.y * q.w);

	Jx = 2.f * (q.x * q.y + q.z * q.w);
	Jy = 1.f - 2.f * (q.x * q.x + q.z * q.z);
	Jz = 2.f * (q.y * q.z - q.x * q.w);

	Kx = 2.f * (q.x * q.z - q.y * q.w);
	Ky = 2.f * (q.y * q.z + q.x * q.w);
	Kz = 1.f - 2.f * (q.x * q.x + q.y * q.y);
}


//-------------------------------------------------------------------------------------------------
Matrix3 Matrix3::GetTranspose() const
{
	Matrix3 result = *this;
	result.Transpose();
	return result;
}


//-------------------------------------------------------------------------------------------------
Matrix3 Matrix3::GetInverse() const
{
	Matrix3 result = *this;
	result.Invert();
	return result;
}


//-------------------------------------------------------------------------------------------------
void Matrix3::operator=(const Matrix3& other)
{
	Ix = other.Ix;
	Iy = other.Iy;
	Iz = other.Iz;

	Jx = other.Jx;
	Jy = other.Jy;
	Jz = other.Jz;

	Kx = other.Kx;
	Ky = other.Ky;
	Kz = other.Kz;
}


//-------------------------------------------------------------------------------------------------
float Matrix3::GetDeterminant() const
{
	float p1 = Ix * Jy * Kz;
	float p2 = Iy * Jz * Kx;
	float p3 = Iz * Jx * Ky;
	float p4 = Ix * Jz * Ky;
	float p5 = Iz * Jy * Kx;
	float p6 = Iy * Jx * Kz;

	return p1 + p2 + p3 - p4 - p5 - p6;
}


//-------------------------------------------------------------------------------------------------
Vector3 Matrix3::GetXVector() const
{
	return Vector3(Ix, Jx, Kx);
}


//-------------------------------------------------------------------------------------------------
Vector3 Matrix3::GetYVector() const
{
	return Vector3(Iy, Jy, Ky);
}


//-------------------------------------------------------------------------------------------------
Vector3 Matrix3::GetZVector() const
{
	return Vector3(Iz, Jz, Kz);
}
