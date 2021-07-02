///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: December 8th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/Matrix3.h"
#include "Engine/Math/Matrix4.h"
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
const Matrix4 Matrix4::IDENTITY = Matrix4(Vector3::X_AXIS, Vector3::Y_AXIS, Vector3::Z_AXIS, Vector3::ZERO);

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
Matrix4::Matrix4()
{
	*this = IDENTITY;
}


//-------------------------------------------------------------------------------------------------
Matrix4::Matrix4(const float* sixteenValuesBasisMajor)
{
	ASSERT_OR_DIE(sixteenValuesBasisMajor != nullptr, "Matrix received null values!");

	for (int i = 0; i < 16; ++i)
	{
		data[i] = sixteenValuesBasisMajor[i];
	}
}


//-------------------------------------------------------------------------------------------------
Matrix4::Matrix4(const Vector3& iBasis, const Vector3& jBasis, const Vector3& kBasis, const Vector3& translation/*=Vector3::ZERO*/)
	: Matrix4()
{
	Ix = iBasis.x;
	Iy = iBasis.y;
	Iz = iBasis.z;
	Iw = 0.f;

	Jx = jBasis.x;
	Jy = jBasis.y;
	Jz = jBasis.z;
	Jw = 0.f;

	Kx = kBasis.x;
	Ky = kBasis.y;
	Kz = kBasis.z;
	Kw = 0.f;

	Tx = translation.x;
	Ty = translation.y;
	Tz = translation.z;
	Tw = 1.0f;
}


//-------------------------------------------------------------------------------------------------
Matrix4::Matrix4(const Vector4& _iBasis, const Vector4& _jBasis, const Vector4& _kBasis, const Vector4& _translation/*=Vector3::ZERO*/)
{
	iBasis = _iBasis;
	jBasis = _jBasis;
	kBasis = _kBasis;
	translation = _translation;
}


//-------------------------------------------------------------------------------------------------
Matrix4::Matrix4(const Matrix4& other)
{
	*this = other;
}


//-------------------------------------------------------------------------------------------------
Matrix4::Matrix4(const Matrix3& mat3)
{
	Ix = mat3.Ix;
	Iy = mat3.Iy;
	Iz = mat3.Iz;
	Iw = 0.f;

	Jx = mat3.Jx;
	Jy = mat3.Jy;
	Jz = mat3.Jz;
	Jw = 0.f;

	Kx = mat3.Kx;
	Ky = mat3.Ky;
	Kz = mat3.Kz;
	Kw = 0.f;

	translation = Vector4(0.f, 0.f, 0.f, 1.0f);
}


//-------------------------------------------------------------------------------------------------
const Matrix4 Matrix4::operator*(const Matrix4& rightMat) const
{
	Matrix4 result = *this;
	result.Append(rightMat);

	return result;
}


//-------------------------------------------------------------------------------------------------
const Vector4 Matrix4::operator*(const Vector4& rhsVector) const
{
	Vector4 result;

	result.x = DotProduct(GetXVector(), rhsVector);
	result.y = DotProduct(GetYVector(), rhsVector);
	result.z = DotProduct(GetZVector(), rhsVector);
	result.w = DotProduct(GetWVector(), rhsVector);

	return result;
}


//-------------------------------------------------------------------------------------------------
const Matrix4 Matrix4::operator*(float scaler) const
{
	Matrix4 result = *this;

	result.Ix *= scaler;
	result.Iy *= scaler;
	result.Iz *= scaler;
	result.Iw *= scaler;

	result.Jx *= scaler;
	result.Jy *= scaler;
	result.Jz *= scaler;
	result.Jw *= scaler;

	result.Kx *= scaler;
	result.Ky *= scaler;
	result.Kz *= scaler;
	result.Kw *= scaler;

	result.Tx *= scaler;
	result.Ty *= scaler;
	result.Tz *= scaler;
	result.Tw *= scaler;

	return result;
}


//-------------------------------------------------------------------------------------------------
bool Matrix4::operator==(const Matrix4& other) const
{
	if (GetIVector() != other.GetIVector())
	{
		return false;
	}

	if (GetJVector() != other.GetJVector())
	{
		return false;
	}

	if (GetKVector() != other.GetKVector())
	{
		return false;
	}

	if (GetTVector() != other.GetTVector())
	{
		return false;
	}

	return true;
}


