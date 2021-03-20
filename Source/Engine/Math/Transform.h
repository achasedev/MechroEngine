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
enum TransformRelation
{
	RELATIVE_TO_SELF,
	RELATIVE_TO_PARENT,
	RELATIVE_TO_WORLD
};


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

	void Translate(float xTranslation, float yTranslation, float zTranslation, TransformRelation relativeTo = RELATIVE_TO_SELF);
	void Translate(const Vector3& translation, TransformRelation relativeTo = RELATIVE_TO_SELF);
	void Rotate(float xRotation, float yRotation, float zRotation);
	void Rotate(const Vector3& deltaRotation);
	void Rotate(const Quaternion& deltaRotation);
	void Scale(float xScale, float yScale, float zScale);
	void Scale(const Vector3& axisScalars);
	void Scale(float uniformScale);
	
	void SetWorldPosition(const Vector3& newPosition);

	void SetLocalMatrix(const Matrix44& local);
	void SetWorldMatrix(const Matrix44& world);
	void SetParentTransform(Transform* parent, bool keepWorldPosRotScale = false);

	Matrix44 GetLocalToParentMatrix();
	Matrix44 GetParentToWorldMatrix();
	Matrix44 GetWorldToParentMatrix();
	Matrix44 GetLocalToWorldMatrix();

	Vector3 GetIVector();
	Vector3 GetJVector();
	Vector3 GetKVector();

	Vector3 GetWorldPosition();
	Vector3 GetWorldRotation();
	Vector3 GetWorldScale();

	Vector3 TransformPositionLocalToWorld(const Vector3& point);


private:
	//-----Private Methods-----

	void UpdateLocalMatrix(bool forceUpdate = false);


public:
	//-----Public Data-----

	// All defined in parent space!
	Vector3		position			= Vector3::ZERO;
	Quaternion	rotation			= Quaternion::IDENTITY;
	Vector3		scale				= Vector3::ONES;


private:
	//-----Private Data-----

	// Save previous state to determine when to update matrix
	Vector3		m_oldPosition		= Vector3::ZERO;
	Quaternion	m_oldRotation		= Quaternion::IDENTITY;
	Vector3		m_oldScale			= Vector3::ONES;
	Matrix44	m_localMatrix		= Matrix44::IDENTITY;
	
	Transform*	m_parentTransform	= nullptr;

};
