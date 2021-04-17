///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 11th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/Entity.h"
#include <vector>

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class CollisionSystem3d;
class ContactManifold3d;
class RigidBody3D;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class PhysicsSystem3D
{
public:
	//-----Public Methods-----

	const RigidBody3D*	AddEntity(Entity* entity);
	void				RemoveEntity(Entity* entity);

	void FrameStep(float deltaSeconds, CollisionSystem3d* collisionSystem = nullptr);


public:
	//-----Public Data-----

	static const float	ALLOWED_PENETRATION;
	static const float	BIAS_FACTOR;
	static const bool	ACCUMULATE_IMPULSES;
	static const bool	WARM_START_ACCUMULATIONS;
	static const Vector3 DEFAULT_GRAVITY_ACC;


private:
	//-----Private Methods-----

	void AddBody(RigidBody3D* body);

	// Called in FrameStep()
	void ApplyForces(float deltaSeconds);
	void CalculateContactImpulses(float deltaSeconds, CollisionSystem3d* collisionSystem);
	void ApplyContactImpulses(float deltaSeconds, CollisionSystem3d* collisionSystem);
	void UpdatePositions(float deltaSeconds);

	void CalculateContactImpulses(float deltaSeconds, ContactManifold3d& manifold);
	void ApplyContactImpulses(float deltaSeconds, ContactManifold3d& manifold);


private:
	//-----Private Data-----

	std::vector<RigidBody3D*>	m_bodies;
	Vector3						m_gravityAcc = DEFAULT_GRAVITY_ACC;

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
