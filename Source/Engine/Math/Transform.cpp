/////--------------------------------------------------------------------------------------------------------------------------------------------------
///// Author: Andrew Chase
///// Date Created: December 13th, 2019
///// Description: 
/////--------------------------------------------------------------------------------------------------------------------------------------------------
//
/////--------------------------------------------------------------------------------------------------------------------------------------------------
/////                                                             *** INCLUDES ***
/////--------------------------------------------------------------------------------------------------------------------------------------------------
//#include "Engine/Framework/EngineCommon.h"
//#include "Engine/Math/MathUtils.h"
//#include "Engine/Math/Transform.h"
//
/////--------------------------------------------------------------------------------------------------------------------------------------------------
/////                                                             *** DEFINES ***
/////--------------------------------------------------------------------------------------------------------------------------------------------------
//
/////--------------------------------------------------------------------------------------------------------------------------------------------------
/////                                                              *** TYPES ***
/////--------------------------------------------------------------------------------------------------------------------------------------------------
//
/////--------------------------------------------------------------------------------------------------------------------------------------------------
/////                                                             *** STRUCTS ***
/////--------------------------------------------------------------------------------------------------------------------------------------------------
//
/////--------------------------------------------------------------------------------------------------------------------------------------------------
/////                                                        *** GLOBALS AND STATICS ***
/////--------------------------------------------------------------------------------------------------------------------------------------------------
//
/////--------------------------------------------------------------------------------------------------------------------------------------------------
/////                                                           *** C FUNCTIONS ***
/////--------------------------------------------------------------------------------------------------------------------------------------------------
//
/////--------------------------------------------------------------------------------------------------------------------------------------------------
/////                                                             *** CLASSES ***
/////--------------------------------------------------------------------------------------------------------------------------------------------------
//
//
////-------------------------------------------------------------------------------------------------
//Transform::Transform(const Vector3& position, const Vector3& rotationP, const Vector3& scale)
//	: position(position), rotation(rotationP), scale(scale)
//{
//	CheckAndUpdateLocalMatrix();
//}
//
//
////-------------------------------------------------------------------------------------------------
//Transform::Transform()
//{
//	CheckAndUpdateLocalMatrix();
//}
//
//
////-------------------------------------------------------------------------------------------------
//void Transform::operator=(const Transform& copyFrom)
//{
//	position = copyFrom.position;
//	rotation = copyFrom.rotation;
//	scale = copyFrom.scale;
//}
//
//
////-------------------------------------------------------------------------------------------------
//void Transform::SetLocalPosition(const Vector3& newPosition)
//{
//	position = newPosition;
//}
//
//
////-------------------------------------------------------------------------------------------------
//void Transform::SetLocalRotation(const Vector3& newRotation)
//{
//	rotation = newRotation;
//}
//
//
////-------------------------------------------------------------------------------------------------
//void Transform::SetLocalScale(const Vector3& newScale)
//{
//	scale = newScale;
//}
//
//
////-------------------------------------------------------------------------------------------------
//void Transform::SetModelMatrix(const Matrix44& model)
//{
//	m_localMatrix = model;
//
//	// Update extracted members to keep them up to date
//	position = Matrix44::ExtractTranslation(model);
//	rotation = Matrix44::ExtractRotationDegrees(model);
//	scale = Matrix44::ExtractScale(model);
//}
//
//
////-------------------------------------------------------------------------------------------------
//void Transform::SetParentTransform(Transform* parent, bool keepWorldPosRotScale /* = true*/)
//{
//	if (keepWorldPosRotScale)
//	{
//		// Get our current parent-relative position/rotation/scale in world coordinates
//		Vector3 worldPosition = GetWorldPosition();
//		Vector3 worldRotation = GetWorldRotation();
//		Vector3 worldScale = GetWorldScale();
//
//		// Update parent
//		m_parentTransform = parent;
//
//		// Re-convert our world position/rotation/scale into our new parent's space
//		SetWorldPosition(worldPosition);
//		SetWorldRotation(worldRotation);
//		SetWorldScale(worldScale);
//	}
//	else
//	{
//		m_parentTransform = parent;
//	}
//}
//
//
////-------------------------------------------------------------------------------------------------
//void Transform::TranslateWorld(const Vector3& worldTranslation)
//{
//	Matrix44 parentToWorld = GetParentToWorldMatrix();
//	Matrix44 worldToParent = parentToWorld.GetInverse();
//	Vector3 parentSpaceTranslation = worldToParent.TransformVector(worldTranslation).xyz();
//
//	position += parentSpaceTranslation;
//}
//
//
////-------------------------------------------------------------------------------------------------
//void Transform::TranslateParent(const Vector3& parentTranslation)
//{
//	position += parentTranslation;
//}
//
//
////-------------------------------------------------------------------------------------------------
//void Transform::TranslateLocal(const Vector3& localTranslation)
//{
//	CheckAndUpdateLocalMatrix();
//
//	Vector3 parentSpaceTranslation = m_localMatrix.TransformVector(localTranslation).xyz();
//	position += parentSpaceTranslation;
//}
//
//
////-------------------------------------------------------------------------------------------------
//void Transform::Rotate(const Vector3& deltaRotation)
//{
//	rotation += deltaRotation;
//
//	rotation.x = GetAngleBetweenZeroThreeSixty(rotation.x);
//	rotation.y = GetAngleBetweenZeroThreeSixty(rotation.y);
//	rotation.z = GetAngleBetweenZeroThreeSixty(rotation.z);
//}
//
//
////-------------------------------------------------------------------------------------------------
//void Transform::Scale(const Vector3& deltaScale)
//{
//	scale.x *= deltaScale.x;
//	scale.y *= deltaScale.y;
//	scale.z *= deltaScale.z;
//}
//
//
////-----------------------------------------------------------------------------------------------
//// Returns the model matrix of this transform, recalculating it if it's outdated
////
//Matrix44 Transform::GetLocalMatrix()
//{
//	CheckAndUpdateLocalMatrix();
//	return m_localMatrix;
//}
//
//
////-----------------------------------------------------------------------------------------------
//// Returns the matrix that transforms this space to absolute world space
////
//Matrix44 Transform::GetToWorldMatrix()
//{
//	CheckAndUpdateLocalMatrix();
//
//	if (m_parentTransform != nullptr)
//	{
//		Matrix44 parentWorld = m_parentTransform->GetToWorldMatrix();
//		return parentWorld * m_localMatrix;
//	}
//	else
//	{
//		return m_localMatrix;
//	}
//}
//
//
////-----------------------------------------------------------------------------------------------
//// Returns the parent's matrix transformation, from parent space to world space
////
//Matrix44 Transform::GetParentToWorldMatrix()
//{
//	if (m_parentTransform != nullptr)
//	{
//		return m_parentTransform->GetToWorldMatrix();
//	}
//
//	return Matrix44::IDENTITY;
//}
//
//
////-------------------------------------------------------------------------------------------------
//Matrix44 Transform::GetWorldToParentMatrix()
//{
//	return GetParentToWorldMatrix().GetInverse();
//}
//
//
////-----------------------------------------------------------------------------------------------
//// Returns the world right vector for this transform
////
//Vector3 Transform::GetIVector()
//{
//	return GetToWorldMatrix().GetIVector().xyz();
//}
//
//
////-----------------------------------------------------------------------------------------------
//// Returns the world up vector for this transform
////
//Vector3 Transform::GetJVector()
//{
//	return GetToWorldMatrix().GetJVector().xyz();
//}
//
//
////-----------------------------------------------------------------------------------------------
//// Returns the world forward vector for this transform
////
//Vector3 Transform::GetKVector()
//{
//	return GetToWorldMatrix().GetKVector().xyz();
//}
//
//
////-----------------------------------------------------------------------------------------------
//// Returns the world position of the transform
////
//Vector3 Transform::GetWorldPosition()
//{
//	CheckAndUpdateLocalMatrix();
//
//	Matrix44 toWorldMatrix = GetToWorldMatrix();
//	return Matrix44::ExtractTranslation(toWorldMatrix);
//}
//
//
////-----------------------------------------------------------------------------------------------
//// Returns the world rotation of the transform, as Euler angles in degrees
////
//Vector3 Transform::GetWorldRotation()
//{
//	CheckAndUpdateLocalMatrix();
//
//	Matrix44 toWorldMatrix = GetToWorldMatrix();
//	return Matrix44::ExtractRotationDegrees(toWorldMatrix);
//}
//
//
////-----------------------------------------------------------------------------------------------
//// Recalculates the model matrix of this transform given its current position, rotation, and scale
////
//void Transform::CheckAndUpdateLocalMatrix()
//{
//	// Check if it needs to be updated first
//	bool translationUpToDate = AreMostlyEqual(position, m_oldPosition);
//	bool rotationUpToDate = AreMostlyEqual(rotation, m_oldRotation);
//	bool scaleUpToDate = AreMostlyEqual(scale, m_oldScale);
//
//	// If any out of date, recalculate the matrix
//	if (!translationUpToDate || !rotationUpToDate || !scaleUpToDate)
//	{
//		Matrix44 translationMatrix = Matrix44::MakeTranslation(position);
//		Matrix44 rotationMatrix = Matrix44::MakeRotation(rotation);
//		Matrix44 scaleMatrix = Matrix44::MakeScale(scale);
//
//		m_localMatrix = translationMatrix * rotationMatrix * scaleMatrix;
//
//		// Set old values for the next call
//		m_oldPosition = position;
//		m_oldRotation = rotation;
//		m_oldScale = scale;
//	}
//}
