///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 11th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Collision/3D/Collider3d.h"
#include "Engine/Collision/3D/CollisionSystem3d.h"
#include "Engine/Collision/3D/ContactManifold3d.h"
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Framework/Entity.h"
#include "Engine/Framework/Rgba.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Physics/3D/PhysicsSystem3D.h"
#include "Engine/Physics/3D/RigidBody3D.h"
#include "Engine/Render/Debug/DebugRenderSystem.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------
const float PhysicsSystem3D::ALLOWED_PENETRATION = 0.01f;
const float PhysicsSystem3D::BIAS_FACTOR = 0.2f;	
const float PhysicsSystem3D::DEFAULT_TIMESTEP = (1.f / 60.f);
const bool PhysicsSystem3D::ACCUMULATE_IMPULSES = true;
const bool PhysicsSystem3D::WARM_START_ACCUMULATIONS = true;
const Vector3 PhysicsSystem3D::DEFAULT_GRAVITY_ACC = Vector3(0.f, -9.8f, 0.f);

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
PhysicsSystem3D::PhysicsSystem3D()
{
	SetTimeStep(DEFAULT_TIMESTEP);
}


//-------------------------------------------------------------------------------------------------
void PhysicsSystem3D::Update(CollisionSystem3d* collisionSystem /*= nullptr*/)
{
	if (m_stepTimer.HasIntervalElapsed())
	{
		int numElapses = m_stepTimer.DecrementByIntervalAll();
		float deltaSeconds = m_stepTimer.GetIntervalSeconds() * numElapses;
		DoPhysicsStep(deltaSeconds, collisionSystem);
	}
}


//-------------------------------------------------------------------------------------------------
void PhysicsSystem3D::SetTimeStep(float stepSeconds)
{
	m_stepTimer.SetInterval(stepSeconds);
}


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
void PhysicsSystem3D::DoPhysicsStep(float deltaSeconds,	CollisionSystem3d* collisionSystem /* = nullptr*/)
{
	ApplyForces(deltaSeconds);

	if (collisionSystem != nullptr)
	{
		CalculateEffectiveMasses(deltaSeconds, collisionSystem);
		ApplyContactImpulses(collisionSystem);
	}

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
		body->m_velocityWs += (body->m_invMass * (body->m_forceWs + gravity)) * deltaSeconds;
		body->m_angularVelocityDegrees += RadiansToDegrees(body->m_invInertia * body->m_torque * deltaSeconds);

		// Zero out forces, they're per-frame
		body->m_forceWs = Vector3::ZERO;
		body->m_torque = Vector3::ZERO;
	}
}


//-------------------------------------------------------------------------------------------------
void PhysicsSystem3D::CalculateEffectiveMasses(float deltaSeconds, CollisionSystem3d* collisionSystem /* = nullptr*/)
{
	Manifold3dIter itr = collisionSystem->m_manifolds.begin();

	for (itr; itr != collisionSystem->m_manifolds.end(); itr++)
	{
		CalculateEffectiveMasses(deltaSeconds, itr->second);
	}
}


