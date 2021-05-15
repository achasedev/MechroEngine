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
#include "Engine/Math/OBB3.h"
#include "Engine/Math/Sphere3D.h"
#include "Engine/Math/Transform.h"

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

	bool		OwnerHasRigidBody() const;
	RigidBody*	GetOwnerRigidBody() const;


public:
	//-----Public Data-----

	Entity* entity = nullptr;	// This entity doesn't need a rigidbody! It just means do the collision detection, but no correction

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
class SphereCollider : public TypedCollider<Sphere3D>
{
public:
	//-----Public Methods-----
	RTTI_DERIVED_CLASS(SphereCollider);

	SphereCollider() {}
	SphereCollider(Entity* owningEntity, const Sphere3D& sphereLs);

	virtual Sphere3D GetDataInWorldSpace() const override;


private:
	//-----Private Data-----

};


//-------------------------------------------------------------------------------------------------
class HalfSpaceCollider : public TypedCollider<Plane3>
{
public:
	//-----Public Methods-----
	RTTI_DERIVED_CLASS(HalfSpaceCollider);

	HalfSpaceCollider() {}
	HalfSpaceCollider(Entity* owningEntity, const Plane3& planeLs);

	virtual Plane3 GetDataInWorldSpace() const override;


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

	virtual OBB3 GetDataInWorldSpace() const override;


private:
	//-----Private Data-----

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