//-------------------------------------------------------------------------------------------------
void Matrix4::operator=(const Matrix4& other)
{
	Ix = other.Ix;
	Iy = other.Iy;
	Iz = other.Iz;
	Iw = other.Iw;

	Jx = other.Jx;
	Jy = other.Jy;
	Jz = other.Jz;
	Jw = other.Jw;

	Kx = other.Kx;
	Ky = other.Ky;
	Kz = other.Kz;
	Kw = other.Kw;

	Tx = other.Tx;
	Ty = other.Ty;
	Tz = other.Tz;
	Tw = other.Tw;
}


//-------------------------------------------------------------------------------------------------
void Matrix4::SetIdentity()
{
	Ix = 1.0f;
	Iy = 0.f;
	Iz = 0.f;
	Iw = 0.f;

	Jx = 0.f;
	Jy = 1.f;
	Jz = 0.f;
	Jw = 0.f;

	Kx = 0.f;
	Ky = 0.f;
	Kz = 1.f;
	Kw = 0.f;

	Tx = 0.f;
	Ty = 0.f;
	Tz = 0.f;
	Tw = 1.f;
}


//-------------------------------------------------------------------------------------------------
void Matrix4::SetValues(const float* sixteenValuesBasisMajor)
{
	Ix = sixteenValuesBasisMajor[0];
	Iy = sixteenValuesBasisMajor[1];
	Iz = sixteenValuesBasisMajor[2];
	Iw = sixteenValuesBasisMajor[3];

	Jx = sixteenValuesBasisMajor[4];
	Jy = sixteenValuesBasisMajor[5];
	Jz = sixteenValuesBasisMajor[6];
	Jw = sixteenValuesBasisMajor[7];

	Kx = sixteenValuesBasisMajor[8];
	Ky = sixteenValuesBasisMajor[9];
	Kz = sixteenValuesBasisMajor[10];
	Kw = sixteenValuesBasisMajor[11];

	Tx = sixteenValuesBasisMajor[12];
	Ty = sixteenValuesBasisMajor[13];
	Tz = sixteenValuesBasisMajor[14];
	Tw = sixteenValuesBasisMajor[15];
}


//-------------------------------------------------------------------------------------------------
// Appends/concatenates the provided matrix on the RIGHT of the current matrix
// i.e. thisMatrix = thisMatrix * matrixToAppend;
//
void Matrix4::Append(const Matrix4& matrixToAppend)
{
	// Copy old values for calculation
	Matrix4 oldValues = *this;

	// New I basis vector
	Ix = DotProduct(oldValues.GetXVector(), matrixToAppend.GetIVector());
	Iy = DotProduct(oldValues.GetYVector(), matrixToAppend.GetIVector());
	Iz = DotProduct(oldValues.GetZVector(), matrixToAppend.GetIVector());
	Iw = DotProduct(oldValues.GetWVector(), matrixToAppend.GetIVector());

	// New J basis vector
	Jx = DotProduct(oldValues.GetXVector(), matrixToAppend.GetJVector());
	Jy = DotProduct(oldValues.GetYVector(), matrixToAppend.GetJVector());
	Jz = DotProduct(oldValues.GetZVector(), matrixToAppend.GetJVector());
	Jw = DotProduct(oldValues.GetWVector(), matrixToAppend.GetJVector());

	// New K basis vector
	Kx = DotProduct(oldValues.GetXVector(), matrixToAppend.GetKVector());
	Ky = DotProduct(oldValues.GetYVector(), matrixToAppend.GetKVector());
	Kz = DotProduct(oldValues.GetZVector(), matrixToAppend.GetKVector());
	Kw = DotProduct(oldValues.GetWVector(), matrixToAppend.GetKVector());

	// New T basis vector
	Tx = DotProduct(oldValues.GetXVector(), matrixToAppend.GetTVector());
	Ty = DotProduct(oldValues.GetYVector(), matrixToAppend.GetTVector());
	Tz = DotProduct(oldValues.GetZVector(), matrixToAppend.GetTVector());
	Tw = DotProduct(oldValues.GetWVector(), matrixToAppend.GetTVector());
}


