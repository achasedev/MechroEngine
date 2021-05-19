///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: May 14th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Collision/Contact.h"
#include "Engine/Collision/ContactResolver.h"
#include "Engine/Core/EngineCommon.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/Quaternion.h"
#include "Engine/Physics/RigidBody/RigidBody.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
static void PrepareContacts(Contact* contacts, int numContacts)
{
	for (int i = 0; i < numContacts; ++i)
	{
		contacts[i].CalculateInternals();
	}
}


//-------------------------------------------------------------------------------------------------
static void ResolveContactPenetration(Contact* contact, Vector3* out_linearChanges, Vector3* out_angularChanges)
{
	// First calculate inertias
	// These stand for "change in linear velocity along normal per unit of impulse" given this contact's position, normal, etc. Delta position along normal can come from the object moving *and* the object rotating - this is non-linear projection
	// Greater inertia here === more of the correction received. It's not the inertia of the object, but the inertia "gained" from the correction!
	float linearInertia[2]; // Change in linear velocity along normal directly from the impulse. Since the impulse is in the direction of the normal, this just becomes inverse mass. Greater mass === Lesser inverse mass === lesser inertia === less response received
	float angularInertia[2]; // Change in linear velocity along normal from rotation induced from impulse, per unit of impulse. Think of it as "due to the rotation from this impulse, how much would that move me along the normal?"
	float totalInertia = 0.f;
	Vector3 deltaAngularVelocityPerImpulse[2]; // We reuse these values when updating position
	const float angularLimit = 0.2f;

	for (int bodyIndex = 0; bodyIndex < 2; ++bodyIndex)
	{
		RigidBody* body = contact->bodies[bodyIndex];
		if (body == nullptr)
		{
			linearInertia[bodyIndex] = 0.f;
			angularInertia[bodyIndex] = 0.f;
			continue;
		}

		// Calculate angular inertia at the contact
		Matrix3 inverseInertiaTensor;
		body->GetWorldInverseInertiaTensor(inverseInertiaTensor);

		Vector3 torquePerImpulse = CrossProduct(contact->bodyToContact[bodyIndex], contact->normal);
		deltaAngularVelocityPerImpulse[bodyIndex] = inverseInertiaTensor * torquePerImpulse;
		Vector3 linearVelocityPerImpulse = CrossProduct(deltaAngularVelocityPerImpulse[bodyIndex], contact->bodyToContact[bodyIndex]);

		angularInertia[bodyIndex] = DotProduct(linearVelocityPerImpulse, contact->normal); // More resistant object (massive) === less linearVelocityPerImpulse === lesser inertia === less change

		// Linear component is just inverse mass
		linearInertia[bodyIndex] = body->GetInverseMass();

		// Track total inertia for distributing proportional to mass. Greater inertia here means *greater* correction!
		totalInertia += linearInertia[bodyIndex] + angularInertia[bodyIndex];
	}

	// Now calculate the translations of this contact in world space from linear and angular change
	float linearMove[2];
	float angularMove[2];
	float inverseTotalInverseInertia = 1.0f / totalInertia;

	for (int bodyIndex = 0; bodyIndex < 2; ++bodyIndex)
	{
		RigidBody* body = contact->bodies[bodyIndex];
		if (body == nullptr)
		{
			out_linearChanges[bodyIndex] = Vector3::ZERO;
			out_angularChanges[bodyIndex] = Vector3::ZERO;
			continue;
		}

		float sign = (bodyIndex == 0 ? 1.0f : -1.0f);

		// Calculate the amount of movement from the linear component of impulse
		linearMove[bodyIndex] = sign * contact->penetration * linearInertia[bodyIndex] * inverseTotalInverseInertia;

		// To move this contact by the linear amount, we can just add it to the object (linear move of the object linearly moves the contact)
		out_linearChanges[bodyIndex] = contact->normal * linearMove[bodyIndex];

		// For the movement from angular component of impulse, we need to calculate the amount of rotation that would create this much movement
		Matrix3 inverseInertiaTensor;
		body->GetWorldInverseInertiaTensor(inverseInertiaTensor);
		angularMove[bodyIndex] = sign * angularInertia[bodyIndex] * inverseTotalInverseInertia;

		// Limit the amount of movement that comes from angular rotation
		float limit = angularLimit * contact->bodyToContact[bodyIndex].GetLength();
		if (Abs(angularMove[bodyIndex]) > limit)
		{
			float totalMove = angularMove[bodyIndex] + linearMove[bodyIndex];

			if (angularMove[bodyIndex] >= 0)
			{
				angularMove[bodyIndex] = limit;
			}
			else
			{
				angularMove[bodyIndex] = -limit;
			}

			linearMove[bodyIndex] = totalMove - angularMove[bodyIndex];
		}

		Vector3 rotationPerMovement = (deltaAngularVelocityPerImpulse[bodyIndex] / angularInertia[bodyIndex]); // angularInertia === deltaLinearVelocityFromRotationPerUnitImpulse - "Per Impulse" cancels out, so this becomes a ratio of angular change per linear change :)
		out_angularChanges[bodyIndex] = angularMove[bodyIndex] * rotationPerMovement;

		// Now apply delta rotations
		body->transform->position += out_linearChanges[bodyIndex];
		body->transform->rotation *= Quaternion::CreateFromEulerAnglesRadians(out_angularChanges[bodyIndex]);

		// Make sure our inertia tensor in world space is up-to-date
		body->CalculateDerivedData();	
	}
}


