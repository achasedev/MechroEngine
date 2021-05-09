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
template <typename T>
class CollisionPrimitive
{
public:
	//-----Public Methods-----

	CollisionPrimitive() {}
	CollisionPrimitive(Entity* owningEntity);

	virtual T	GetDataInWorldSpace() const = 0;

	bool		HasRigidBody() const { return entity->rigidBody != nullptr; }
	RigidBody*	GetRigidBody() const { return entity->rigidBody; }


protected:
	//-----Protected Data-----

	Entity*		entity = nullptr;	// This entity doesn't need a rigidbody! It just means do the collision detection, but no correction
	T			dataLs;				// Defined in the owning entity's transform

};


//-------------------------------------------------------------------------------------------------
template <typename T>
CollisionPrimitive<T>::CollisionPrimitive(Entity* owningEntity)
	: entity(owningEntity)
{
}


//-------------------------------------------------------------------------------------------------
class CollisionSphere : public CollisionPrimitive<Sphere3D>
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
class CollisionHalfSpace : public CollisionPrimitive<Plane3>
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
class CollisionBox : public CollisionPrimitive<OBB3>
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