//-------------------------------------------------------------------------------------------------
void Matrix4::Transpose()
{
	Matrix4 original = *this;

	Iy = original.Jx;
	Jx = original.Iy;

	Iz = original.Kx;
	Kx = original.Iz;

	Iw = original.Tx;
	Tx = original.Iw;

	Jz = original.Ky;
	Ky = original.Jz;

	Jw = original.Ty;
	Ty = original.Jw;

	Kw = original.Tz;
	Tz = original.Kw;
}


//-------------------------------------------------------------------------------------------------
void Matrix4::Invert()
{
	double inv[16];
	double det;
	double m[16];

	m[0] = static_cast<double>(Ix);
	m[1] = static_cast<double>(Iy);
	m[2] = static_cast<double>(Iz);
	m[3] = static_cast<double>(Iw);
	m[4] = static_cast<double>(Jx);
	m[5] = static_cast<double>(Jy);
	m[6] = static_cast<double>(Jz);
	m[7] = static_cast<double>(Jw);
	m[8] = static_cast<double>(Kx);
	m[9] = static_cast<double>(Ky);
	m[10] = static_cast<double>(Kz);
	m[11] = static_cast<double>(Kw);
	m[12] = static_cast<double>(Tx);
	m[13] = static_cast<double>(Ty);
	m[14] = static_cast<double>(Tz);
	m[15] = static_cast<double>(Tw);

	inv[0] = m[5] * m[10] * m[15] -
		m[5] * m[11] * m[14] -
		m[9] * m[6] * m[15] +
		m[9] * m[7] * m[14] +
		m[13] * m[6] * m[11] -
		m[13] * m[7] * m[10];

	inv[4] = -m[4] * m[10] * m[15] +
		m[4] * m[11] * m[14] +
		m[8] * m[6] * m[15] -
		m[8] * m[7] * m[14] -
		m[12] * m[6] * m[11] +
		m[12] * m[7] * m[10];

	inv[8] = m[4] * m[9] * m[15] -
		m[4] * m[11] * m[13] -
		m[8] * m[5] * m[15] +
		m[8] * m[7] * m[13] +
		m[12] * m[5] * m[11] -
		m[12] * m[7] * m[9];

	inv[12] = -m[4] * m[9] * m[14] +
		m[4] * m[10] * m[13] +
		m[8] * m[5] * m[14] -
		m[8] * m[6] * m[13] -
		m[12] * m[5] * m[10] +
		m[12] * m[6] * m[9];

	inv[1] = -m[1] * m[10] * m[15] +
		m[1] * m[11] * m[14] +
		m[9] * m[2] * m[15] -
		m[9] * m[3] * m[14] -
		m[13] * m[2] * m[11] +
		m[13] * m[3] * m[10];

	inv[5] = m[0] * m[10] * m[15] -
		m[0] * m[11] * m[14] -
		m[8] * m[2] * m[15] +
		m[8] * m[3] * m[14] +
		m[12] * m[2] * m[11] -
		m[12] * m[3] * m[10];

	inv[9] = -m[0] * m[9] * m[15] +
		m[0] * m[11] * m[13] +
		m[8] * m[1] * m[15] -
		m[8] * m[3] * m[13] -
		m[12] * m[1] * m[11] +
		m[12] * m[3] * m[9];

	inv[13] = m[0] * m[9] * m[14] -
		m[0] * m[10] * m[13] -
		m[8] * m[1] * m[14] +
		m[8] * m[2] * m[13] +
		m[12] * m[1] * m[10] -
		m[12] * m[2] * m[9];

	inv[2] = m[1] * m[6] * m[15] -
		m[1] * m[7] * m[14] -
		m[5] * m[2] * m[15] +
		m[5] * m[3] * m[14] +
		m[13] * m[2] * m[7] -
		m[13] * m[3] * m[6];

	inv[6] = -m[0] * m[6] * m[15] +
		m[0] * m[7] * m[14] +
		m[4] * m[2] * m[15] -
		m[4] * m[3] * m[14] -
		m[12] * m[2] * m[7] +
		m[12] * m[3] * m[6];

	inv[10] = m[0] * m[5] * m[15] -
		m[0] * m[7] * m[13] -
		m[4] * m[1] * m[15] +
		m[4] * m[3] * m[13] +
		m[12] * m[1] * m[7] -
		m[12] * m[3] * m[5];

	inv[14] = -m[0] * m[5] * m[14] +
		m[0] * m[6] * m[13] +
		m[4] * m[1] * m[14] -
		m[4] * m[2] * m[13] -
		m[12] * m[1] * m[6] +
		m[12] * m[2] * m[5];

	inv[3] = -m[1] * m[6] * m[11] +
		m[1] * m[7] * m[10] +
		m[5] * m[2] * m[11] -
		m[5] * m[3] * m[10] -
		m[9] * m[2] * m[7] +
		m[9] * m[3] * m[6];

	inv[7] = m[0] * m[6] * m[11] -
		m[0] * m[7] * m[10] -
		m[4] * m[2] * m[11] +
		m[4] * m[3] * m[10] +
		m[8] * m[2] * m[7] -
		m[8] * m[3] * m[6];

	inv[11] = -m[0] * m[5] * m[11] +
		m[0] * m[7] * m[9] +
		m[4] * m[1] * m[11] -
		m[4] * m[3] * m[9] -
		m[8] * m[1] * m[7] +
		m[8] * m[3] * m[5];

	inv[15] = m[0] * m[5] * m[10] -
		m[0] * m[6] * m[9] -
		m[4] * m[1] * m[10] +
		m[4] * m[2] * m[9] +
		m[8] * m[1] * m[6] -
		m[8] * m[2] * m[5];

	det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
	ASSERT_RETURN(det != 0.f, NO_RETURN_VAL, "Cannot invert, 0.f determinant!");
	det = 1.0 / det;

	Ix = static_cast<float>(inv[0] * det);
	Iy = static_cast<float>(inv[1] * det);
	Iz = static_cast<float>(inv[2] * det);
	Iw = static_cast<float>(inv[3] * det);
	Jx = static_cast<float>(inv[4] * det);
	Jy = static_cast<float>(inv[5] * det);
	Jz = static_cast<float>(inv[6] * det);
	Jw = static_cast<float>(inv[7] * det);
	Kx = static_cast<float>(inv[8] * det);
	Ky = static_cast<float>(inv[9] * det);
	Kz = static_cast<float>(inv[10] * det);
	Kw = static_cast<float>(inv[11] * det);
	Tx = static_cast<float>(inv[12] * det);
	Ty = static_cast<float>(inv[13] * det);
	Tz = static_cast<float>(inv[14] * det);
	Tw = static_cast<float>(inv[15] * det);
}


