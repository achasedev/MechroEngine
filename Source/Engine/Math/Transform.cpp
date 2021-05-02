///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 17th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/Transform.h"

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
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
Transform::Transform()
{
	UpdateLocalMatrix(true);
}


//-------------------------------------------------------------------------------------------------
Transform::Transform(const Vector3& startPosition, const Vector3& startRotation, const Vector3& startScale)
	: position(startPosition), rotation(Quaternion::CreateFromEulerAnglesDegrees(startRotation)), scale(startScale)
{
	UpdateLocalMatrix(true);
}


//-------------------------------------------------------------------------------------------------
void Transform::operator=(const Transform& copyFrom)
{
	position = copyFrom.position;
	rotation = copyFrom.rotation;
	scale = copyFrom.scale;

	m_parentTransform = copyFrom.m_parentTransform;
}


//-------------------------------------------------------------------------------------------------
void Transform::SetPosition(const Vector3& newPosition)
{
	position = newPosition;
}


//-------------------------------------------------------------------------------------------------
void Transform::SetRotation(const Vector3& newRotation)
{
	rotation = Quaternion::CreateFromEulerAnglesDegrees(newRotation);
}


//-------------------------------------------------------------------------------------------------
void Transform::SetScale(const Vector3& newScale)
{
	scale = newScale;
}


//-------------------------------------------------------------------------------------------------
void Transform::Translate(const Vector3& translation, TransformRelativeTo relativeTo /*= RELATIVE_TO_SELF*/)
{
	switch (relativeTo)
	{
	case RELATIVE_TO_SELF:
	{
		UpdateLocalMatrix();
		Vector3 localTranslation = m_localMatrix.TransformVector(translation).xyz();
		position += localTranslation;
	}
		break;
	case RELATIVE_TO_PARENT:
		position += translation;
		break;
	case RELATIVE_TO_WORLD:
	{
		Matrix4 worldToParent = GetWorldToParentMatrix();
		Vector3 worldTranslation = worldToParent.TransformVector(translation).xyz();
		position += worldTranslation;
	}
		break;
	default:
		break;
	}
}


//-------------------------------------------------------------------------------------------------
void Transform::Translate(float xTranslation, float yTranslation, float zTranslation, TransformRelativeTo relativeTo /*= RELATIVE_TO_SELF*/)
{
	Translate(Vector3(xTranslation, yTranslation, zTranslation), relativeTo);
}


//-------------------------------------------------------------------------------------------------
void Transform::SetWorldPosition(const Vector3& newPosition)
{
	Matrix4 worldToParent = GetWorldToParentMatrix();
	position = worldToParent.TransformPoint(newPosition).xyz();
}


//-------------------------------------------------------------------------------------------------
void Transform::SetLocalMatrix(const Matrix4& local)
{
	m_localMatrix = local;

	position = Matrix4::ExtractTranslation(local);
	rotation = Quaternion::FromMatrix(local);
	scale = Matrix4::ExtractScale(local);
}


//-------------------------------------------------------------------------------------------------
void Transform::SetWorldMatrix(const Matrix4& world)
{
	Matrix4 worldToParent = GetWorldToParentMatrix();
	SetLocalMatrix(worldToParent * world);
}


//-------------------------------------------------------------------------------------------------
void Transform::SetParentTransform(const Transform* parent, bool keepWorldPosRotScale /*= false*/)
{
	if (keepWorldPosRotScale)
	{
		Matrix4 oldWorld = GetLocalToWorldMatrix();
		m_parentTransform = parent;
		Matrix4 worldToParent = GetWorldToParentMatrix();
		SetLocalMatrix(worldToParent * oldWorld);
	}
	else
	{
		m_parentTransform = parent;
	}
}


//-------------------------------------------------------------------------------------------------
void Transform::RotateDegrees(float xDegrees, float yDegrees, float zDegrees, TransformRelativeTo relativeTo /*= RELATIVE_TO_SELF*/)
{
	RotateDegrees(Vector3(xDegrees, yDegrees, zDegrees), relativeTo);
}


//-------------------------------------------------------------------------------------------------
void Transform::RotateDegrees(const Vector3& rotationDegrees, TransformRelativeTo relativeTo /*= RELATIVE_TO_SELF*/)
{
	Rotate(Quaternion::CreateFromEulerAnglesDegrees(rotationDegrees), relativeTo);
}


//-------------------------------------------------------------------------------------------------
void Transform::RotateRadians(float xRadians, float yRadians, float zRadians, TransformRelativeTo relativeTo /*= RELATIVE_TO_SELF*/)
{
	RotateRadians(Vector3(xRadians, yRadians, zRadians), relativeTo);
}


//-------------------------------------------------------------------------------------------------
void Transform::RotateRadians(const Vector3& rotationRadians, TransformRelativeTo relativeTo /*= RELATIVE_TO_SELF*/)
{
	Rotate(Quaternion::CreateFromEulerAnglesRadians(rotationRadians), relativeTo);
}