//-------------------------------------------------------------------------------------------------
static Vector3 CalculateFrictionlessImpulse(Contact* contact)
{
	// Calculate how much delta velocity is created from 1 unit of impulse along the contact normal
	float deltaVelocityAlongNormalPerUnitOfImpulse = 0.f;

	// First body
	{
		Matrix3 inverseInertiaTensorWs;
		contact->bodies[0]->GetWorldInverseInertiaTensor(inverseInertiaTensorWs);

		Vector3 deltaVelocityWs = CrossProduct(contact->bodyToContact[0], contact->normal);	
		deltaVelocityWs = inverseInertiaTensorWs * deltaVelocityWs;
		deltaVelocityWs = CrossProduct(deltaVelocityWs, contact->bodyToContact[0]);

		// Angular part
		deltaVelocityAlongNormalPerUnitOfImpulse = DotProduct(deltaVelocityWs, contact->normal);

		// Linear part
		deltaVelocityAlongNormalPerUnitOfImpulse += contact->bodies[0]->GetInverseMass();
	}
	
	// Second body
	if (contact->bodies[1] != nullptr)
	{
		Matrix3 inverseInertiaTensorWs;
		contact->bodies[1]->GetWorldInverseInertiaTensor(inverseInertiaTensorWs);

		Vector3 deltaVelocityWs = CrossProduct(contact->bodyToContact[1], contact->normal);
		deltaVelocityWs = inverseInertiaTensorWs * deltaVelocityWs;
		deltaVelocityWs = CrossProduct(deltaVelocityWs, contact->bodyToContact[1]);

		// Angular part
		deltaVelocityAlongNormalPerUnitOfImpulse += DotProduct(deltaVelocityWs, contact->normal);

		// Linear part
		deltaVelocityAlongNormalPerUnitOfImpulse += contact->bodies[1]->GetInverseMass();
	}

	// Then calculate how much impulse we'd need to get our desired velocity along the normal
	return Vector3(contact->desiredDeltaVelocity / deltaVelocityAlongNormalPerUnitOfImpulse, 0.f, 0.f); // X vector in contact space is the normal, 0 out the other directions for no friction
}