//-------------------------------------------------------------------------------------------------
void Matrix4::FastInverse()
{
	// Remove the translation
	Vector3 invTranslation = -1.0f * translation.xyz();
	translation = Vector4(0.f, 0.f, 0.f, 1.f);

	// What remains is a rotation (+ scale), so the inverse is the transpose
	Transpose();

	// Set the translation to be its inverse
	translation = Vector4(invTranslation, 1.0f);
}


//-------------------------------------------------------------------------------------------------
Vector4 Matrix4::GetIVector() const
{
	return Vector4(Ix, Iy, Iz, Iw);
}


//-------------------------------------------------------------------------------------------------
Vector4 Matrix4::GetJVector() const
{
	return Vector4(Jx, Jy, Jz, Jw);
}


//-------------------------------------------------------------------------------------------------
Vector4 Matrix4::GetKVector() const
{
	return Vector4(Kx, Ky, Kz, Kw);
}


//-------------------------------------------------------------------------------------------------
Vector4 Matrix4::GetTVector() const
{
	return Vector4(Tx, Ty, Tz, Tw);
}


//-------------------------------------------------------------------------------------------------
Vector4 Matrix4::GetXVector() const
{
	return Vector4(Ix, Jx, Kx, Tx);
}


//-------------------------------------------------------------------------------------------------
Vector4 Matrix4::GetYVector() const
{
	return Vector4(Iy, Jy, Ky, Ty);
}


