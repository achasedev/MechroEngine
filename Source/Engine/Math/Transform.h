///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: December 13th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Math/Quaternion.h"
#include "Engine/Math/Matrix44.h"
#include "Engine/Math/Vector3.h"

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
class Transform
{
public:
	//-----Public Methods-----

	Transform();
	Transform(const Vector3& startPosition, const Vector3& startRotation, const Vector3& startScale);
	void operator=(const Transform& copyFrom);

	void SetPosition(const Vector3& newPosition);
	void SetRotation(const Vector3& newRotation);
	void SetScale(const Vector3& newScale);

	void SetWorldPosition(const Vector3& newPosition);

	void SetLocalMatrix(const Matrix44& local);
	void SetWorldMatrix(const Matrix44& world);
	void SetParentTransform(Transform* parent);

	void TranslateWorld(const Vector3& worldTranslation);
	void Rotate(const Vector3& deltaRotation);
	void Rotate(const Quaternion& deltaRotation);
	void Scale(const Vector3& deltaScale);

	Matrix44 GetLocalMatrix();			// Matrix that transforms this space to parent's space
	Matrix44 GetParentToWorldMatrix();	// Matrix that transforms from parent space to absolute world space
	Matrix44 GetWorldToParentMatrix();	// Matrix that transforms from absolute world space to parent space
	Matrix44 GetToWorldMatrix();		// Matrix that transforms this space to absolute world space

	Vector3 GetIVector();
	Vector3 GetJVector();
	Vector3 GetKVector();

	Vector3 GetWorldPosition();
	Vector3 GetWorldRotation();


private:
	//-----Private Methods-----

	void UpdateLocalMatrix(bool forceUpdate = false);


public:
	//-----Public Data-----

	// All relative to parent!
	Vector3 position;
	Quaternion rotation;
	Vector3 scale;


private:
	//-----Private Data-----

	// Save previous state to determine when to update matrix
	Vector3		m_oldPosition = Vector3::ZERO;
	Quaternion	m_oldRotation = Quaternion::IDENTITY;
	Vector3		m_oldScale = Vector3::ONES;
	Matrix44	m_localMatrix;
	
	Transform*	m_parentTransform = nullptr;

};
