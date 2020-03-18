///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 17th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

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
	: position(startPosition), rotation(Quaternion::FromEuler(startRotation)), scale(startScale)
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
	rotation = Quaternion::FromEuler(newRotation);
}


//-------------------------------------------------------------------------------------------------
void Transform::SetScale(const Vector3& newScale)
{
	scale = newScale;
}


//-------------------------------------------------------------------------------------------------
void Transform::SetWorldPosition(const Vector3& newPosition)
{
	Matrix44 worldToParent = GetWorldToParentMatrix();
	position = worldToParent.TransformPoint(newPosition).xyz();
}


//-------------------------------------------------------------------------------------------------
void Transform::SetLocalMatrix(const Matrix44& local)
{
	m_localMatrix = local;

	position = Matrix44::ExtractTranslation(local);
	rotation = Quaternion::FromMatrix(local);
	scale = Matrix44::ExtractScale(local);
}


//-------------------------------------------------------------------------------------------------
void Transform::SetWorldMatrix(const Matrix44& world)
{
	Matrix44 worldToParent = GetWorldToParentMatrix();
	m_localMatrix = worldToParent * world;
}


//-------------------------------------------------------------------------------------------------
// TODO: Update to allow keeping world transform on re-parent
void Transform::SetParentTransform(Transform* parent)
{
	m_parentTransform = parent;
}


//-------------------------------------------------------------------------------------------------
void Transform::TranslateWorld(const Vector3& worldTranslation)
{
	Matrix44 worldToParent = GetWorldToParentMatrix();
	Vector3 translation = worldToParent.TransformVector(worldTranslation).xyz();
	position += translation;
}


//-------------------------------------------------------------------------------------------------
void Transform::Rotate(const Vector3& deltaRotation)
{
	Rotate(Quaternion::FromEuler(deltaRotation));
}


//-------------------------------------------------------------------------------------------------
void Transform::Rotate(const Quaternion& deltaRotation)
{
	rotation *= deltaRotation;
}


//-------------------------------------------------------------------------------------------------
void Transform::Scale(const Vector3& deltaScale)
{
	scale.x *= deltaScale.x;
	scale.y *= deltaScale.y;
	scale.z *= deltaScale.z;
}


//-------------------------------------------------------------------------------------------------
Matrix44 Transform::GetLocalMatrix()
{
	return m_localMatrix;
}


//-------------------------------------------------------------------------------------------------
Matrix44 Transform::GetParentToWorldMatrix()
{
	if (m_parentTransform != nullptr)
	{
		return m_parentTransform->GetToWorldMatrix();
	}

	return Matrix44::IDENTITY;
}


//-------------------------------------------------------------------------------------------------
Matrix44 Transform::GetWorldToParentMatrix()
{
	if (m_parentTransform != nullptr)
	{
		// TODO: Quick inverse?
		return Matrix44::GetInverse(m_parentTransform->GetToWorldMatrix());
	}

	return Matrix44::IDENTITY;
}


//-------------------------------------------------------------------------------------------------
Matrix44 Transform::GetToWorldMatrix()
{
	UpdateLocalMatrix();

	if (m_parentTransform != nullptr)
	{
		return m_parentTransform->GetToWorldMatrix() * m_localMatrix;
	}

	return m_localMatrix;
}


//-------------------------------------------------------------------------------------------------
Vector3 Transform::GetIVector()
{
	UpdateLocalMatrix();
	return m_localMatrix.GetIVector().xyz();
}


//-------------------------------------------------------------------------------------------------
Vector3 Transform::GetJVector()
{
	UpdateLocalMatrix();
	return m_localMatrix.GetJVector().xyz();
}


//-------------------------------------------------------------------------------------------------
Vector3 Transform::GetKVector()
{
	UpdateLocalMatrix();
	return m_localMatrix.GetKVector().xyz();
}


//-------------------------------------------------------------------------------------------------
Vector3 Transform::GetWorldPosition()
{
	Matrix44 parentToWorld = GetParentToWorldMatrix();
	return parentToWorld.TransformPoint(position).xyz();
}


//-------------------------------------------------------------------------------------------------
Vector3 Transform::GetWorldRotation()
{
	UpdateLocalMatrix();

	Matrix44 toWorldMatrix = GetToWorldMatrix();
	return Matrix44::ExtractRotationDegrees(toWorldMatrix);
}


//-------------------------------------------------------------------------------------------------
void Transform::UpdateLocalMatrix(bool forceUpdate /*= false*/)
{
	// Check if it needs to be updated first
	bool translationUpToDate = AreMostlyEqual(position, m_oldPosition);
	bool rotationUpToDate = AreMostlyEqual(rotation, m_oldRotation);
	bool scaleUpToDate = AreMostlyEqual(scale, m_oldScale);

	// If any out of date, recalculate the matrix
	if (!translationUpToDate || !rotationUpToDate || !scaleUpToDate || forceUpdate)
	{
		Matrix44 translationMatrix = Matrix44::MakeTranslation(position);
		Matrix44 rotationMatrix = Matrix44::MakeRotation(rotation);
		Matrix44 scaleMatrix = Matrix44::MakeScale(scale);

		m_localMatrix = translationMatrix * rotationMatrix * scaleMatrix;

		// Set old values for the next call
		m_oldPosition = position;
		m_oldRotation = rotation;
		m_oldScale = scale;
	}
}
