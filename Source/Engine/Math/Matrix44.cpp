///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: December 8th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/Matrix44.h"
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
const Matrix44 Matrix44::IDENTITY = Matrix44();

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
Matrix44::Matrix44()
{
}


//-------------------------------------------------------------------------------------------------
Matrix44::Matrix44(const float* sixteenValuesBasisMajor)
{
	ASSERT_OR_DIE(sixteenValuesBasisMajor != nullptr, "Matrix received null values!");

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
Matrix44::Matrix44(const Vector3& iBasis, const Vector3& jBasis, const Vector3& kBasis, const Vector3& translation/*=Vector3::ZERO*/)
	: Matrix44()
{
	Ix = iBasis.x;
	Iy = iBasis.y;
	Iz = iBasis.z;

	Jx = jBasis.x;
	Jy = jBasis.y;
	Jz = jBasis.z;

	Kx = kBasis.x;
	Ky = kBasis.y;
	Kz = kBasis.z;

	Tx = translation.x;
	Ty = translation.y;
	Tz = translation.z;
	Tw = 1.0f;
}


//-------------------------------------------------------------------------------------------------
Matrix44::Matrix44(const Vector4& iBasis, const Vector4& jBasis, const Vector4& kBasis, const Vector4& translation/*=Vector3::ZERO*/)
{
	Ix = iBasis.x;
	Iy = iBasis.y;
	Iz = iBasis.z;
	Iw = iBasis.w;

	Jx = jBasis.x;
	Jy = jBasis.y;
	Jz = jBasis.z;
	Jw = iBasis.w;

	Kx = kBasis.x;
	Ky = kBasis.y;
	Kz = kBasis.z;
	Kw = kBasis.w;

	Tx = translation.x;
	Ty = translation.y;
	Tz = translation.z;
	Tw = translation.w;
}


//-------------------------------------------------------------------------------------------------
const Matrix44 Matrix44::operator*(const Matrix44& rightMat) const
{
	Matrix44 result = *this;
	result.Append(rightMat);

	return result;
}


//-------------------------------------------------------------------------------------------------
const Vector4 Matrix44::operator*(const Vector4& rightVector) const
{
	return Transform(rightVector);
}


//-------------------------------------------------------------------------------------------------
const Matrix44 Matrix44::operator*(float scaler) const
{
	Matrix44 result = *this;

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
bool Matrix44::operator==(const Matrix44& other) const
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
Vector4 Matrix44::TransformPoint(const Vector2& point) const
{
	Vector4 pointToTransform = Vector4(point.x, point.y, 0.f, 1.0f);
	return Transform(pointToTransform);
}


//-------------------------------------------------------------------------------------------------
Vector4 Matrix44::TransformPoint(const Vector3& point) const
{
	Vector4 pointToTransform = Vector4(point.x, point.y, point.z, 1.0f);
	return Transform(pointToTransform);
}


//-------------------------------------------------------------------------------------------------
Vector4 Matrix44::TransformVector(const Vector2& vector) const
{
	Vector4 vectorToTransform = Vector4(vector.x, vector.y, 0.f, 0.0f);
	return Transform(vectorToTransform);
}


//-------------------------------------------------------------------------------------------------
Vector4 Matrix44::TransformVector(const Vector3& vector) const
{
	Vector4 vectorToTransform = Vector4(vector.x, vector.y, vector.z, 0.0f);
	return Transform(vectorToTransform);
}


//-------------------------------------------------------------------------------------------------
Vector4 Matrix44::Transform(const Vector4& vector) const
{
	Vector4 result;

	result.x = DotProduct(GetXVector(), vector);
	result.y = DotProduct(GetYVector(), vector);
	result.z = DotProduct(GetZVector(), vector);
	result.w = DotProduct(GetWVector(), vector);

	return result;
}


//-------------------------------------------------------------------------------------------------
void Matrix44::SetIdentity()
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
void Matrix44::SetValues(const float* sixteenValuesBasisMajor)
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
void Matrix44::Append(const Matrix44& matrixToAppend)
{
	// Copy old values for calculation
	Matrix44 oldValues = *this;

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
void Matrix44::Transpose()
{
	Matrix44 original = *this;

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
void Matrix44::Invert()
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
Vector4 Matrix44::GetIVector() const
{
	return Vector4(Ix, Iy, Iz, Iw);
}


//-------------------------------------------------------------------------------------------------
Vector4 Matrix44::GetJVector() const
{
	return Vector4(Jx, Jy, Jz, Jw);
}


//-------------------------------------------------------------------------------------------------
Vector4 Matrix44::GetKVector() const
{
	return Vector4(Kx, Ky, Kz, Kw);
}


//-------------------------------------------------------------------------------------------------
Vector4 Matrix44::GetTVector() const
{
	return Vector4(Tx, Ty, Tz, Tw);
}


//-------------------------------------------------------------------------------------------------
Vector4 Matrix44::GetXVector() const
{
	return Vector4(Ix, Jx, Kx, Tx);
}


//-------------------------------------------------------------------------------------------------
Vector4 Matrix44::GetYVector() const
{
	return Vector4(Iy, Jy, Ky, Ty);
}


//-------------------------------------------------------------------------------------------------
Vector4 Matrix44::GetZVector() const
{
	return Vector4(Iz, Jz, Kz, Tz);
}


//-------------------------------------------------------------------------------------------------
Vector4 Matrix44::GetWVector() const
{
	return Vector4(Iw, Jw, Kw, Tw);
}


//-------------------------------------------------------------------------------------------------
Matrix44 Matrix44::MakeTranslation(const Vector3& translation)
{
	Matrix44 translationMatrix;

	translationMatrix.Tx = translation.x;
	translationMatrix.Ty = translation.y;
	translationMatrix.Tz = translation.z;

	return translationMatrix;
}


//-------------------------------------------------------------------------------------------------
Matrix44 Matrix44::MakeScale(const Vector3& scale)
{
	Matrix44 scaleMatrix;

	scaleMatrix.Ix = scale.x;
	scaleMatrix.Jy = scale.y;
	scaleMatrix.Kz = scale.z;

	return scaleMatrix;
}


//-------------------------------------------------------------------------------------------------
Matrix44 Matrix44::MakeScaleUniform(float uniformScale)
{
	return MakeScale(Vector3(uniformScale));
}


//-------------------------------------------------------------------------------------------------
Matrix44 Matrix44::MakeModelMatrix(const Vector3& translation, const Vector3& rotation, const Vector3& scale)
{
	Matrix44 translationMatrix = MakeTranslation(translation);
	Matrix44 rotationMatrix = MakeRotation(rotation);
	Matrix44 scaleMatrix = MakeScale(scale);

	Matrix44 result = translationMatrix * rotationMatrix * scaleMatrix;

	return result;
}


//-----------------------------------------------------------------------------------------------
// Constructs a matrix that transforms points from orthographic space (within the bounds specified)
// into clips space (bounds(-1, -1) to (1, 1) with center at (0, 0))
//
Matrix44 Matrix44::MakeOrtho(float leftX, float rightX, float bottomY, float topY, float nearZ, float farZ)
{
	Matrix44 orthoMatrix;

	orthoMatrix.Ix = 2.f / (rightX - leftX);
	orthoMatrix.Jy = 2.f / (topY - bottomY);
	orthoMatrix.Kz = 2.f / (farZ - nearZ);

	orthoMatrix.Tx = -(rightX + leftX) / (rightX - leftX);
	orthoMatrix.Ty = -(topY + bottomY) / (topY - bottomY);
	orthoMatrix.Tz = -(farZ + nearZ) / (farZ - nearZ);

	return orthoMatrix;
}


//-------------------------------------------------------------------------------------------------
Matrix44 Matrix44::MakeOrtho(const Vector2& bottomLeft, const Vector2& topRight, float nearZ/*=0.f*/, float farZ/*=1.0f*/)
{
	return MakeOrtho(bottomLeft.x, topRight.x, bottomLeft.y, topRight.y, nearZ, farZ);
}


//-------------------------------------------------------------------------------------------------
Matrix44 Matrix44::MakePerspective(float fovDegrees, float aspect, float nearZ, float farZ)
{
	float d = (1.f / TanDegrees(0.5f * fovDegrees));

	Matrix44 perspective;

	perspective.Ix = (d / aspect);
	perspective.Jy = d;
	perspective.Kz = (farZ + nearZ) / (farZ - nearZ);
	perspective.Tz = (-2.f * nearZ * farZ) / (farZ - nearZ);

	perspective.Kw = 1.f;
	perspective.Tw = 0.f;

	return perspective;
}


//-------------------------------------------------------------------------------------------------
Vector3 Matrix44::ExtractTranslation(const Matrix44& translationMatrix)
{
	Vector3 translation;

	translation.x = translationMatrix.Tx;
	translation.y = translationMatrix.Ty;
	translation.z = translationMatrix.Tz;

	return translation;
}


//-------------------------------------------------------------------------------------------------
Vector3 Matrix44::ExtractScale(const Matrix44& scaleMatrix)
{
	// TODO: Check signs of cross product to flip correct axes for negative scales
	float xScale = scaleMatrix.GetIVector().GetLength();
	float yScale = scaleMatrix.GetJVector().GetLength();
	float zScale = scaleMatrix.GetKVector().GetLength();

	return Vector3(xScale, yScale, zScale);
}


//-------------------------------------------------------------------------------------------------
Matrix44 Matrix44::GetInverse() const
{	
	Matrix44 inverse = *this;
	inverse.Invert();
	return inverse;
}


//-------------------------------------------------------------------------------------------------
Matrix44 Matrix44::GetInverse(const Matrix44& matrix)
{
	return matrix.GetInverse();
}


//-------------------------------------------------------------------------------------------------
Matrix44 Matrix44::MakeRotation(const Vector3& rotation)
{
	// Rotation about z
	Matrix44 rollMatrix;

	rollMatrix.Ix = CosDegrees(rotation.z);
	rollMatrix.Iy = SinDegrees(rotation.z);

	rollMatrix.Jx = -SinDegrees(rotation.z);
	rollMatrix.Jy = CosDegrees(rotation.z);

	// Rotation about y
	Matrix44 yawMatrix;

	yawMatrix.Ix = CosDegrees(rotation.y);
	yawMatrix.Iz = -SinDegrees(rotation.y);

	yawMatrix.Kx = SinDegrees(rotation.y);
	yawMatrix.Kz = CosDegrees(rotation.y);

	// Rotation about x
	Matrix44 pitchMatrix;

	pitchMatrix.Jy = CosDegrees(rotation.x);
	pitchMatrix.Jz = SinDegrees(rotation.x);

	pitchMatrix.Ky = -SinDegrees(rotation.x);
	pitchMatrix.Kz = CosDegrees(rotation.x);

	// Concatenate and return
	return yawMatrix * pitchMatrix * rollMatrix;
}


//-------------------------------------------------------------------------------------------------
Matrix44 Matrix44::MakeRotation(const Quaternion& rotation)
{
	// Imaginary part
	float const x = rotation.v.x;
	float const y = rotation.v.y;
	float const z = rotation.v.z;

	// Cache off some squares
	float const x2 = x * x;
	float const y2 = y * y;
	float const z2 = z * z;

	// I Basis
	Vector4 iCol = Vector4(
		1.0f - 2.0f * y2 - 2.0f * z2,
		2.0f * x * y + 2.0f * rotation.s * z,
		2.0f * x * z - 2.0f * rotation.s * y,
		0.f
	);

	// J Basis
	Vector4 jCol = Vector4(
		2.f * x * y - 2.0f * rotation.s * z,
		1.0f - 2.0f * x2 - 2.0f * z2,
		2.0f * y * z + 2.0f * rotation.s * x,
		0.f
	);

	// K Basis
	Vector4 kCol = Vector4(
		2.0f * x * z + 2.0f * rotation.s * y,
		2.0f * y * z - 2.0f * rotation.s * x,
		1.0f - 2.0f * x2 - 2.0f * y2,
		0.f
	);

	// T Basis
	Vector4 tCol = Vector4(0.f, 0.f, 0.f, 1.0f);

	Matrix44 result = Matrix44(iCol, jCol, kCol, tCol);
	return result;
}


//-------------------------------------------------------------------------------------------------
Vector3 Matrix44::ExtractRotationDegrees(const Matrix44& rotationMatrix)
{
	float xDegrees;
	float yDegrees;
	float zDegrees;

	float sineX = -1.0f * rotationMatrix.Ky;
	xDegrees = ASinDegrees(sineX);

	float cosX = CosDegrees(xDegrees);
	if (cosX != 0.f)
	{
		yDegrees = Atan2Degrees(rotationMatrix.Kx, rotationMatrix.Kz);
		zDegrees = Atan2Degrees(rotationMatrix.Iy, rotationMatrix.Jy);
	}
	else
	{
		// Gimble lock, lose roll but keep yaw
		zDegrees = 0.f;
		yDegrees = Atan2Degrees(-rotationMatrix.Iz, rotationMatrix.Ix);
	}

	return Vector3(xDegrees, yDegrees, zDegrees);
}


//-------------------------------------------------------------------------------------------------
Matrix44 Matrix44::MakeLookAt(const Vector3& position, const Vector3& target, const Vector3& referenceUp /*= Vector3::Y_AXIS*/)
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

	return Matrix44(right, lookUp, forward, position);
}
