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
	void SetParentTransform(const Transform* parent, bool keepWorldPosRotScale = false);

	Matrix44 GetLocalToParentMatrix() const;
	Matrix44 GetParentToWorldMatrix() const;
	Matrix44 GetWorldToParentMatrix() const;
	Matrix44 GetLocalToWorldMatrix() const;
	Matrix44 GetWorldToLocalMatrix() const;

	Vector3 GetIVector() const;
	Vector3 GetJVector() const;
	Vector3 GetKVector() const;

	Vector3 GetWorldPosition() const;
	Vector3 GetWorldRotationDegrees() const;
	Quaternion GetWorldRotation() const;
	Vector3 GetWorldScale() const;

	Vector3 TransformPoint(const Vector3& point) const;
	Vector3 InverseTransformPoint(const Vector3& point) const;
	Vector3 TransformDirection(const Vector3& direction) const;
	Vector3 InverseTransformDirection(const Vector3& direction) const;


private:
	//-----Private Methods-----

	void UpdateLocalMatrix(bool forceUpdate = false) const;


public:
	//-----Public Data-----

	Vector3		position = Vector3::ZERO; // Relative to parent's transform
	Quaternion	rotation = Quaternion::IDENTITY; // Relative to my own vectors (i.e. an x rotation rotates me around my own x basis vector)
	Vector3		scale = Vector3::ONES; // Relative to parent's scale (i.e. Parent is 2x, I am 0.5x, relative to world I am 1x)


private:
	//-----Private Data-----

	// Save previous state to determine when to update matrix
	mutable Vector3		m_oldPosition = Vector3::ZERO;
	mutable Quaternion	m_oldRotation = Quaternion::IDENTITY;
	mutable Vector3		m_oldScale = Vector3::ONES;
	mutable Matrix44	m_localMatrix = Matrix44::IDENTITY;
	
	const Transform* m_parentTransform = nullptr;

};
