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
const Matrix3 Matrix3::ZERO = Matrix3(Vector3::ZERO, Vector3::ZERO, Vector3::ZERO);


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
void Matrix3::operator+=(const Matrix3& other)
{
	for (int i = 0; i < 9; ++i)
	{
		data[i] += other.data[i];
	}
}


//-------------------------------------------------------------------------------------------------
bool Matrix3::operator==(const Matrix3& other) const
{
	return (iBasis == other.iBasis) && (jBasis == other.jBasis) && (kBasis == other.kBasis);
}


//-------------------------------------------------------------------------------------------------
void Matrix3::operator*=(const float scalar)
{
	for (int i = 0; i < 9; ++i)
	{
		data[i] *= scalar;
	}
}


//-------------------------------------------------------------------------------------------------
void Matrix3::operator*=(const Matrix3& other)
{
	*this = ((*this) * other);
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
Vector3 Matrix3::operator*(const Vector3& v) const
{
	Vector3 result;

	result.x = DotProduct(GetXVector(), v);
	result.y = DotProduct(GetYVector(), v);
	result.z = DotProduct(GetZVector(), v);

	return result;
}


//-------------------------------------------------------------------------------------------------
Matrix3 Matrix3::operator*(const float scalar) const
{
	Matrix3 result = *this;

	for (int i = 0; i < 9; ++i)
	{
		result.data[i] *= scalar;
	}

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
	//ASSERT_RETURN(!AreMostlyEqual(det, 0.f), NO_RETURN_VAL, "Cannot invert, 0.f determinant!");
	
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
void Matrix3::SetFromQuaternion(const Quaternion& quat)
{
	Quaternion q = quat.GetNormalized();

	Ix = 1.f - 2.f * (q.y * q.y + q.z * q.z);
	Iy = 2.f * (q.x * q.y + q.z * q.w);
	Iz = 2.f * (q.x * q.z - q.y * q.w);

	Jx = 2.f * (q.x * q.y - q.z * q.w);
	Jy = 1.f - 2.f * (q.x * q.x + q.z * q.z);
	Jz = 2.f * (q.y * q.z + q.x * q.w);

	Kx = 2.f * (q.x * q.z + q.y * q.w);
	Ky = 2.f * (q.y * q.z - q.x * q.w);
	Kz = 1.f - 2.f * (q.x * q.x + q.y * q.y);
}


//-------------------------------------------------------------------------------------------------
// Makes a matrix s.t. thisMatrix * vector == CrossProduct(lhsCrossVector, vector)
void Matrix3::SetAsSkewSymmetric(const Vector3& lhsCrossVector)
{
	Ix = 0.f;
	Iy = lhsCrossVector.z;
	Iz = -lhsCrossVector.y;

	Jx = -lhsCrossVector.z;
	Jy = 0.f;
	Jz = lhsCrossVector.x;

	Kx = lhsCrossVector.y;
	Ky = -lhsCrossVector.x;
	Kz = 0.f;
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


//-------------------------------------------------------------------------------------------------
Matrix3 Matrix3::MakeRotationFromEulerAnglesDegrees(const Vector3& anglesDegrees)
{
	return MakeRotationFromEulerAnglesRadians(DegreesToRadians(anglesDegrees));
}


//-------------------------------------------------------------------------------------------------
Matrix3 Matrix3::MakeRotationFromEulerAnglesRadians(const Vector3& anglesRadians)
{
	float cosx = cosf(anglesRadians.x);
	float sinx = sinf(anglesRadians.x);
	float cosy = cosf(anglesRadians.y);
	float siny = sinf(anglesRadians.y);
	float cosz = cosf(anglesRadians.z);
	float sinz = sinf(anglesRadians.z);

	// Rotation about z
	Matrix3 rollMatrix;
	rollMatrix.Ix = cosz;
	rollMatrix.Iy = sinz;
	rollMatrix.Jx = -sinz;
	rollMatrix.Jy = cosz;

	// Rotation about y
	Matrix3 yawMatrix;
	yawMatrix.Ix = cosy;
	yawMatrix.Iz = -siny;
	yawMatrix.Kx = siny;
	yawMatrix.Kz = cosy;

	// Rotation about x
	Matrix3 pitchMatrix;
	pitchMatrix.Jy = cosx;
	pitchMatrix.Jz = sinx;
	pitchMatrix.Ky = -sinx;
	pitchMatrix.Kz = cosx;

	// Concatenate and return
	return yawMatrix * pitchMatrix * rollMatrix;
}


//-------------------------------------------------------------------------------------------------
Vector3 Matrix3::ExtractRotationAsEulerAnglesDegrees(const Matrix3& matrix)
{
	return RadiansToDegrees(ExtractRotationAsEulerAnglesRadians(matrix));
}


//-------------------------------------------------------------------------------------------------
Vector3 Matrix3::ExtractRotationAsEulerAnglesRadians(const Matrix3& matrix)
{
	float xRadians;
	float yRadians;
	float zRadians;

	float iScalar = (1.f / matrix.iBasis.GetLength());
	float jScalar = (1.f / matrix.jBasis.GetLength());
	float kScalar = (1.f / matrix.kBasis.GetLength());

	float sineX = -1.0f * kScalar * matrix.Ky;
	xRadians = asinf(sineX);

	float cosX = cosf(xRadians);
	if (!AreMostlyEqual(cosX, 0.f))
	{
		yRadians = atan2f(kScalar * matrix.Kx, kScalar * matrix.Kz);
		zRadians = atan2f(iScalar * matrix.Iy, jScalar * matrix.Jy);
	}
	else
	{
		// Gimble lock, lose roll but keep yaw
		zRadians = 0.f;
		yRadians = atan2f(-iScalar * matrix.Iz, iScalar * matrix.Ix);
	}

	return Vector3(xRadians, yRadians, zRadians);
}


//-------------------------------------------------------------------------------------------------
// Makes a rotation matrix from the given quaternion
Matrix3 Matrix3::MakeRotation(const Quaternion& quat)
{
	Matrix3 matrix;
	matrix.SetFromQuaternion(quat);

	return matrix;
}
