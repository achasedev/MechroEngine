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
const float PhysicsSystem3D::ALLOWED_PENETRATION = 0.01f;
const float PhysicsSystem3D::BIAS_FACTOR = 0.0f; // Always do position correction for now?
const Vector3 PhysicsSystem3D::DEFAULT_GRAVITY_ACC = Vector3(0.f, -3.f, 0.f);

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
void PhysicsSystem3D::FrameStep(float deltaSeconds,	CollisionSystem3d* collisionSystem /* = nullptr*/)
{
	ApplyForces(deltaSeconds);

	if (collisionSystem != nullptr)
	{
		CalculateContactImpulses(deltaSeconds, collisionSystem);
		ApplyContactImpulses(deltaSeconds, collisionSystem);
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
void PhysicsSystem3D::CalculateContactImpulses(float deltaSeconds, CollisionSystem3d* collisionSystem /* = nullptr*/)
{
	Manifold3dIter itr = collisionSystem->m_manifolds.begin();

	for (itr; itr != collisionSystem->m_manifolds.end(); itr++)
	{
		CalculateContactImpulses(deltaSeconds, itr->second);
	}
}


//-------------------------------------------------------------------------------------------------
void PhysicsSystem3D::CalculateContactImpulses(float deltaSeconds, ContactManifold3d& manifold)
{
	RigidBody3D* body1 = manifold.GetReferenceEntity()->GetRigidBody();
	RigidBody3D* body2 = manifold.GetIncidentEntity()->GetRigidBody();

	ContactPoint3D* contacts = manifold.GetContacts();

	float invDeltaSeconds = (deltaSeconds > 0.f ? 1.0f / deltaSeconds : 0.f);

	for (int contactIndex = 0; contactIndex < manifold.GetNumContacts(); ++contactIndex)
	{
		ContactPoint3D& contact = contacts[contactIndex];

		// Compute vectors from center of mass to contact
		contact.m_r1 = contact.m_position - body1->GetCenterOfMassWs();
		contact.m_r2 = contact.m_position - body2->GetCenterOfMassWs();

		// Precompute normal mass, tangent mass, and bias
		// Mass normal is used to calculate impulse necessary to prevent penetration
		{
			float r1Normal = DotProduct(contact.m_r1, contact.m_normal);
			float r2Normal = DotProduct(contact.m_r2, contact.m_normal);

			Vector3 kNormal = Vector3(body1->m_invMass + body2->m_invMass);
			kNormal.x += body1->m_invInertia.x * (DotProduct(contact.m_r1, contact.m_r1) - r1Normal * r1Normal) + body2->m_invInertia.x * (DotProduct(contact.m_r2, contact.m_r2) - r2Normal * r2Normal);
			kNormal.y += body1->m_invInertia.y * (DotProduct(contact.m_r1, contact.m_r1) - r1Normal * r1Normal) + body2->m_invInertia.y * (DotProduct(contact.m_r2, contact.m_r2) - r2Normal * r2Normal);
			kNormal.z += body1->m_invInertia.z * (DotProduct(contact.m_r1, contact.m_r1) - r1Normal * r1Normal) + body2->m_invInertia.z * (DotProduct(contact.m_r2, contact.m_r2) - r2Normal * r2Normal);

			ASSERT_OR_DIE(!AreMostlyEqual(kNormal.x, 0.f), "oof");
			ASSERT_OR_DIE(!AreMostlyEqual(kNormal.y, 0.f), "oof");
			ASSERT_OR_DIE(!AreMostlyEqual(kNormal.z, 0.f), "oof");
			//contact.m_massNormal = (1.0f / kNormal.x) + (1.0f / kNormal.y) + (1.0f / kNormal.z);
			contact.m_massNormal = 1.0f / (kNormal.x + kNormal.y + kNormal.z);
		}

		// Mass tangent and mass bitangent is used to calculate impulses to simulate friction
		Vector3 crossReference = (!AreMostlyEqual(Abs(DotProduct(contact.m_normal, Vector3::Y_AXIS)), 1.0f) ? Vector3::Y_AXIS : Vector3::X_AXIS);
		Vector3 tangent = CrossProduct(crossReference, contact.m_normal);
		{
			float r1Tangent = DotProduct(contact.m_r1, tangent);
			float r2Tangent = DotProduct(contact.m_r2, tangent);

			Vector3 kTangent = Vector3(body1->m_invMass + body2->m_invMass);
			kTangent.x += body1->m_invInertia.x * (DotProduct(contact.m_r1, contact.m_r1) - r1Tangent * r1Tangent) + body2->m_invInertia.x * (DotProduct(contact.m_r2, contact.m_r2) - r2Tangent * r2Tangent);
			kTangent.y += body1->m_invInertia.y * (DotProduct(contact.m_r1, contact.m_r1) - r1Tangent * r1Tangent) + body2->m_invInertia.y * (DotProduct(contact.m_r2, contact.m_r2) - r2Tangent * r2Tangent);
			kTangent.z += body1->m_invInertia.z * (DotProduct(contact.m_r1, contact.m_r1) - r1Tangent * r1Tangent) + body2->m_invInertia.z * (DotProduct(contact.m_r2, contact.m_r2) - r2Tangent * r2Tangent);

			ASSERT_OR_DIE(!AreMostlyEqual(kTangent.x, 0.f), "oof");
			ASSERT_OR_DIE(!AreMostlyEqual(kTangent.y, 0.f), "oof");
			ASSERT_OR_DIE(!AreMostlyEqual(kTangent.z, 0.f), "oof");

			//contact.m_massTangent = (1.0f / kTangent.x) + (1.0f / kTangent.y) + (1.0f / kTangent.z);
			contact.m_massTangent = 1.0f / (kTangent.x + kTangent.y + kTangent.z);
		}

		{
			Vector3 bitangent = CrossProduct(contact.m_normal, tangent);
			float r1Bitangent = DotProduct(contact.m_r1, bitangent);
			float r2Bitangent = DotProduct(contact.m_r2, bitangent);

			Vector3 kBitangent = Vector3(body1->m_invMass + body2->m_invMass);
			kBitangent.x += body1->m_invInertia.x * (DotProduct(contact.m_r1, contact.m_r1) - r1Bitangent * r1Bitangent) + body2->m_invInertia.x * (DotProduct(contact.m_r2, contact.m_r2) - r2Bitangent * r2Bitangent);
			kBitangent.y += body1->m_invInertia.y * (DotProduct(contact.m_r1, contact.m_r1) - r1Bitangent * r1Bitangent) + body2->m_invInertia.y * (DotProduct(contact.m_r2, contact.m_r2) - r2Bitangent * r2Bitangent);
			kBitangent.z += body1->m_invInertia.z * (DotProduct(contact.m_r1, contact.m_r1) - r1Bitangent * r1Bitangent) + body2->m_invInertia.z * (DotProduct(contact.m_r2, contact.m_r2) - r2Bitangent * r2Bitangent);

			ASSERT_OR_DIE(!AreMostlyEqual(kBitangent.x, 0.f), "oof");
			ASSERT_OR_DIE(!AreMostlyEqual(kBitangent.y, 0.f), "oof");
			ASSERT_OR_DIE(!AreMostlyEqual(kBitangent.z, 0.f), "oof");

			//contact.m_massBitangent = (1.0f / bitangent.x) + (1.0f / bitangent.y) + (1.0f / bitangent.z);
			contact.m_massBitangent = 1.0f / (bitangent.x + bitangent.y + bitangent.z);
		}

		// To quote Erin Catto, this gives the normal impulse "some extra oomph"
		// Proportional to the penetration, so if two objects are really intersecting -> greater bias -> greater normal force -> larger correction this frame
		// Allowed penetration means this will correct over time, not instantaneously - make it less jittery?
		contact.m_bias = -BIAS_FACTOR * invDeltaSeconds * Min(contact.m_pen + ALLOWED_PENETRATION, 0.f); // separation is *always* negative, it's distance below the reference edge
	}
}


//-------------------------------------------------------------------------------------------------
void PhysicsSystem3D::ApplyContactImpulses(float deltaSeconds, CollisionSystem3d* collisionSystem)
{
	Manifold3dIter itr = collisionSystem->m_manifolds.begin();

	for (itr; itr != collisionSystem->m_manifolds.end(); itr++)
	{
		ApplyContactImpulses(deltaSeconds, itr->second);
	}
}


//-------------------------------------------------------------------------------------------------
void PhysicsSystem3D::ApplyContactImpulses(float deltaSeconds, ContactManifold3d& manifold)
{
	RigidBody3D* body1 = manifold.GetReferenceEntity()->GetRigidBody();
	RigidBody3D* body2 = manifold.GetIncidentEntity()->GetRigidBody();

	ContactPoint3D* contacts = manifold.GetContacts();

	for (int contactIndex = 0; contactIndex < manifold.GetNumContacts(); ++contactIndex)
	{
		ContactPoint3D& contact = contacts[contactIndex];

		// Find how angular velocities will affect our point's velocity
		Vector3 angularContribution1FromX = DegreesToRadians(body1->m_angularVelocityDegrees.x) * CrossProduct(contact.m_r1, Vector3::X_AXIS);
		Vector3 angularContribution1FromY = DegreesToRadians(body1->m_angularVelocityDegrees.y) * CrossProduct(contact.m_r1, Vector3::Y_AXIS);
		Vector3 angularContribution1FromZ = DegreesToRadians(body1->m_angularVelocityDegrees.z) * CrossProduct(contact.m_r1, Vector3::Z_AXIS);

		Vector3 angularContribution2FromX = DegreesToRadians(body2->m_angularVelocityDegrees.x) * CrossProduct(contact.m_r2, Vector3::X_AXIS);
		Vector3 angularContribution2FromY = DegreesToRadians(body2->m_angularVelocityDegrees.y) * CrossProduct(contact.m_r2, Vector3::Y_AXIS);
		Vector3 angularContribution2FromZ = DegreesToRadians(body2->m_angularVelocityDegrees.z) * CrossProduct(contact.m_r2, Vector3::Z_AXIS);

		// Relative velocity at contact from body 1's point of view
		Vector3 relativeVelocity = body2->m_velocityWs + angularContribution2FromX + angularContribution2FromY + angularContribution2FromZ - (body1->m_velocityWs + angularContribution1FromX + angularContribution1FromY + angularContribution1FromZ);

		// Compute normal impulse
		float speedAlongNormal = DotProduct(relativeVelocity, contact.m_normal);
		float normalImpulseMagnitude = contact.m_massNormal * (-1.0f * speedAlongNormal + contact.m_bias); // Impulse goes against the velocity to correct, add a little bias for oomph
		normalImpulseMagnitude = Max(normalImpulseMagnitude, 0.f);
	
		// Apply normal impulse
		Vector3 normalImpulse = normalImpulseMagnitude * contact.m_normal;

		Vector3 pos = body1->m_transform->GetWorldPosition();
		Vector3 centerOfMass = body1->GetCenterOfMassWs();

		body1->m_velocityWs -= body1->m_invMass * normalImpulse;

		if (isnan(body1->m_velocityWs.x))
		{
			int x = 7;
			x = 4;
		}

		body1->m_angularVelocityDegrees.x -= RadiansToDegrees(body1->m_invInertia.x * DotProduct(normalImpulse, CrossProduct(contact.m_r1, Vector3::X_AXIS)));
		body1->m_angularVelocityDegrees.y -= RadiansToDegrees(body1->m_invInertia.y * DotProduct(normalImpulse, CrossProduct(contact.m_r1, Vector3::Y_AXIS)));
		body1->m_angularVelocityDegrees.z -= RadiansToDegrees(body1->m_invInertia.z * DotProduct(normalImpulse, CrossProduct(contact.m_r1, Vector3::Z_AXIS)));

		body2->m_velocityWs += body2->m_invMass * normalImpulse;

		if (isnan(body2->m_velocityWs.x))
		{
			int x = 7;
			x = 4;
		}

		body2->m_angularVelocityDegrees.x += RadiansToDegrees(body2->m_invInertia.x * DotProduct(normalImpulse, CrossProduct(contact.m_r2, Vector3::X_AXIS)));
		body2->m_angularVelocityDegrees.y += RadiansToDegrees(body2->m_invInertia.y * DotProduct(normalImpulse, CrossProduct(contact.m_r2, Vector3::Y_AXIS)));
		body2->m_angularVelocityDegrees.z += RadiansToDegrees(body2->m_invInertia.z * DotProduct(normalImpulse, CrossProduct(contact.m_r2, Vector3::Z_AXIS)));


		// Recalculate the relative velocity
		angularContribution1FromX = DegreesToRadians(body1->m_angularVelocityDegrees.x) * CrossProduct(contact.m_r1, Vector3::X_AXIS);
		angularContribution1FromY = DegreesToRadians(body1->m_angularVelocityDegrees.y) * CrossProduct(contact.m_r1, Vector3::Y_AXIS);
		angularContribution1FromZ = DegreesToRadians(body1->m_angularVelocityDegrees.z) * CrossProduct(contact.m_r1, Vector3::Z_AXIS);

		angularContribution2FromX = DegreesToRadians(body2->m_angularVelocityDegrees.x) * CrossProduct(contact.m_r2, Vector3::X_AXIS);
		angularContribution2FromY = DegreesToRadians(body2->m_angularVelocityDegrees.y) * CrossProduct(contact.m_r2, Vector3::Y_AXIS);
		angularContribution2FromZ = DegreesToRadians(body2->m_angularVelocityDegrees.z) * CrossProduct(contact.m_r2, Vector3::Z_AXIS);

		// Relative velocity at contact from body 1's point of view
		relativeVelocity = body2->m_velocityWs + angularContribution2FromX + angularContribution2FromY + angularContribution2FromZ - (body1->m_velocityWs + angularContribution1FromX + angularContribution1FromY + angularContribution1FromZ);


		float friction = Sqrt(body1->GetFriction() + body2->GetFriction());


		// Compute tangent impulse
		Vector3 crossReference = (!AreMostlyEqual(Abs(DotProduct(contact.m_normal, Vector3::Y_AXIS)), 1.0f) ? Vector3::Y_AXIS : Vector3::X_AXIS);
		Vector3 tangent = CrossProduct(crossReference, contact.m_normal);
		{
			float speedAlongTangent = DotProduct(relativeVelocity, tangent);
			float tangentImpulseMagnitude = contact.m_massTangent * (-speedAlongTangent); // Friction opposes movement

			// Factor in friction coefficient
			float maxTangentImpulseMag = friction * normalImpulseMagnitude; // Always >= 0.f
			tangentImpulseMagnitude = Clamp(tangentImpulseMagnitude, -maxTangentImpulseMag, maxTangentImpulseMag);

			// Apply the tangent impulse
			Vector3 tangentImpulse = tangentImpulseMagnitude * tangent;

			body1->m_velocityWs -= body1->m_invMass * tangentImpulse;

			if (isnan(body1->m_velocityWs.x))
			{
				int x = 7;
				x = 4;
			}

			body1->m_angularVelocityDegrees.x -= RadiansToDegrees(body1->m_invInertia.x * DotProduct(tangentImpulse, CrossProduct(contact.m_r1, Vector3::X_AXIS)));
			body1->m_angularVelocityDegrees.y -= RadiansToDegrees(body1->m_invInertia.y * DotProduct(tangentImpulse, CrossProduct(contact.m_r1, Vector3::Y_AXIS)));
			body1->m_angularVelocityDegrees.z -= RadiansToDegrees(body1->m_invInertia.z * DotProduct(tangentImpulse, CrossProduct(contact.m_r1, Vector3::Z_AXIS)));

			body2->m_velocityWs += body2->m_invMass * tangentImpulse;

			if (isnan(body2->m_velocityWs.x))
			{
				int x = 7;
				x = 4;
			}

			body2->m_angularVelocityDegrees.x += RadiansToDegrees(body2->m_invInertia.x * DotProduct(tangentImpulse, CrossProduct(contact.m_r2, Vector3::X_AXIS)));
			body2->m_angularVelocityDegrees.y += RadiansToDegrees(body2->m_invInertia.y * DotProduct(tangentImpulse, CrossProduct(contact.m_r2, Vector3::Y_AXIS)));
			body2->m_angularVelocityDegrees.z += RadiansToDegrees(body2->m_invInertia.z * DotProduct(tangentImpulse, CrossProduct(contact.m_r2, Vector3::Z_AXIS)));
		}

		// Compute bitangent impulse
		{
			Vector3 bitangent = CrossProduct(contact.m_normal, tangent);
			float speedAlongBitangent = DotProduct(relativeVelocity, bitangent);
			float bitangentImpulseMagnitude = contact.m_massBitangent * (-speedAlongBitangent); // Friction opposes movement

			// Factor in friction coefficient
			float maxBitangentImpulseMag = friction * normalImpulseMagnitude; // Always >= 0.f
			bitangentImpulseMagnitude = Clamp(bitangentImpulseMagnitude, -maxBitangentImpulseMag, maxBitangentImpulseMag);

			// Apply the bitangent impulse
			Vector3 bitangentImpulse = bitangentImpulseMagnitude * bitangent;

			body1->m_velocityWs -= body1->m_invMass * bitangentImpulse;

			if (isnan(body1->m_velocityWs.x))
			{
				int x = 7;
				x = 4;
			}

			body1->m_angularVelocityDegrees.x -= RadiansToDegrees(body1->m_invInertia.x * DotProduct(bitangentImpulse, CrossProduct(contact.m_r1, Vector3::X_AXIS)));
			body1->m_angularVelocityDegrees.y -= RadiansToDegrees(body1->m_invInertia.y * DotProduct(bitangentImpulse, CrossProduct(contact.m_r1, Vector3::Y_AXIS)));
			body1->m_angularVelocityDegrees.z -= RadiansToDegrees(body1->m_invInertia.z * DotProduct(bitangentImpulse, CrossProduct(contact.m_r1, Vector3::Z_AXIS)));

			body2->m_velocityWs += body2->m_invMass * bitangentImpulse;

			if (isnan(body2->m_velocityWs.x))
			{
				int x = 7;
				x = 4;
			}

			body2->m_angularVelocityDegrees.x += RadiansToDegrees(body2->m_invInertia.x * DotProduct(bitangentImpulse, CrossProduct(contact.m_r2, Vector3::X_AXIS)));
			body2->m_angularVelocityDegrees.y += RadiansToDegrees(body2->m_invInertia.y * DotProduct(bitangentImpulse, CrossProduct(contact.m_r2, Vector3::Y_AXIS)));
			body2->m_angularVelocityDegrees.z += RadiansToDegrees(body2->m_invInertia.z * DotProduct(bitangentImpulse, CrossProduct(contact.m_r2, Vector3::Z_AXIS)));
		}
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
