///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 18th, 2021
/// Description: Library of functions for 3D collision detection
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Collision/3D/Collider3d.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
struct BroadphaseResult3d
{
	BroadphaseResult3d() {}
	BroadphaseResult3d(bool collisionFound)
		: m_collisionFound(collisionFound) {}

	bool	m_collisionFound = false;
	Vector3 m_direction = Vector3::ZERO;
	float	m_penetration = FLT_MAX;
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
namespace CollisionUtils3d
{
	// Sphere/Sphere
	BroadphaseResult3d Collide(SphereCollider3d* colA, SphereCollider3d* colB);

	// Box/Box
	BroadphaseResult3d Collide(BoxCollider3d* colA, BoxCollider3d* colB);

	// Capsule/Capsule
	BroadphaseResult3d Collide(CapsuleCollider3d* colA, CapsuleCollider3d* colB);

	// Polytope/Polytope
	BroadphaseResult3d Collide(PolytopeCollider3d* colA, PolytopeCollider3d* colB);

	// Sphere/Box
	BroadphaseResult3d Collide(SphereCollider3d* colA, BoxCollider3d* colB);
	BroadphaseResult3d Collide(BoxCollider3d* colA, SphereCollider3d* colB);

	// Sphere/Capsule
	BroadphaseResult3d Collide(SphereCollider3d* colA, CapsuleCollider3d* colB);
	BroadphaseResult3d Collide(CapsuleCollider3d* colA, SphereCollider3d* colB);

	// Sphere/Polytope
	BroadphaseResult3d Collide(SphereCollider3d* colA, PolytopeCollider3d* colB);
	BroadphaseResult3d Collide(PolytopeCollider3d* colA, SphereCollider3d* colB);

	// Box/Capsule
	BroadphaseResult3d Collide(BoxCollider3d* colA, CapsuleCollider3d* colB);
	BroadphaseResult3d Collide(CapsuleCollider3d* colA, BoxCollider3d* colB);

	// Box/Polytope
	BroadphaseResult3d Collide(BoxCollider3d* colA, PolytopeCollider3d* colB);
	BroadphaseResult3d Collide(PolytopeCollider3d* colA, BoxCollider3d* colB);

	// Capsule/Polytope
	BroadphaseResult3d Collide(CapsuleCollider3d* colA, PolytopeCollider3d* colB);
	BroadphaseResult3d Collide(PolytopeCollider3d* colA, CapsuleCollider3d* colB);
}