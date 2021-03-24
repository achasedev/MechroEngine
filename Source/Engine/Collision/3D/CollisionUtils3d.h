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
struct ContactPoint3D;

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
	BroadphaseResult3d	Collide(SphereCollider3d* colA, SphereCollider3d* colB);
	int					CalculateContacts(SphereCollider3d* colA, SphereCollider3d* colB, const BroadphaseResult3d& broadResult, ContactPoint3D* out_contacts);

	// Box/Box
	BroadphaseResult3d	Collide(BoxCollider3d* colA, BoxCollider3d* colB);
	int					CalculateContacts(BoxCollider3d* colA, BoxCollider3d* colB, const BroadphaseResult3d& broadResult, ContactPoint3D* out_contacts);

	// Capsule/Capsule
	BroadphaseResult3d	Collide(CapsuleCollider3d* colA, CapsuleCollider3d* colB);
	int					CalculateContacts(CapsuleCollider3d* colA, CapsuleCollider3d* colB, const BroadphaseResult3d& broadResult, ContactPoint3D* out_contacts);

	// Polytope/Polytope
	BroadphaseResult3d	Collide(PolytopeCollider3d* colA, PolytopeCollider3d* colB);
	int					CalculateContacts(PolytopeCollider3d* colA, PolytopeCollider3d* colB, const BroadphaseResult3d& broadResult, ContactPoint3D* out_contacts);

	// Sphere/Box
	BroadphaseResult3d	Collide(SphereCollider3d* colA, BoxCollider3d* colB);
	BroadphaseResult3d	Collide(BoxCollider3d* colA, SphereCollider3d* colB);
	int					CalculateContacts(SphereCollider3d* colA, BoxCollider3d* colB, const BroadphaseResult3d& broadResult, ContactPoint3D* out_contacts);
	int					CalculateContacts(BoxCollider3d* colA, SphereCollider3d* colB, const BroadphaseResult3d& broadResult, ContactPoint3D* out_contacts);

	// Sphere/Capsule
	BroadphaseResult3d	Collide(SphereCollider3d* colA, CapsuleCollider3d* colB);
	BroadphaseResult3d	Collide(CapsuleCollider3d* colA, SphereCollider3d* colB);
	int					CalculateContacts(SphereCollider3d* colA, CapsuleCollider3d* colB, const BroadphaseResult3d& broadResult, ContactPoint3D* out_contacts);
	int					CalculateContacts(CapsuleCollider3d* colA, SphereCollider3d* colB, const BroadphaseResult3d& broadResult, ContactPoint3D* out_contacts);

	// Sphere/Polytope
	BroadphaseResult3d	Collide(SphereCollider3d* colA, PolytopeCollider3d* colB);
	BroadphaseResult3d	Collide(PolytopeCollider3d* colA, SphereCollider3d* colB);
	int					CalculateContacts(SphereCollider3d* colA, PolytopeCollider3d* colB, const BroadphaseResult3d& broadResult, ContactPoint3D* out_contacts);
	int					CalculateContacts(PolytopeCollider3d* colA, SphereCollider3d* colB, const BroadphaseResult3d& broadResult, ContactPoint3D* out_contacts);

	// Box/Capsule
	BroadphaseResult3d	Collide(BoxCollider3d* colA, CapsuleCollider3d* colB);
	BroadphaseResult3d	Collide(CapsuleCollider3d* colA, BoxCollider3d* colB);
	int					CalculateContacts(BoxCollider3d* colA, CapsuleCollider3d* colB, const BroadphaseResult3d& broadResult, ContactPoint3D* out_contacts);
	int					CalculateContacts(CapsuleCollider3d* colA, BoxCollider3d* colB, const BroadphaseResult3d& broadResult, ContactPoint3D* out_contacts);

	// Box/Polytope
	BroadphaseResult3d	Collide(BoxCollider3d* colA, PolytopeCollider3d* colB);
	BroadphaseResult3d	Collide(PolytopeCollider3d* colA, BoxCollider3d* colB);
	int					CalculateContacts(BoxCollider3d* colA, PolytopeCollider3d* colB, const BroadphaseResult3d& broadResult, ContactPoint3D* out_contacts);
	int					CalculateContacts(PolytopeCollider3d* colA, BoxCollider3d* colB, const BroadphaseResult3d& broadResult, ContactPoint3D* out_contacts);

	// Capsule/Polytope
	BroadphaseResult3d	Collide(CapsuleCollider3d* colA, PolytopeCollider3d* colB);
	BroadphaseResult3d	Collide(PolytopeCollider3d* colA, CapsuleCollider3d* colB);
	int					CalculateContacts(CapsuleCollider3d* colA, PolytopeCollider3d* colB, const BroadphaseResult3d& broadResult, ContactPoint3D* out_contacts);
	int					CalculateContacts(PolytopeCollider3d* colA, CapsuleCollider3d* colB, const BroadphaseResult3d& broadResult, ContactPoint3D* out_contacts);

}