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
static void PrepareContacts(Contact* contacts, int numContacts, float deltaSeconds)
{
	for (int i = 0; i < numContacts; ++i)
	{
		contacts[i].CalculateInternals(deltaSeconds);
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
		body->transform->position += out_linearChanges[bodyIndex];

		// For the movement from angular component of impulse, we need to calculate the amount of rotation that would create this much movement
		Matrix3 inverseInertiaTensor;
		body->GetWorldInverseInertiaTensor(inverseInertiaTensor);

		Vector3 rotationPerMovement = (deltaAngularVelocityPerImpulse[bodyIndex] / angularInertia[bodyIndex]); // angularInertia === deltaLinearVelocityFromRotationPerUnitImpulse - "Per Impulse" cancels out, so this becomes a ratio of angular change per linear change :)
		out_angularChanges[bodyIndex] = angularMove[bodyIndex] * rotationPerMovement;

		// Now apply delta rotations
		body->transform->rotation *= Quaternion::CreateFromEulerAnglesRadians(out_angularChanges[bodyIndex]);

		if (!body->IsAwake())
		{
			// If the object is sleeping, it's derived data wasn't calculated
			// Make sure our inertia tensor in world space is up-to-date
			body->CalculateDerivedData();
		}
	}
}

//-------------------------------------------------------------------------------------------------
static void UpdateContactPenetrations(Contact* contacts, int numContacts, Vector3* linearChanges, Vector3* angularChanges, Contact* resolvedContact)
{
	// For each contact
	for (int contactIndex = 0; contactIndex < numContacts; ++contactIndex)
	{
		Contact* contact = &contacts[contactIndex];

		if (contact == resolvedContact)
			continue;

		// Check each body in the contact
		for (int bodyIndex = 0; bodyIndex < 2; ++bodyIndex)
		{
			RigidBody* body = contact->bodies[bodyIndex];
			if (body == nullptr)
				continue;

			// Find if this contact shares a body with the contact we just resolved
			for (int resolvedBodyIndex = 0; resolvedBodyIndex < 2; ++resolvedBodyIndex)
			{
				RigidBody* resolvedBody = resolvedContact->bodies[bodyIndex];

				if (body == resolvedBody)
				{
					// If so, find and update the new penetration for this contact
					Vector3 deltaPosition = linearChanges[resolvedBodyIndex] + CrossProduct(angularChanges[resolvedBodyIndex], contact->bodyToContact[bodyIndex]);
					float sign = (bodyIndex == 1 ? 1.f : -1.0f); // If we're body A, any movement along this normal would reduce this penetration, so negative sign. If we're body B, any movement along the normal makes the penetration worse.
					contact->penetration += sign * DotProduct(deltaPosition, contact->normal);
				}
			}
		}
	}
}


//-------------------------------------------------------------------------------------------------
static void ResolvePenetrations(Contact* contacts, int numContacts, int maxIterations)
{
	for (int iteration = 0; iteration < maxIterations; ++iteration)
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
static void ResolveVelocities(Contact* contacts, int numContacts, float deltaSeconds, int maxIterations)
{
	for (int iteration = 0; iteration < maxIterations; ++iteration)
	{
		Contact* contactToResolve = nullptr;

		for (int contactIndex = 0; contactIndex < numContacts; ++contactIndex)
		{
			Contact* contact = &contacts[contactIndex];

			// Find the contact with the worst pen (< 0.f is no pen, > 0.f is pen)
			if (contact->closingVelocityContactSpace > 0.f && (contactToResolve == nullptr || contact->penetration > contactToResolve->penetration))
			{
				contactToResolve = contact;
			}
		}

		if (contactToResolve == nullptr)
			break;

		contactToResolve->MatchAwakeState();

		blah blah blah
	}
}

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
void ContactResolver::ResolveContacts(Contact* contacts, int numContacts, float deltaSeconds)
{
	PrepareContacts(contacts, numContacts, deltaSeconds);
	ResolveVelocities(contacts, numContacts, deltaSeconds, m_maxVelocityIterations);
	ResolvePenetrations(contacts, numContacts, m_maxPenetrationIterations);
}