//-------------------------------------------------------------------------------------------------
static void ResolveContactVelocity(Contact* contact, Vector3* out_linearDeltaVelocities, Vector3* out_angularDeltaVelocities)
{
	Matrix3 inverseInertiaTensorsWs[2];
	contact->bodies[0]->GetWorldInverseInertiaTensor(inverseInertiaTensorsWs[0]);
	if (contact->bodies[1] != nullptr)
	{
		contact->bodies[1]->GetWorldInverseInertiaTensor(inverseInertiaTensorsWs[1]);
	}

	Vector3 impulseInContactSpace = Vector3::ZERO;

	if (contact->friction == 0.f)
	{
		impulseInContactSpace = CalculateFrictionlessImpulse(contact);
	}
	else
	{
		// TODO: Friction impulse
		ERROR_AND_DIE("No friction!");
	}

	Vector3 impulseWs = contact->contactToWorld * impulseInContactSpace;

	// Calculate first body delta velocities
	{
		Vector3 torqueWs = CrossProduct(contact->bodyToContact[0], impulseWs);

		// Calculate changes
		out_linearDeltaVelocities[0] = impulseWs * contact->bodies[0]->GetInverseMass();
		out_angularDeltaVelocities[0] = inverseInertiaTensorsWs[0] * torqueWs;

		// Apply them
		contact->bodies[0]->AddWorldVelocity(out_linearDeltaVelocities[0]);
		contact->bodies[0]->AddWorldAngularVelocityRadians(out_angularDeltaVelocities[0]);
	}

	// Calculate second bodies velocities
	if (contact->bodies[1] != nullptr)
	{
		Vector3 torqueWs = CrossProduct(impulseWs, contact->bodyToContact[1]); // Switched cross, since torque would be in opposite direction

		// Calculate changes
		out_linearDeltaVelocities[1] = -1.0f * impulseWs * contact->bodies[1]->GetInverseMass(); // Velocity change would be opposite the first
		out_angularDeltaVelocities[1] = inverseInertiaTensorsWs[1] * torqueWs;

		// Apply them
		contact->bodies[1]->AddWorldVelocity(out_linearDeltaVelocities[0]);
		contact->bodies[1]->AddWorldAngularVelocityRadians(out_angularDeltaVelocities[0]);
	}
}


//-------------------------------------------------------------------------------------------------
static void UpdateContactPenetrations(Contact* contacts, int numContacts, Vector3* linearChanges, Vector3* angularChanges, Contact* resolvedContact)
{
	// For each contact
	for (int contactIndex = 0; contactIndex < numContacts; ++contactIndex)
	{
		Contact* contact = &contacts[contactIndex];

		// Check each body in the contact
		for (int bodyIndex = 0; bodyIndex < 2; ++bodyIndex)
		{
			RigidBody* body = contact->bodies[bodyIndex];
			if (body == nullptr)
				continue;

			// Find if this contact shares a body with the contact we just resolved
			for (int resolvedBodyIndex = 0; resolvedBodyIndex < 2; ++resolvedBodyIndex)
			{
				RigidBody* resolvedBody = resolvedContact->bodies[resolvedBodyIndex];

				if (body == resolvedBody)
				{
					// If so, find and update the new penetration for this contact
					Vector3 deltaPosition = linearChanges[resolvedBodyIndex] + CrossProduct(angularChanges[resolvedBodyIndex], contact->bodyToContact[bodyIndex]);
					float sign = (bodyIndex == 1 ? 1.f : -1.0f); // If we're body A, any movement along this normal would reduce this penetration, so negative sign. If we're body B, any movement along the normal makes the penetration worse.
					contact->penetration += sign * DotProduct(deltaPosition, contact->normal);

					// TODO: Is this needed?
					contact->position += deltaPosition;
				}
			}
		}
	}
}


