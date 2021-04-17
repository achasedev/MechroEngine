///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 11th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Collision/3D/Collider3d.h"
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Framework/Entity.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Physics/3D/PhysicsSystem3D.h"
#include "Engine/Physics/3D/RigidBody3D.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------
const Vector3 PhysicsSystem3D::DEFAULT_GRAVITY_ACC = Vector3(0.f, 0.f, 0.f);

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
const RigidBody3D* PhysicsSystem3D::AddEntity(Entity* entity)
{
	ASSERT_OR_DIE(entity->m_collider != nullptr, "Rigid bodies need colliders! (for now)");
	
	RigidBody3D* body = new RigidBody3D();
	entity->m_rigidBody = body;
	body->m_owner = entity;
	body->m_transform = &entity->transform;
	body->m_collider = entity->m_collider->GetAsType<PolytopeCollider3d>();

	AddBody(body);
	return body;
}


//-------------------------------------------------------------------------------------------------
void PhysicsSystem3D::FrameStep(float deltaSeconds)
{
	ApplyForces(deltaSeconds);
	//ApplyCollisionContactForces(deltaSeconds);
	UpdatePositions(deltaSeconds);
}


//-------------------------------------------------------------------------------------------------
void PhysicsSystem3D::AddBody(RigidBody3D* body)
{
#ifndef DISABLE_ASSERTS
	// Check for duplicates
	int numBodies = (int)m_bodies.size();
	for (int bodyIndex = 0; bodyIndex < numBodies; ++bodyIndex)
	{
		ASSERT_RETURN(m_bodies[bodyIndex] != body, NO_RETURN_VAL, "Duplicate rigidbody!");
	}
#endif

	m_bodies.push_back(body);
}


//-------------------------------------------------------------------------------------------------
void PhysicsSystem3D::ApplyForces(float deltaSeconds)
{
	uint32 numBodies = (uint32)m_bodies.size();

	for (uint32 bodyIndex = 0; bodyIndex < numBodies; ++bodyIndex)
	{
		RigidBody3D* body = m_bodies[bodyIndex];

		if (body->m_invMass == 0.f)
		{
			continue;
		}

		// Force = Mass * Acceleration :)
		Vector3 gravity = (body->IsAffectedByGravity() ? m_gravityAcc : Vector3::ZERO);
		body->m_velocityWs += (body->m_invMass * body->m_forceWs + gravity) * deltaSeconds;

		Vector3 deltaAngularVelocity;
		deltaAngularVelocity.x = RadiansToDegrees(body->m_invInertia.x * body->m_torque.x * deltaSeconds);
		deltaAngularVelocity.y = RadiansToDegrees(body->m_invInertia.y * body->m_torque.y * deltaSeconds);
		deltaAngularVelocity.z = RadiansToDegrees(body->m_invInertia.z * body->m_torque.z * deltaSeconds);

		body->m_angularVelocityDegrees += deltaAngularVelocity;

		// Zero out forces, they're per-frame
		body->m_forceWs = Vector3::ZERO;
		body->m_torque = Vector3::ZERO;
	}
}


//-------------------------------------------------------------------------------------------------
void PhysicsSystem3D::UpdatePositions(float deltaSeconds)
{
	uint32 numBodies = (uint32)m_bodies.size();

	for (uint32 bodyIndex = 0; bodyIndex < numBodies; ++bodyIndex)
	{
		RigidBody3D* body = m_bodies[bodyIndex];

		// Get the deltas
		// Reminder that transform's position/rotation/scale is defined relative to parent, so work in that space
		Matrix44	worldToParent = body->m_transform->GetWorldToParentMatrix();
		Vector3		deltaPositionWs = body->m_velocityWs * deltaSeconds;
		Vector3		deltaPositionPs = worldToParent.TransformVector(deltaPositionWs).xyz();
		Vector3		deltaRotationDegrees = body->m_angularVelocityDegrees * deltaSeconds;
		Quaternion	deltaRotation = Quaternion::FromEulerAngles(deltaRotationDegrees);

		// Update position in 3 steps:
		// 1. Apply linear velocity (ez)
		// 2. For angular velocity, rotating about the center of mass will also update position if the center of mass and position aren't the same point! So:
		//  a. Translate the object from the position to the center of mass
		//  b. Then translate back along the same vector **transformed by the delta rotation** - to simulate rotating around an axis going through the center of mass
		Vector3		positionPs = body->m_transform->position;
		Vector3		centerOfMassPs = worldToParent.TransformPoint(body->GetCenterOfMassWs()).xyz();
		Vector3		positionToCenterOfMassPs = centerOfMassPs - positionPs;

		// Determine new position after rotating
		body->m_transform->position += positionToCenterOfMassPs;
		body->m_transform->position -= deltaRotation.Rotate(positionToCenterOfMassPs);

		// Apply the delta position from linear movement
		body->m_transform->position += deltaPositionPs;

		// Update rotation, straightforward
		body->m_transform->Rotate(deltaRotation);
	}
}
