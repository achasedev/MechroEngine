///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: May 8th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/Math/Capsule3D.h"
#include "Engine/Math/Cylinder3D.h"
#include "Engine/Math/OBB3.h"
#include "Engine/Math/Sphere3D.h"
#include "Engine/Math/Transform.h"
#include "Engine/Render/Debug/DebugRenderObject.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Entity;
class RigidBody;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class Collider
{
public:
	//-----Public Methods-----
	RTTI_BASE_CLASS(Collider);

	Collider() {}
	Collider(Entity* owningEntity);

	virtual void	ShowDebug() = 0;
	virtual void	HideDebug();
	virtual int		GetColliderMatrixIndex() const = 0;

	bool			OwnerHasRigidBody() const;
	RigidBody*		GetOwnerRigidBody() const;


public:
	//-----Public Data-----

	Entity*					m_entity = nullptr;	// This entity doesn't need a rigidbody! It just means do the collision detection, but no correction
	bool					m_ignoreFriction = false; // If true, friction won't be calculated regardless of what the value of friction is on either collider.
	float					m_friction = 0.3f;
	float					m_restitution = 0.f;


protected:
	//-----Protected Data-----

	static const DebugRenderOptions DEFAULT_COLLIDER_RENDER_OPTIONS;
	DebugRenderObjectHandle m_debugRenderHandle = INVALID_DEBUG_RENDER_OBJECT_HANDLE;

};


//-------------------------------------------------------------------------------------------------
template <typename T>
class TypedCollider : public Collider
{
public:
	//-----Public Methods-----

	TypedCollider() {}
	TypedCollider(Entity* owningEntity, const T& dataLs);

	virtual T GetDataInWorldSpace() const = 0;


protected:
	//-----Protected Data-----

	T m_dataLs; // Defined in the owning entity's transform

};


//-------------------------------------------------------------------------------------------------
template <typename T>
TypedCollider<T>::TypedCollider(Entity* owningEntity, const T& dataLs)
	: Collider(owningEntity)
	, m_dataLs(dataLs)
{
}


//-------------------------------------------------------------------------------------------------
class HalfSpaceCollider : public TypedCollider<Plane3>
{
public:
	//-----Public Methods-----
	RTTI_DERIVED_CLASS(HalfSpaceCollider);

	HalfSpaceCollider() {}
	HalfSpaceCollider(Entity* owningEntity, const Plane3& planeLs);

	virtual void	ShowDebug() override;
	virtual Plane3	GetDataInWorldSpace() const override;
	virtual int		GetColliderMatrixIndex() const { return COLLIDER_MATRIX_INDEX; }


private:
	//-----Private Data-----

	static constexpr int COLLIDER_MATRIX_INDEX = 0;

};


//-------------------------------------------------------------------------------------------------
class PlaneCollider : public TypedCollider<Plane3>
{
public:
	//-----Public Methods-----
	RTTI_DERIVED_CLASS(PlaneCollider);

	PlaneCollider() {}
	PlaneCollider(Entity* owningEntity, const Plane3& planeLs);

	virtual void	ShowDebug() override;
	virtual Plane3	GetDataInWorldSpace() const override;
	virtual int		GetColliderMatrixIndex() const { return COLLIDER_MATRIX_INDEX; }


private:
	//-----Private Data-----

	static constexpr int COLLIDER_MATRIX_INDEX = 1;

};


//-------------------------------------------------------------------------------------------------
class SphereCollider : public TypedCollider<Sphere3D>
{
public:
	//-----Public Methods-----
	RTTI_DERIVED_CLASS(SphereCollider);

	SphereCollider() {}
	SphereCollider(Entity* owningEntity, const Sphere3D& sphereLs);

	virtual void		ShowDebug() override;
	virtual Sphere3D	GetDataInWorldSpace() const override;
	virtual int			GetColliderMatrixIndex() const { return COLLIDER_MATRIX_INDEX; }


private:
	//-----Private Data-----

	static constexpr int COLLIDER_MATRIX_INDEX = 2;

};


//-------------------------------------------------------------------------------------------------
class CapsuleCollider : public TypedCollider<Capsule3D>
{
public:
	//-----Public Methods-----
	RTTI_DERIVED_CLASS(CapsuleCollider);

	CapsuleCollider() {}
	CapsuleCollider(Entity* owningEntity, const Capsule3D& capsuleLs);

	virtual void		ShowDebug() override;
	virtual Capsule3D	GetDataInWorldSpace() const override;
	virtual int			GetColliderMatrixIndex() const { return COLLIDER_MATRIX_INDEX; }


private:
	//-----Private Data-----

	static constexpr int COLLIDER_MATRIX_INDEX = 3;

};


//-------------------------------------------------------------------------------------------------
class BoxCollider : public TypedCollider<OBB3>
{
public:
	//-----Public Methods-----
	RTTI_DERIVED_CLASS(BoxCollider);

	BoxCollider() {}
	BoxCollider(Entity* owningEntity, const OBB3& boxLs);

	virtual void	ShowDebug() override;
	virtual OBB3	GetDataInWorldSpace() const override;
	virtual int		GetColliderMatrixIndex() const { return COLLIDER_MATRIX_INDEX; }


private:
	//-----Private Data-----

	static constexpr int COLLIDER_MATRIX_INDEX = 4;

};


//-------------------------------------------------------------------------------------------------
class CylinderCollider : public TypedCollider<Cylinder3D>
{
public:
	//-----Public Methods-----
	RTTI_DERIVED_CLASS(CylinderCollider);

	CylinderCollider() {}
	CylinderCollider(Entity* owningEntity, const Cylinder3D& cylinderLs);

	virtual void		ShowDebug() override;
	virtual Cylinder3D	GetDataInWorldSpace() const override;
	virtual int			GetColliderMatrixIndex() const { return COLLIDER_MATRIX_INDEX; }


private:
	//-----Private Data-----

	static constexpr int COLLIDER_MATRIX_INDEX = 5;

};
///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
