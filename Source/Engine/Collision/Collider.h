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
#include "Engine/Math/Capsule3.h"
#include "Engine/Math/Cylinder.h"
#include "Engine/Math/OBB3.h"
#include "Engine/Math/Polyhedron.h"
#include "Engine/Math/Sphere.h"
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
	virtual int		GetTypeIndex() const = 0;

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
	virtual int		GetTypeIndex() const { return TYPE_INDEX; }


public:
	//-----Public Data-----

	static constexpr int TYPE_INDEX = 0;


private:
	//-----Private Data-----

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
	virtual int		GetTypeIndex() const { return TYPE_INDEX; }


public:
	//-----Public Data-----

	static constexpr int TYPE_INDEX = 1;


private:
	//-----Private Data-----

};


//-------------------------------------------------------------------------------------------------
class SphereCollider : public TypedCollider<Sphere>
{
public:
	//-----Public Methods-----
	RTTI_DERIVED_CLASS(SphereCollider);

	SphereCollider() {}
	SphereCollider(Entity* owningEntity, const Sphere& sphereLs);

	virtual void		ShowDebug() override;
	virtual Sphere	GetDataInWorldSpace() const override;
	virtual int			GetTypeIndex() const { return TYPE_INDEX; }


public:
	//-----Public Data-----

	static constexpr int TYPE_INDEX = 2;


private:
	//-----Private Data-----

};


//-------------------------------------------------------------------------------------------------
class CapsuleCollider : public TypedCollider<Capsule3>
{
public:
	//-----Public Methods-----
	RTTI_DERIVED_CLASS(CapsuleCollider);

	CapsuleCollider() {}
	CapsuleCollider(Entity* owningEntity, const Capsule3& capsuleLs);

	virtual void		ShowDebug() override;
	virtual Capsule3	GetDataInWorldSpace() const override;
	virtual int			GetTypeIndex() const { return TYPE_INDEX; }


public:
	//-----Public Data-----

	static constexpr int TYPE_INDEX = 3;


private:
	//-----Private Data-----

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
	virtual int		GetTypeIndex() const { return TYPE_INDEX; }


public:
	//-----Public Data-----

	static constexpr int TYPE_INDEX = 4;


private:
	//-----Private Data-----

};


//-------------------------------------------------------------------------------------------------
class CylinderCollider : public TypedCollider<Cylinder>
{
public:
	//-----Public Methods-----
	RTTI_DERIVED_CLASS(CylinderCollider);

	CylinderCollider() {}
	CylinderCollider(Entity* owningEntity, const Cylinder& cylinderLs);

	virtual void		ShowDebug() override;
	virtual Cylinder	GetDataInWorldSpace() const override;
	virtual int			GetTypeIndex() const { return TYPE_INDEX; }


public:
	//-----Public Data-----

	static constexpr int TYPE_INDEX = 5;


private:
	//-----Private Data-----

};


//-------------------------------------------------------------------------------------------------
class ConvexHullCollider : public TypedCollider<Polyhedron>
{
public:
	//-----Public Methods-----
	RTTI_DERIVED_CLASS(ConvexHullCollider);

	ConvexHullCollider() {}
	ConvexHullCollider(Entity* owningEntity, const Polyhedron& hullLs);

	virtual void		ShowDebug() override;
	virtual Polyhedron	GetDataInWorldSpace() const override;
	virtual int			GetTypeIndex() const { return TYPE_INDEX; }


public:
	//-----Public Data-----

	static constexpr int TYPE_INDEX = 6;


private:
	//-----Private Data-----

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
