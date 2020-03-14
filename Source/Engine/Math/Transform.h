/////--------------------------------------------------------------------------------------------------------------------------------------------------
///// Author: Andrew Chase
///// Date Created: December 13th, 2019
///// Description: 
/////--------------------------------------------------------------------------------------------------------------------------------------------------
//#pragma once
//
/////--------------------------------------------------------------------------------------------------------------------------------------------------
///// INCLUDES
/////--------------------------------------------------------------------------------------------------------------------------------------------------
//#include "Engine/Math/Vector3.h"
//#include "Engine/Math/Matrix44.h"
//
/////--------------------------------------------------------------------------------------------------------------------------------------------------
///// DEFINES
/////--------------------------------------------------------------------------------------------------------------------------------------------------
//
/////--------------------------------------------------------------------------------------------------------------------------------------------------
///// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
/////--------------------------------------------------------------------------------------------------------------------------------------------------
//
/////--------------------------------------------------------------------------------------------------------------------------------------------------
///// GLOBALS AND STATICS
/////--------------------------------------------------------------------------------------------------------------------------------------------------
//
/////--------------------------------------------------------------------------------------------------------------------------------------------------
///// CLASS DECLARATIONS
/////--------------------------------------------------------------------------------------------------------------------------------------------------
//
/////--------------------------------------------------------------------------------------------------------------------------------------------------
///// C FUNCTIONS
/////--------------------------------------------------------------------------------------------------------------------------------------------------
//
////-------------------------------------------------------------------------------------------------
//class Transform
//{
//public:
//	//-----Public Methods-----
//
//	Transform();
//	Transform(const Vector3& startPosition, const Vector3& startRotation, const Vector3& startScale);
//	void operator=(const Transform& copyFrom);
//
//	void SetLocalPosition(const Vector3& newPosition);
//	void SetLocalRotation(const Vector3& newRotation);
//	void SetLocalScale(const Vector3& newScale);
//
//	void SetWorldPosition(const Vector3& newPosition);
//	void SetWorldRotation(const Vector3& newRotation);
//	void SetWorldScale(const Vector3& newScale);
//
//	void SetModelMatrix(const Matrix44& model);
//	void SetParentTransform(Transform* parent, bool keepCurrentPosRotScale = true);
//
//	void TranslateWorld(const Vector3& worldTranslation);
//	void TranslateParent(const Vector3& parentTranslation);
//	void TranslateLocal(const Vector3& localTranslation);
//
//	void Rotate(const Vector3& deltaRotation);
//	void Scale(const Vector3& deltaScale);
//
//	Matrix44 GetLocalMatrix();	// Matrix that transforms this space to parent's space
//	Matrix44 GetParentToWorldMatrix(); // Matrix that transforms from parent space to absolute world space
//	Matrix44 GetWorldToParentMatrix(); // Matrix that transforms from absolute world space to parent space
//	Matrix44 GetToWorldMatrix();	// Matrix that transforms this space to absolute world space
//
//	Vector3 GetIVector();
//	Vector3 GetJVector();
//	Vector3 GetKVector();
//
//	Vector3 GetWorldPosition();
//	Vector3 GetWorldRotation();
//
//
//private:
//	//-----Private Methods-----
//
//	void CheckAndUpdateLocalMatrix();
//
//
//public:
//	//-----Public Data-----
//
//	// All defined in parent space!
//	Vector3 position = Vector3::ZERO;
//	Vector3 rotation = Vector3::ZERO;
//	Vector3 scale = Vector3::ONES;
//
//
//private:
//	//-----Private Data-----
//
//	Vector3 m_oldPosition = Vector3::ZERO;
//	Vector3 m_oldRotation = Vector3::ZERO;
//	Vector3 m_oldScale = Vector3::ONES;
//	Matrix44 m_localMatrix; // This transform's local to parent matrix
//	
//	Transform* m_parentTransform = nullptr; // nullptr means parent space is world
//
//};