//-------------------------------------------------------------------------------------------------
Vector4 Matrix4::GetZVector() const
{
	return Vector4(Iz, Jz, Kz, Tz);
}


//-------------------------------------------------------------------------------------------------
Vector4 Matrix4::GetWVector() const
{
	return Vector4(Iw, Jw, Kw, Tw);
}


//-------------------------------------------------------------------------------------------------
Matrix4 Matrix4::MakeTranslation(const Vector3& translation)
{
	Matrix4 translationMatrix;

	translationMatrix.Tx = translation.x;
	translationMatrix.Ty = translation.y;
	translationMatrix.Tz = translation.z;

	return translationMatrix;
}


//-------------------------------------------------------------------------------------------------
Matrix4 Matrix4::MakeScale(const Vector3& scale)
{
	Matrix4 scaleMatrix;

	scaleMatrix.Ix = scale.x;
	scaleMatrix.Jy = scale.y;
	scaleMatrix.Kz = scale.z;

	return scaleMatrix;
}


//-------------------------------------------------------------------------------------------------
Matrix4 Matrix4::MakeScaleUniform(float uniformScale)
{
	return MakeScale(Vector3(uniformScale));
}


//-------------------------------------------------------------------------------------------------
Matrix4 Matrix4::MakeModelMatrix(const Vector3& translation, const Vector3& rotation, const Vector3& scale)
{
	Matrix4 translationMatrix = MakeTranslation(translation);
	Matrix4 rotationMatrix = MakeRotationFromEulerAnglesDegrees(rotation);
	Matrix4 scaleMatrix = MakeScale(scale);

	Matrix4 result = translationMatrix * rotationMatrix * scaleMatrix;

	return result;
}


//-----------------------------------------------------------------------------------------------
// Constructs a matrix that transforms points from orthographic space (within the bounds specified)
// into clips space (bounds(-1, -1) to (1, 1) with center at (0, 0))
//
Matrix4 Matrix4::MakeOrtho(float leftX, float rightX, float bottomY, float topY, float nearZ, float farZ)
{
	Matrix4 orthoMatrix;

	orthoMatrix.Ix = 2.f / (rightX - leftX);
	orthoMatrix.Jy = 2.f / (topY - bottomY);
	orthoMatrix.Kz = 2.f / (farZ - nearZ);

	orthoMatrix.Tx = -(rightX + leftX) / (rightX - leftX);
	orthoMatrix.Ty = -(topY + bottomY) / (topY - bottomY);
	orthoMatrix.Tz = -(farZ + nearZ) / (farZ - nearZ);

	return orthoMatrix;
}


//-------------------------------------------------------------------------------------------------
Matrix4 Matrix4::MakeOrtho(const Vector2& bottomLeft, const Vector2& topRight, float nearZ/*=0.f*/, float farZ/*=1.0f*/)
{
	return MakeOrtho(bottomLeft.x, topRight.x, bottomLeft.y, topRight.y, nearZ, farZ);
}


//-------------------------------------------------------------------------------------------------
Matrix4 Matrix4::MakePerspective(float fovDegrees, float aspect, float nearZ, float farZ)
{
	float d = (1.f / TanDegrees(0.5f * fovDegrees));

	Matrix4 perspective;

	perspective.Ix = (d / aspect);
	perspective.Jy = d;
	perspective.Kz = (farZ + nearZ) / (farZ - nearZ);
	perspective.Tz = (-2.f * nearZ * farZ) / (farZ - nearZ);

	perspective.Kw = 1.f;
	perspective.Tw = 0.f;

	return perspective;
}


//-------------------------------------------------------------------------------------------------
Vector3 Matrix4::ExtractTranslation(const Matrix4& translationMatrix)
{
	Vector3 translation;

	translation.x = translationMatrix.Tx;
	translation.y = translationMatrix.Ty;
	translation.z = translationMatrix.Tz;

	return translation;
}