//-------------------------------------------------------------------------------------------------
void Transform::Rotate(const Quaternion& deltaRotation, TransformRelativeTo relativeTo /*= RELATIVE_TO_SELF*/)
{
	switch (relativeTo)
	{
	case RELATIVE_TO_SELF:
		rotation *= deltaRotation;
		break;
	case RELATIVE_TO_PARENT:
		rotation = deltaRotation * rotation;
		break;
	case RELATIVE_TO_WORLD:
		if (m_parentTransform == nullptr)
		{
			rotation = deltaRotation * rotation;
		}
		else
		{
			Quaternion parentWorldRotation = m_parentTransform->GetWorldRotation();
			Quaternion invParentWorldRotation = parentWorldRotation.GetInverse();
			Quaternion newWorldRotation = deltaRotation * parentWorldRotation * rotation;
			rotation = invParentWorldRotation * newWorldRotation;
		}
		break;
	default:
		ERROR_AND_DIE("Bad enum!");
		break;
	}
}


//-------------------------------------------------------------------------------------------------
void Transform::Scale(const Vector3& axisScalars)
{
	scale.x *= axisScalars.x;
	scale.y *= axisScalars.y;
	scale.z *= axisScalars.z;
}


//-------------------------------------------------------------------------------------------------
void Transform::Scale(float uniformScale)
{
	scale *= uniformScale;
}


//-------------------------------------------------------------------------------------------------
void Transform::Scale(float xScale, float yScale, float zScale)
{
	Scale(Vector3(xScale, yScale, zScale));
}


//-------------------------------------------------------------------------------------------------
Matrix4 Transform::GetLocalToParentMatrix() const
{
	return m_localMatrix;
}


//-------------------------------------------------------------------------------------------------
Matrix4 Transform::GetParentToWorldMatrix() const
{
	if (m_parentTransform != nullptr)
	{
		return m_parentTransform->GetLocalToWorldMatrix();
	}

	return Matrix4::IDENTITY;
}


//-------------------------------------------------------------------------------------------------
Matrix4 Transform::GetWorldToParentMatrix() const
{
	if (m_parentTransform != nullptr)
	{
		return Matrix4::GetInverse(m_parentTransform->GetLocalToWorldMatrix());
	}

	return Matrix4::IDENTITY;
}


//-------------------------------------------------------------------------------------------------
Matrix4 Transform::GetLocalToWorldMatrix() const
{
	UpdateLocalMatrix();

	if (m_parentTransform != nullptr)
	{
		return m_parentTransform->GetLocalToWorldMatrix() * m_localMatrix;
	}

	return m_localMatrix;
}


//-------------------------------------------------------------------------------------------------
Matrix4 Transform::GetWorldToLocalMatrix() const
{
	return Matrix4::GetInverse(GetLocalToWorldMatrix());
}


//-------------------------------------------------------------------------------------------------
Vector3 Transform::GetIVector() const
{
	return GetLocalToWorldMatrix().GetIVector().xyz();
}


//-------------------------------------------------------------------------------------------------
Vector3 Transform::GetJVector() const
{
	return GetLocalToWorldMatrix().GetJVector().xyz();
}


//-------------------------------------------------------------------------------------------------
Vector3 Transform::GetKVector() const
{
	return GetLocalToWorldMatrix().GetKVector().xyz();
}


//-------------------------------------------------------------------------------------------------
Vector3 Transform::GetWorldPosition() const
{
	Matrix4 parentToWorld = GetParentToWorldMatrix();
	return parentToWorld.TransformPoint(position).xyz();
}


//-------------------------------------------------------------------------------------------------
Quaternion Transform::GetWorldRotation() const
{
	if (m_parentTransform != nullptr)
	{
		Quaternion parentWorldRotation = GetWorldRotation();
		return parentWorldRotation * rotation;
	}

	return rotation;
}


//-------------------------------------------------------------------------------------------------
Vector3 Transform::GetWorldScale() const
{
	UpdateLocalMatrix();

	Matrix4 toWorldMatrix = GetLocalToWorldMatrix();
	return Matrix4::ExtractScale(toWorldMatrix);
}


//-------------------------------------------------------------------------------------------------
Vector3 Transform::TransformPoint(const Vector3& point) const
{
	Matrix4 localToWorld = GetLocalToWorldMatrix();
	Vector4 result = localToWorld.TransformPoint(point);

	return result.xyz();
}


//-------------------------------------------------------------------------------------------------
Vector3 Transform::InverseTransformDirection(const Vector3& direction) const
{
	Matrix4 worldToLocal = GetWorldToLocalMatrix();
	Vector4 result = worldToLocal.TransformVector(direction);

	return result.xyz();
}


//-------------------------------------------------------------------------------------------------
void Transform::UpdateLocalMatrix(bool forceUpdate /*= false*/) const
{
	// Check if it needs to be updated first
	bool translationUpToDate = AreMostlyEqual(position, m_oldPosition);
	bool rotationUpToDate = AreMostlyEqual(rotation, m_oldRotation);
	bool scaleUpToDate = AreMostlyEqual(scale, m_oldScale);

	// If any out of date, recalculate the matrix
	if (!translationUpToDate || !rotationUpToDate || !scaleUpToDate || forceUpdate)
	{
		Matrix4 translationMatrix = Matrix4::MakeTranslation(position);
		Matrix4 rotationMatrix = Matrix4::MakeRotation(rotation);
		Matrix4 scaleMatrix = Matrix4::MakeScale(scale);

		m_localMatrix = translationMatrix * rotationMatrix * scaleMatrix;

		// Set old values for the next call
		m_oldPosition = position;
		m_oldRotation = rotation;
		m_oldScale = scale;
	}
}
