///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: May 8th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
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
class CollisionPrimitive
{
public:
	//-----Public Methods-----

	CollisionPrimitive() {}
	CollisionPrimitive(Entity* owningEntity);

	inline bool			OwnerHasRigidBody() const;
	inline RigidBody*	GetOwnerRigidBody() const;


public:
	//-----Public Data-----

	Entity* entity = nullptr;	// This entity doesn't need a rigidbody! It just means do the collision detection, but no correction

};


//-------------------------------------------------------------------------------------------------
template <typename T>
class TypedCollisionPrimitive : public CollisionPrimitive
{
public:
	//-----Public Methods-----

	TypedCollisionPrimitive() {}
	TypedCollisionPrimitive(Entity* owningEntity, const T& dataLs);

	virtual T GetDataInWorldSpace() const = 0;


protected:
	//-----Protected Data-----

	T m_dataLs; // Defined in the owning entity's transform

};


//-------------------------------------------------------------------------------------------------
template <typename T>
TypedCollisionPrimitive<T>::TypedCollisionPrimitive(Entity* owningEntity, const T& dataLs)
	: CollisionPrimitive(owningEntity)
	, m_dataLs(dataLs)
{
}
 

//-------------------------------------------------------------------------------------------------
class CollisionSphere : public TypedCollisionPrimitive<Sphere3D>
{
public:
	//-----Public Methods-----

	CollisionSphere() {}
	CollisionSphere(Entity* owningEntity, const Sphere3D& sphereLs);

	virtual Sphere3D GetDataInWorldSpace() const override;


private:
	//-----Private Data-----

};


//-------------------------------------------------------------------------------------------------
class CollisionHalfSpace : public TypedCollisionPrimitive<Plane3>
{
public:
	//-----Public Methods-----

	CollisionHalfSpace() {}
	CollisionHalfSpace(Entity* owningEntity, const Plane3& planeLs);

	virtual Plane3 GetDataInWorldSpace() const override;


private:
	//-----Private Data-----

};

//-------------------------------------------------------------------------------------------------
class CollisionBox : public TypedCollisionPrimitive<OBB3>
{
public:
	//-----Public Methods-----

	CollisionBox() {}
	CollisionBox(Entity* owningEntity, const OBB3& boxLs);

	virtual OBB3 GetDataInWorldSpace() const override;


private:
	//-----Private Data-----

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