//-------------------------------------------------------------------------------------------------
static void UpdateContactVelocities(Contact* contacts, int numContacts, Vector3* linearVelocityChanges, Vector3* angularVelocityChanges, Contact* resolvedContact)
{
	// For each contact
	for (int contactIndex = 0; contactIndex < numContacts; ++contactIndex)
	{
		Contact* contact = &contacts[contactIndex];

		// Check each body in the contact
		for (int bodyIndex = 0; bodyIndex < 2; ++bodyIndex)
		{
			RigidBody* body = contact->bodies[bodyIndex];
			if (body == nullptr)
				continue;

			// Find if this contact shares a body with the contact we just resolved
			for (int resolvedBodyIndex = 0; resolvedBodyIndex < 2; ++resolvedBodyIndex)
			{
				RigidBody* resolvedBody = resolvedContact->bodies[resolvedBodyIndex];

				if (body == resolvedBody)
				{
					// If so, find and update the new penetration for this contact
					Vector3 deltaVelocityWs = linearVelocityChanges[resolvedBodyIndex] + CrossProduct(angularVelocityChanges[resolvedBodyIndex], contact->bodyToContact[bodyIndex]);
					float sign = (bodyIndex == 1 ? -1.f : 1.f); // From the perspective of A
					
					Vector3 deltaVelocityContactSpace = contact->contactToWorld.GetTranspose() * deltaVelocityWs;
					contact->closingVelocityContactSpace += sign * deltaVelocityContactSpace;

					// Recalculate the desired velocity
					contact->CalculateDesiredVelocityInContactSpace();
				}
			}
		}
	}
}


//-------------------------------------------------------------------------------------------------
static void ResolvePenetrations(Contact* contacts, int numContacts, int numIterations)
{
	for (int iteration = 0; iteration < numIterations; ++iteration)
	{
		Contact* contactToResolve = nullptr;

		for (int contactIndex = 0; contactIndex < numContacts; ++contactIndex)
		{
			Contact* contact = &contacts[contactIndex];

			// Find the contact with the worst pen (< 0.f is no pen, > 0.f is pen)
			if (contact->penetration > 0.f && (contactToResolve == nullptr || contact->penetration > contactToResolve->penetration))
			{
				contactToResolve = contact;
			}
		}

		if (contactToResolve == nullptr)
			break;

		Vector3 linearChanges[2];
		Vector3 angularChanges[2];

		contactToResolve->MatchAwakeState();

		ResolveContactPenetration(contactToResolve, linearChanges, angularChanges);

		// Update all other contacts that may have moved by fixing this contact
		UpdateContactPenetrations(contacts, numContacts, linearChanges, angularChanges, contactToResolve);
	}
}


//-------------------------------------------------------------------------------------------------
static void ResolveVelocities(Contact* contacts, int numContacts, int numIterations)
{
	for (int iteration = 0; iteration < numIterations; ++iteration)
	{
		Contact* contactToResolve = nullptr;

		for (int contactIndex = 0; contactIndex < numContacts; ++contactIndex)
		{
			Contact* contact = &contacts[contactIndex];

			// Find the contact the greatest desired change on velocity
			if (contact->desiredDeltaVelocity > 0.f && (contactToResolve == nullptr || contact->desiredDeltaVelocity > contactToResolve->desiredDeltaVelocity))
			{
				contactToResolve = contact;
			}
		}

		if (contactToResolve == nullptr)
			break;

		contactToResolve->MatchAwakeState();

		Vector3 linearVelocityChanges[2];
		Vector3 angularVelocityChanges[2];

		ResolveContactVelocity(contactToResolve, linearVelocityChanges, angularVelocityChanges);
		UpdateContactVelocities(contacts, numContacts, linearVelocityChanges, angularVelocityChanges, contactToResolve);
	}
}

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
void ContactResolver::ResolveContacts(Contact* contacts, int numContacts)
{
	PrepareContacts(contacts, numContacts);
	ResolveVelocities(contacts, numContacts, m_defaultNumVelocityIterations);
	ResolvePenetrations(contacts, numContacts, m_defaultNumPenetrationIterations);
}
