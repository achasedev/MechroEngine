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
#include "Engine/Core/DevConsole.h"
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
	const float angularLimit = 0.2f;
	float angularMove[2];
	float linearMove[2];

	float totalInertia = 0.f;
	float linearInertia[2];
	float angularInertia[2];

	//// We need to work out the inertia of each object in the direction
	//// of the contact normal, due to angular inertia only.
	//for (unsigned i = 0; i < 2; i++) if (contact->bodies[i])
	//{
	//	Matrix3 inverseInertiaTensor;
	//	contact->bodies[i]->GetWorldInverseInertiaTensor(inverseInertiaTensor);

	//	// Use the same procedure as for calculating frictionless
	//	// velocity change to work out the angular inertia.
	//	Vector3 angularInertiaWorld =
	//		CrossProduct(contact->bodyToContact[i], contact->normal);
	//	angularInertiaWorld =
	//		inverseInertiaTensor * angularInertiaWorld;
	//	angularInertiaWorld =
	//		CrossProduct(angularInertiaWorld, contact->bodyToContact[i]);
	//	angularInertia[i] =
	//		DotProduct(angularInertiaWorld, contact->normal);

	//	// The linear component is simply the inverse mass
	//	linearInertia[i] = contact->bodies[i]->GetInverseMass();

	//	// Keep track of the total inertia from all components
	//	totalInertia += linearInertia[i] + angularInertia[i];

	//	// We break the loop here so that the totalInertia value is
	//	// completely calculated (by both iterations) before
	//	// continuing.
	//}

	Vector3 deltaAngularVelocityPerImpulse[2];

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

	// Loop through again calculating and applying the changes
	for (unsigned i = 0; i < 2; i++) if (contact->bodies[i])
	{
		float sign = (i == 0 ? 1.0f : -1.0f);

		// Calculate the amount of linear movement from both the linear and angular components
		linearMove[i] = sign * contact->penetration * linearInertia[i] / totalInertia;
		angularMove[i] = sign * contact->penetration * angularInertia[i] / totalInertia;

		// To avoid angular projections that are too great (when mass is large
		// but inertia tensor is small) limit the angular move.
		Vector3 projection = contact->bodyToContact[i];
		projection += contact->normal * DotProduct(-1.0f * contact->bodyToContact[i], contact->normal);

		// Limit the amount of movement that comes from angular rotation
		float limit = angularLimit * contact->bodyToContact[i].GetLength(); // Normally this would be sin(angularLimit) * hypotenuse, but small angle approximation sin(angle) ~= angle
		if (Abs(angularMove[i]) > limit)
		{
			float totalMove = angularMove[i] + linearMove[i];

			if (angularMove[i] >= 0)
			{
				angularMove[i] = limit;
			}
			else
			{
				angularMove[i] = -limit;
			}

			linearMove[i] = totalMove - limit;
		}

		// Calculate changes
		out_linearChanges[i] = contact->normal * linearMove[i];
		Vector3 rotationPerMovement = (deltaAngularVelocityPerImpulse[i] / angularInertia[i]); // angularInertia === deltaLinearVelocityFromRotationPerUnitImpulse - "Per Impulse" cancels out, so this becomes a ratio of angular change per linear change :)
		out_angularChanges[i] = angularMove[i] * rotationPerMovement; // This is the linear movement we need from rotation * amount of rotation needed for 1 unit of linear movement

		// Apply changes
		contact->bodies[i]->transform->position += (contact->normal * linearMove[i]);
		contact->bodies[i]->transform->RotateRadians(out_angularChanges[i], RELATIVE_TO_WORLD);

		// Probably not necessary, but update the contact position as well
		contact->position += (linearMove[i] + angularMove[i]) * contact->normal;

		// Update the world inertia tensor, since we rotated
		contact->bodies[i]->CalculateDerivedData();
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