//-------------------------------------------------------------------------------------------------
void PhysicsSystem3D::CalculateEffectiveMasses(float deltaSeconds, ContactManifold3d& manifold)
{
	RigidBody3D* body1 = manifold.GetReferenceEntity()->GetRigidBody();
	RigidBody3D* body2 = manifold.GetIncidentEntity()->GetRigidBody();

	ContactPoint3D* contacts = manifold.GetContacts();

	float invDeltaSeconds = (deltaSeconds > 0.f ? (1.0f / deltaSeconds) : 0.f);

	for (int contactIndex = 0; contactIndex < manifold.GetNumContacts(); ++contactIndex)
	{
		ContactPoint3D& contact = contacts[contactIndex];

		// Compute vectors from center of mass to contact
		contact.m_r1 = contact.m_position - body1->GetCenterOfMassWs();
		contact.m_r2 = contact.m_position - body2->GetCenterOfMassWs();

		// Precompute normal mass, tangent mass, and bias
		// Mass normal is used to calculate impulse necessary to prevent penetration
		{
			// me = invM1 + invM2 + dot(r1 x n, invI1 * (r1 x n)) + dot(r2 x n, invI2 * (r2 x n))
			Vector3 kNormal = Vector3(body1->m_invMass + body2->m_invMass);

			kNormal.x += DotProduct(CrossProduct(contact.m_r1, contact.m_normal), body1->m_invInertia.x * CrossProduct(contact.m_r1, contact.m_normal)) + DotProduct(CrossProduct(contact.m_r2, contact.m_normal), body2->m_invInertia.x * CrossProduct(contact.m_r2, contact.m_normal));
			kNormal.y += DotProduct(CrossProduct(contact.m_r1, contact.m_normal), body1->m_invInertia.y * CrossProduct(contact.m_r1, contact.m_normal)) + DotProduct(CrossProduct(contact.m_r2, contact.m_normal), body2->m_invInertia.y * CrossProduct(contact.m_r2, contact.m_normal));
			kNormal.z += DotProduct(CrossProduct(contact.m_r1, contact.m_normal), body1->m_invInertia.z * CrossProduct(contact.m_r1, contact.m_normal)) + DotProduct(CrossProduct(contact.m_r2, contact.m_normal), body2->m_invInertia.z * CrossProduct(contact.m_r2, contact.m_normal));	

			contact.m_massNormal = 1.0f / (kNormal.x + kNormal.y + kNormal.z);
		}

		// Mass tangent and mass bitangent is used to calculate impulses to simulate friction
		Vector3 crossReference = (!AreMostlyEqual(Abs(DotProduct(contact.m_normal, Vector3::Y_AXIS)), 1.0f) ? Vector3::Y_AXIS : Vector3::X_AXIS);
		Vector3 tangent = CrossProduct(crossReference, contact.m_normal);
		{
			Vector3 kTangent = Vector3(body1->m_invMass + body2->m_invMass);

			// me = invM1 + invM2 + dot(r1 x n, invI1 * (r1 x n)) + dot(r2 x n, invI2 * (r2 x n))
			kTangent.x += DotProduct(CrossProduct(contact.m_r1, tangent), body1->m_invInertia.x * CrossProduct(contact.m_r1, tangent)) + DotProduct(CrossProduct(contact.m_r2, tangent), body2->m_invInertia.x * CrossProduct(contact.m_r2, tangent));
			kTangent.y += DotProduct(CrossProduct(contact.m_r1, tangent), body1->m_invInertia.y * CrossProduct(contact.m_r1, tangent)) + DotProduct(CrossProduct(contact.m_r2, tangent), body2->m_invInertia.y * CrossProduct(contact.m_r2, tangent));
			kTangent.z += DotProduct(CrossProduct(contact.m_r1, tangent), body1->m_invInertia.z * CrossProduct(contact.m_r1, tangent)) + DotProduct(CrossProduct(contact.m_r2, tangent), body2->m_invInertia.z * CrossProduct(contact.m_r2, tangent));

			contact.m_massTangent = 1.0f / (kTangent.x + kTangent.y + kTangent.z);
		}

		Vector3 bitangent = CrossProduct(contact.m_normal, tangent);
		{

			Vector3 kBitangent = Vector3(body1->m_invMass + body2->m_invMass);
			// me = invM1 + invM2 + dot(r1 x n, invI1 * (r1 x n)) + dot(r2 x n, invI2 * (r2 x n))
			kBitangent.x += DotProduct(CrossProduct(contact.m_r1, bitangent), body1->m_invInertia.x * CrossProduct(contact.m_r1, bitangent)) + DotProduct(CrossProduct(contact.m_r2, bitangent), body2->m_invInertia.x * CrossProduct(contact.m_r2, bitangent));
			kBitangent.y += DotProduct(CrossProduct(contact.m_r1, bitangent), body1->m_invInertia.y * CrossProduct(contact.m_r1, bitangent)) + DotProduct(CrossProduct(contact.m_r2, bitangent), body2->m_invInertia.y * CrossProduct(contact.m_r2, bitangent));
			kBitangent.z += DotProduct(CrossProduct(contact.m_r1, bitangent), body1->m_invInertia.z * CrossProduct(contact.m_r1, bitangent)) + DotProduct(CrossProduct(contact.m_r2, bitangent), body2->m_invInertia.z * CrossProduct(contact.m_r2, bitangent));

			contact.m_massBitangent = 1.0f / (kBitangent.x + kBitangent.y + kBitangent.z);
		}

		// To quote Erin Catto, this gives the normal impulse "some extra oomph"
		// Proportional to the penetration, so if two objects are really intersecting -> greater bias -> greater normal force -> larger correction this frame
		// Allowed penetration means this will correct over time, not instantaneously - make it less jittery?
		contact.m_bias = -BIAS_FACTOR * invDeltaSeconds * Min(contact.m_pen + ALLOWED_PENETRATION, 0.f); // separation is *always* negative, it's distance below the reference edge

		// If accumulating impulses, apply them here while we still have the tangent and bitangent
		if (ACCUMULATE_IMPULSES)
		{
			Vector3 impulse = contact.m_accNormalImpulse * contact.m_normal + contact.m_accTangentImpulse * tangent + contact.m_accBitangentImpulse * bitangent;

			// Q. But friction impulse should be in some way related to relative velocity! Yet there's no velocity here!
			// A. These incrementals were calculated last frame in ApplyImpulse(), so unless velocity instantaneous and largely changed outside
			//    the physics system this warm start should feel "continuous" from where it just left off
			body1->m_velocityWs -= body1->m_invMass * impulse;
			body1->m_angularVelocityDegrees -= RadiansToDegrees(body1->m_invInertia * CrossProduct(contact.m_r1, impulse));

			body2->m_velocityWs += body2->m_invMass * impulse;
			body2->m_angularVelocityDegrees += RadiansToDegrees(body2->m_invInertia * CrossProduct(contact.m_r2, impulse));
		}
	}
}