//-------------------------------------------------------------------------------------------------
Vector3 Matrix4::ExtractScale(const Matrix4& scaleMatrix)
{
	// TODO: Check signs of cross product to flip correct axes for negative scales
	float xScale = scaleMatrix.GetIVector().GetLength();
	float yScale = scaleMatrix.GetJVector().GetLength();
	float zScale = scaleMatrix.GetKVector().GetLength();

	return Vector3(xScale, yScale, zScale);
}


//-------------------------------------------------------------------------------------------------
Matrix4 Matrix4::GetInverse() const
{	
	Matrix4 inverse = *this;
	inverse.Invert();
	return inverse;
}


//-------------------------------------------------------------------------------------------------
Matrix4 Matrix4::GetInverse(const Matrix4& matrix)
{
	return matrix.GetInverse();
}


//-------------------------------------------------------------------------------------------------
Matrix3 Matrix4::GetMatrix3Part() const
{
	Matrix3 matrix;

	matrix.Ix = Ix;
	matrix.Iy = Iy;
	matrix.Iz = Iz;

	matrix.Jx = Jx;
	matrix.Jy = Jy;
	matrix.Jz = Jz;

	matrix.Kx = Kx;
	matrix.Ky = Ky;
	matrix.Kz = Kz;

	return matrix;
}


//-------------------------------------------------------------------------------------------------
float Matrix4::GetDeterminant() const
{
	float t1 = data[8] * data[5] * data[2];
	float t2 = data[4] * data[9] * data[2];
	float t3 = data[8] * data[1] * data[6];
	float t4 = data[0] * data[9] * data[6];
	float t5 = data[4] * data[1] * data[10];
	float t6 = data[0] * data[5] * data[10];

	return t1 + t2 + t3 - t4 - t5 + t6;
}


//-------------------------------------------------------------------------------------------------
Vector3 Matrix4::TransformPosition(const Vector3& position) const
{
	return ((*this) * Vector4(position, 1.0f)).xyz();
}


Vector3 Matrix4::TransformDirection(const Vector3& direction) const
{
	return ((*this) * Vector4(direction, 0.f)).xyz();
}


//-------------------------------------------------------------------------------------------------
Matrix4 Matrix4::MakeRotationFromEulerAnglesDegrees(const Vector3& anglesDegrees)
{
	return Matrix4(Matrix3::MakeRotationFromEulerAnglesDegrees(anglesDegrees));
}


//-------------------------------------------------------------------------------------------------
Matrix4 Matrix4::MakeRotationFromEulerAnglesRadians(const Vector3& anglesRadians)
{
	return Matrix4(Matrix3::MakeRotationFromEulerAnglesRadians(anglesRadians));
}


//-------------------------------------------------------------------------------------------------
Matrix4 Matrix4::MakeRotation(const Quaternion& rotation)
{
	return Matrix4(Matrix3(rotation));
}


//-------------------------------------------------------------------------------------------------
Vector3 Matrix4::ExtractRotationAsEulerAnglesDegrees(const Matrix4& rotationMatrix)
{
	return Matrix3::ExtractRotationAsEulerAnglesDegrees(rotationMatrix.GetMatrix3Part());
}


//-------------------------------------------------------------------------------------------------
Vector3 Matrix4::ExtractRotationAsEulerAnglesRadians(const Matrix4& rotationMatrix)
{
	return Matrix3::ExtractRotationAsEulerAnglesRadians(rotationMatrix.GetMatrix3Part());
}


//-------------------------------------------------------------------------------------------------
Matrix4 Matrix4::MakeLookAt(const Vector3& position, const Vector3& target, const Vector3& referenceUp /*= Vector3::Y_AXIS*/)
{
	// Edge case - Target and position are the same position, then just look world forward
	Vector3 forward;
	if (position == target)
	{
		forward = Vector3::Z_AXIS;
	}
	else
	{
		forward = (target - position).GetNormalized();
	}

	// Edge case - check if the forward happens to be the reference up vector, and if so just set right to the reference up
	ASSERT_OR_DIE(forward != referenceUp, "Error: Matrix44::LookAt() had forward and up vector matched.");

	Vector3 right = CrossProduct(referenceUp, forward);
	right.Normalize();

	Vector3 lookUp = CrossProduct(forward, right);

	return Matrix4(right, lookUp, forward, position);
}