//-------------------------------------------------------------------------------------------------
void PhysicsSystem3D::ApplyContactImpulses(CollisionSystem3d* collisionSystem)
{
	Manifold3dIter itr = collisionSystem->m_manifolds.begin();

	for (itr; itr != collisionSystem->m_manifolds.end(); itr++)
	{
		ApplyContactImpulses(itr->second);
	}
}


//-------------------------------------------------------------------------------------------------
void PhysicsSystem3D::ApplyContactImpulses(ContactManifold3d& manifold)
{
	RigidBody3D* body1 = manifold.GetReferenceEntity()->GetRigidBody();
	RigidBody3D* body2 = manifold.GetIncidentEntity()->GetRigidBody();

	ContactPoint3D* contacts = manifold.GetContacts();

	for (int contactIndex = 0; contactIndex < manifold.GetNumContacts(); ++contactIndex)
	{
		ContactPoint3D& contact = contacts[contactIndex];

		// Find how angular velocities will affect our point's velocity
		// Relative velocity at contact from body 1's point of view
		// relV = v2 + w2 x r2 - v1 - w1 x r1
		Vector3 relativeVelocity1 = body2->m_velocityWs + CrossProduct(DegreesToRadians(body2->m_angularVelocityDegrees), contact.m_r2) - body1->m_velocityWs - CrossProduct(DegreesToRadians(body1->m_angularVelocityDegrees), contact.m_r1);

		// Compute normal impulse
		float speedAlongNormal = DotProduct(relativeVelocity1, contact.m_normal);
		float normalImpulseMagnitude = contact.m_massNormal * (-1.0f * speedAlongNormal + contact.m_bias); // Impulse goes against the velocity to correct, add a little bias for oomph
		
		if (ACCUMULATE_IMPULSES)
		{
			// Clamp the accumulated impulse
			float oldAccNormalImpulse = contact.m_accNormalImpulse;
			contact.m_accNormalImpulse = Max(oldAccNormalImpulse + normalImpulseMagnitude, 0.f);
			normalImpulseMagnitude = Max(contact.m_accNormalImpulse - oldAccNormalImpulse, 0.f);
		}
		else
		{
			normalImpulseMagnitude = Max(normalImpulseMagnitude, 0.f);
		}
	
		// Apply normal impulse
		Vector3 normalImpulse = normalImpulseMagnitude * contact.m_normal;

		body1->m_velocityWs -= body1->m_invMass * normalImpulse;
		Vector3 deltaAnglesN1 = RadiansToDegrees(body1->m_invInertia * CrossProduct(contact.m_r1, normalImpulse));
		body1->m_angularVelocityDegrees -= deltaAnglesN1;

		body2->m_velocityWs += body2->m_invMass * normalImpulse;
		Vector3 deltaAnglesN2 = RadiansToDegrees(body2->m_invInertia * CrossProduct(contact.m_r2, normalImpulse));
		body2->m_angularVelocityDegrees += deltaAnglesN2;

		// Recalculate the relative velocity
		Vector3 relativeVelocity = body2->m_velocityWs + CrossProduct(DegreesToRadians(body2->m_angularVelocityDegrees), contact.m_r2) - body1->m_velocityWs - CrossProduct(DegreesToRadians(body1->m_angularVelocityDegrees), contact.m_r1);

		float friction = Sqrt(body1->GetFriction() + body2->GetFriction());

		// Compute tangent impulse
		Vector3 crossReference = (!AreMostlyEqual(Abs(DotProduct(contact.m_normal, Vector3::Y_AXIS)), 1.0f) ? Vector3::Y_AXIS : Vector3::X_AXIS);
		Vector3 tangent = CrossProduct(crossReference, contact.m_normal);
		
		float speedAlongTangent = DotProduct(relativeVelocity, tangent);
		float tangentImpulseMagnitude = contact.m_massTangent * (-speedAlongTangent); // Friction opposes movement

		if (ACCUMULATE_IMPULSES)
		{
			// Factor in friction coefficient
			float maxTangentImpulseMag = friction * contact.m_accNormalImpulse; // Always >= 0.f

			// Clamp friction
			float oldTangentImpulse = contact.m_accTangentImpulse;
			contact.m_accTangentImpulse = Clamp(oldTangentImpulse + tangentImpulseMagnitude, -maxTangentImpulseMag, maxTangentImpulseMag);
			tangentImpulseMagnitude = contact.m_accTangentImpulse - oldTangentImpulse;
		}
		else
		{
			// Factor in friction coefficient
			float maxTangentImpulseMag = friction * normalImpulseMagnitude; // Always >= 0.f
			tangentImpulseMagnitude = Clamp(tangentImpulseMagnitude, -maxTangentImpulseMag, maxTangentImpulseMag);
		}		

		// Apply the tangent impulse
		Vector3 tangentImpulse = tangentImpulseMagnitude * tangent;
		body1->m_velocityWs -= body1->m_invMass * tangentImpulse;
		Vector3 deltaAnglesT1 = RadiansToDegrees(body1->m_invInertia * CrossProduct(contact.m_r1, tangentImpulse));
		body1->m_angularVelocityDegrees -= deltaAnglesT1;
		body2->m_velocityWs += body2->m_invMass * tangentImpulse;
		Vector3 deltaAnglesT2 = RadiansToDegrees(body2->m_invInertia * CrossProduct(contact.m_r2, tangentImpulse));
		body2->m_angularVelocityDegrees += deltaAnglesT2;

		// Recalculate the relative velocity
		relativeVelocity = body2->m_velocityWs + CrossProduct(DegreesToRadians(body2->m_angularVelocityDegrees), contact.m_r2) - body1->m_velocityWs - CrossProduct(DegreesToRadians(body1->m_angularVelocityDegrees), contact.m_r1);

		// Compute bitangent impulse
		Vector3 bitangent = CrossProduct(contact.m_normal, tangent);
		float speedAlongBitangent = DotProduct(relativeVelocity, bitangent);
		float bitangentImpulseMagnitude = contact.m_massBitangent * (-speedAlongBitangent); // Friction opposes movement

		if (ACCUMULATE_IMPULSES)
		{
			// Factor in friction coefficient
			float maxBitangentImpulseMag = friction * contact.m_accNormalImpulse; // Always >= 0.f

			// Clamp friction
			float oldBitangentImpulse = contact.m_accBitangentImpulse;
			contact.m_accBitangentImpulse = Clamp(oldBitangentImpulse + bitangentImpulseMagnitude, -maxBitangentImpulseMag, maxBitangentImpulseMag);
			bitangentImpulseMagnitude = contact.m_accBitangentImpulse - oldBitangentImpulse;
		}
		else
		{
			// Factor in friction coefficient
			float maxBitangentImpulseMag = friction * normalImpulseMagnitude; // Always >= 0.f
			bitangentImpulseMagnitude = Clamp(bitangentImpulseMagnitude, -maxBitangentImpulseMag, maxBitangentImpulseMag);
		}

		// Apply the bitangent impulse
		Vector3 bitangentImpulse = bitangentImpulseMagnitude * bitangent;
		body1->m_velocityWs -= body1->m_invMass * bitangentImpulse;
		body2->m_velocityWs += body2->m_invMass * bitangentImpulse;
		Vector3 deltaAnglesB1 = RadiansToDegrees(body1->m_invInertia * CrossProduct(contact.m_r1, bitangentImpulse));
		Vector3 deltaAnglesB2 = RadiansToDegrees(body2->m_invInertia * CrossProduct(contact.m_r2, bitangentImpulse));
		body1->m_angularVelocityDegrees -= deltaAnglesB1;
		body2->m_angularVelocityDegrees += deltaAnglesB2;

		DebugDrawLine3D(contact.m_position, contact.m_position + 5.f * normalImpulse, Rgba::BLUE, 1.0f);
		DebugDrawLine3D(contact.m_position, contact.m_position + 5.f * tangentImpulse, Rgba::RED, 1.0f);
		DebugDrawLine3D(contact.m_position, contact.m_position + 5.f * bitangentImpulse, Rgba::GREEN, 1.0f);
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
		Vector3	positionPs = body->m_transform->position;
		Vector3	centerOfMassPs = worldToParent.TransformPoint(body->GetCenterOfMassWs()).xyz();
		if (!AreMostlyEqual(centerOfMassPs, positionPs))
		{
			Vector3	positionToCenterOfMassPs = centerOfMassPs - positionPs;

			// Determine new position after rotating
			body->m_transform->position += positionToCenterOfMassPs;
			body->m_transform->position -= deltaRotation.Rotate(positionToCenterOfMassPs);
		}	

		// Apply the delta position from linear movement
		body->m_transform->position += deltaPositionPs;

		// Update rotation, straightforward
		body->m_transform->Rotate(deltaRotation);
	}
}
