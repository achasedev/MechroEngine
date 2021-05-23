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
static void PrepareContacts(Contact* contacts, int numContacts, float deltaSeconds)
{
	for (int i = 0; i < numContacts; ++i)
	{
		contacts[i].CheckValuesAreReasonable();
		contacts[i].CalculateInternals(deltaSeconds);
		contacts[i].CheckValuesAreReasonable();
	}
}


//-------------------------------------------------------------------------------------------------
static void ResolveContactPenetration(Contact* contact, Vector3* out_linearChanges, Vector3* out_angularChanges)
{
	contact->CheckValuesAreReasonable();

	const float angularLimit = 0.2f;
	float angularMove[2];
	float linearMove[2];

	float totalInertia = 0.f;
	float linearInertia[2];
	float angularInertia[2];

	Vector3 deltaAngularVelocityPerUnitImpulse[2];

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
		ASSERT_REASONABLE(inverseInertiaTensor);

		Vector3 torquePerUnitImpulse = CrossProduct(contact->bodyToContact[bodyIndex], contact->normal);
		ASSERT_REASONABLE(torquePerUnitImpulse);
		deltaAngularVelocityPerUnitImpulse[bodyIndex] = inverseInertiaTensor * torquePerUnitImpulse;
		ASSERT_REASONABLE(deltaAngularVelocityPerUnitImpulse[bodyIndex]);
		Vector3 linearVelocityPerImpulse = CrossProduct(deltaAngularVelocityPerUnitImpulse[bodyIndex], contact->bodyToContact[bodyIndex]);
		ASSERT_REASONABLE(linearVelocityPerImpulse);

		angularInertia[bodyIndex] = DotProduct(linearVelocityPerImpulse, contact->normal); // More resistant object (massive) === less linearVelocityPerImpulse === lesser inertia === less change
		ASSERT_REASONABLE(angularInertia[bodyIndex]);

		// Linear component is just inverse mass
		linearInertia[bodyIndex] = body->GetInverseMass();

		// Track total inertia for distributing proportional to mass. Greater inertia here means *greater* correction!
		totalInertia += linearInertia[bodyIndex] + angularInertia[bodyIndex];
	}

	// Don't use a single loop - we need the inertias fully calculated before proceeding

	// Loop through again calculating and applying the changes
	for (unsigned i = 0; i < 2; i++) if (contact->bodies[i])
	{
		float sign = (i == 0 ? 1.0f : -1.0f);

		// Calculate the amount of linear movement from both the linear and angular components
		linearMove[i] = sign * contact->penetration * linearInertia[i] / totalInertia;
		angularMove[i] = sign * contact->penetration * angularInertia[i] / totalInertia;
		ASSERT_REASONABLE(linearMove[i]);
		ASSERT_REASONABLE(angularMove[i]);

		// To avoid angular projections that are too great (when mass is large
		// but inertia tensor is small) limit the angular move.
		Vector3 projection = contact->bodyToContact[i];
		projection += contact->normal * DotProduct(-1.0f * contact->bodyToContact[i], contact->normal);

		// Limit the amount of movement that comes from angular rotation
		float limit = angularLimit * projection.GetLength(); // Normally this would be sin(angularLimit) * hypotenuse, but small angle approximation sin(angle) ~= angle
		if (angularMove[i] < -limit)
		{
			float totalMove = angularMove[i] + linearMove[i];
			angularMove[i] = -limit;
			linearMove[i] = totalMove - angularMove[i];
		}
		else if (angularMove[i] > limit)
		{
			float totalMove = angularMove[i] + linearMove[i];
			angularMove[i] = limit;
			linearMove[i] = totalMove - angularMove[i];
		}

		// Calculate changes
		out_linearChanges[i] = contact->normal * linearMove[i];
		ASSERT_REASONABLE(out_linearChanges[i]);
		Vector3 rotationPerMovement = (deltaAngularVelocityPerUnitImpulse[i] / angularInertia[i]); // angularInertia === deltaLinearVelocityFromRotationPerUnitImpulse - "Per Impulse" cancels out, so this becomes a ratio of angular change per linear change :)
		ASSERT_REASONABLE(rotationPerMovement);
		out_angularChanges[i] = angularMove[i] * rotationPerMovement; // This is the linear movement we need from rotation * amount of rotation needed for 1 unit of linear movement
		ASSERT_REASONABLE(out_angularChanges[i]);

		// Apply changes
		contact->bodies[i]->transform->position += (contact->normal * linearMove[i]);
		contact->bodies[i]->transform->RotateRadians(out_angularChanges[i], RELATIVE_TO_WORLD);

		// Probably not necessary, but update the contact position as well
		contact->CheckValuesAreReasonable();
	}
}


//-------------------------------------------------------------------------------------------------
static Vector3 CalculateFrictionlessImpulse(Contact* contact)
{
	contact->CheckValuesAreReasonable();

	// Calculate how much delta velocity is created from 1 unit of impulse along the contact normal
	float deltaVelocityAlongNormalPerUnitOfImpulse = 0.f;

	// First body
	{
		Matrix3 inverseInertiaTensorWs;
		contact->bodies[0]->GetWorldInverseInertiaTensor(inverseInertiaTensorWs);
		ASSERT_REASONABLE(inverseInertiaTensorWs);

		Vector3 deltaVelocityWs = CrossProduct(contact->bodyToContact[0], contact->normal);	
		deltaVelocityWs = inverseInertiaTensorWs * deltaVelocityWs;
		deltaVelocityWs = CrossProduct(deltaVelocityWs, contact->bodyToContact[0]);
		ASSERT_REASONABLE(deltaVelocityWs);

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
		ASSERT_REASONABLE(inverseInertiaTensorWs);

		Vector3 deltaVelocityWs = CrossProduct(contact->bodyToContact[1], contact->normal);
		deltaVelocityWs = inverseInertiaTensorWs * deltaVelocityWs;
		deltaVelocityWs = CrossProduct(deltaVelocityWs, contact->bodyToContact[1]);
		ASSERT_REASONABLE(deltaVelocityWs);

		// Angular part
		deltaVelocityAlongNormalPerUnitOfImpulse += DotProduct(deltaVelocityWs, contact->normal);

		// Linear part
		deltaVelocityAlongNormalPerUnitOfImpulse += contact->bodies[1]->GetInverseMass();
	}

	// Then calculate how much impulse we'd need to get our desired velocity along the normal
	return Vector3(contact->desiredDeltaVelocityAlongNormal / deltaVelocityAlongNormalPerUnitOfImpulse, 0.f, 0.f); // X vector in contact space is the normal, 0 out the other directions for no friction
}


//-------------------------------------------------------------------------------------------------
// Isotropic, Static/Dynamic depends on coplanar velocity. No rolling friction.
static Vector3 CalculateFrictionImpulse(Contact* contact)
{
	contact->CheckValuesAreReasonable();

	Vector3 impulseContact;
	float inverseMass = contact->bodies[0]->GetInverseMass();

	// The equivalent of a cross product in matrices is multiplication
	// by a skew symmetric matrix - we build the matrix for converting
	// between linear and angular quantities.
	Matrix3 impulseToTorque;
	impulseToTorque.SetAsSkewSymmetric(contact->bodyToContact[0]);

	// Build the matrix to convert contact impulse to change in velocity
	// in world coordinates.
	Matrix3 inverseInertiaTensorWorld;
	contact->bodies[0]->GetWorldInverseInertiaTensor(inverseInertiaTensorWorld);
	ASSERT_REASONABLE(inverseInertiaTensorWorld);

	Matrix3 deltaVelWorld = impulseToTorque;
	deltaVelWorld *= inverseInertiaTensorWorld;
	deltaVelWorld *= impulseToTorque;
	deltaVelWorld *= -1.f;
	ASSERT_REASONABLE(deltaVelWorld);

	// Check if we need to add body two's data
	if (contact->bodies[1])
	{
		// Set the cross product matrix
		impulseToTorque.SetAsSkewSymmetric(contact->bodyToContact[1]);

		// Calculate the velocity change matrix
		Matrix3 inverseInertiaTensorWorld2;
		contact->bodies[1]->GetWorldInverseInertiaTensor(inverseInertiaTensorWorld2);

		Matrix3 deltaVelWorld2 = impulseToTorque;
		deltaVelWorld2 *= inverseInertiaTensorWorld2;
		deltaVelWorld2 *= impulseToTorque;
		deltaVelWorld2 *= -1;
		ASSERT_REASONABLE(deltaVelWorld2);

		// Add to the total delta velocity.
		deltaVelWorld += deltaVelWorld2;

		// Add to the inverse mass
		inverseMass += contact->bodies[1]->GetInverseMass();
	}

	// Do a change of basis to convert into contact coordinates.
	Matrix3 deltaVelocity = contact->contactToWorld.GetTranspose();
	deltaVelocity *= deltaVelWorld;
	deltaVelocity *= contact->contactToWorld;
	
	// Add in the linear velocity change
	deltaVelocity.data[0] += inverseMass;
	deltaVelocity.data[4] += inverseMass;
	deltaVelocity.data[8] += inverseMass;
	ASSERT_REASONABLE(deltaVelocity);

	// Invert to get the impulse needed per unit velocity
	Matrix3 impulseMatrix = deltaVelocity.GetInverse();

	// Find the target velocities to kill
	Vector3 killVelocity(contact->desiredDeltaVelocityAlongNormal, -contact->closingVelocityContactSpace.y, -contact->closingVelocityContactSpace.z);

	// Find the impulse to kill target velocities
	impulseContact = impulseMatrix * killVelocity;

	// Check for exceeding friction
	float planarImpulse = Sqrt(impulseContact.y * impulseContact.y + impulseContact.z * impulseContact.z);

	if (!AreMostlyEqual(planarImpulse, 0.f) && planarImpulse > impulseContact.x * contact->friction) // AreMostlyEqual check to protect against near-zero cases
	{
		// We need to use dynamic friction
		impulseContact.y /= planarImpulse;
		impulseContact.z /= planarImpulse;

		impulseContact.x = deltaVelocity.data[0] + deltaVelocity.data[1] * contact->friction*impulseContact.y + deltaVelocity.data[2] * contact->friction*impulseContact.z;
		impulseContact.x = contact->desiredDeltaVelocityAlongNormal / impulseContact.x;
		impulseContact.y *= contact->friction * impulseContact.x;
		impulseContact.z *= contact->friction * impulseContact.x;
	}

	return impulseContact;
}


//-------------------------------------------------------------------------------------------------
static void ResolveContactVelocity(Contact* contact, Vector3* out_linearDeltaVelocities, Vector3* out_angularDeltaVelocities)
{
	if (contact->bodies[1] == nullptr)
	{
		ASSERT_OR_DIE(contact->bodies[0]->IsAwake(), "Sleeping body attempted to resolve velocity!");
	}
	else
	{
		ASSERT_OR_DIE(contact->bodies[0]->IsAwake() || contact->bodies[1]->IsAwake(), "Two sleeping bodies attempted to resolve velocity!")
	}

	Matrix3 inverseInertiaTensorsWs[2];
	contact->bodies[0]->GetWorldInverseInertiaTensor(inverseInertiaTensorsWs[0]);
	ASSERT_REASONABLE(inverseInertiaTensorsWs[0]);

	if (contact->bodies[1] != nullptr)
	{
		contact->bodies[1]->GetWorldInverseInertiaTensor(inverseInertiaTensorsWs[1]);
		ASSERT_REASONABLE(inverseInertiaTensorsWs[1]);
	}

	Vector3 impulseInContactSpace = Vector3::ZERO;

	if (AreMostlyEqual(contact->friction, 0.f))
	{
		impulseInContactSpace = CalculateFrictionlessImpulse(contact);
	}
	else
	{
		impulseInContactSpace = CalculateFrictionImpulse(contact);
	}

	ASSERT_REASONABLE(impulseInContactSpace);

	Vector3 impulseWs = contact->contactToWorld * impulseInContactSpace;
	ASSERT_REASONABLE(impulseWs);

	// Calculate first body delta velocities
	{
		Vector3 torqueWs = CrossProduct(contact->bodyToContact[0], impulseWs);
		ASSERT_REASONABLE(torqueWs);

		// Calculate changes
		out_linearDeltaVelocities[0] = impulseWs * contact->bodies[0]->GetInverseMass();
		out_angularDeltaVelocities[0] = inverseInertiaTensorsWs[0] * torqueWs;

		ASSERT_REASONABLE(out_linearDeltaVelocities[0]);
		ASSERT_REASONABLE(out_angularDeltaVelocities[0]);

		// Apply them
		contact->bodies[0]->AddWorldVelocity(out_linearDeltaVelocities[0]);
		contact->bodies[0]->AddWorldAngularVelocityRadians(out_angularDeltaVelocities[0]);
	}

	// Calculate second bodies velocities
	if (contact->bodies[1] != nullptr)
	{
		Vector3 torqueWs = CrossProduct(impulseWs, contact->bodyToContact[1]); // Switched cross, since torque would be in opposite direction
		ASSERT_REASONABLE(torqueWs);

		// Calculate changes
		out_linearDeltaVelocities[1] = -1.0f * impulseWs * contact->bodies[1]->GetInverseMass(); // Velocity change would be opposite the first
		out_angularDeltaVelocities[1] = inverseInertiaTensorsWs[1] * torqueWs;

		ASSERT_REASONABLE(out_linearDeltaVelocities[1]);
		ASSERT_REASONABLE(out_angularDeltaVelocities[1]);

		// Apply them
		contact->bodies[1]->AddWorldVelocity(out_linearDeltaVelocities[1]);
		contact->bodies[1]->AddWorldAngularVelocityRadians(out_angularDeltaVelocities[1]);
	}
}


//-------------------------------------------------------------------------------------------------
static void UpdateContactPenetrations(Contact* contacts, int numContacts, Vector3* linearChanges, Vector3* angularChanges, Contact* resolvedContact)
{
	// For each contact
	for (int contactIndex = 0; contactIndex < numContacts; ++contactIndex)
	{
		Contact* contact = &contacts[contactIndex];

		contact->CheckValuesAreReasonable();

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
					ASSERT_REASONABLE(deltaPosition);

					float sign = (bodyIndex == 1 ? 1.f : -1.0f); // If we're body A, any movement along this normal would reduce this penetration, so negative sign. If we're body B, any movement along the normal makes the penetration worse.
					contact->penetration += sign * DotProduct(deltaPosition, contact->normal);

					// TODO: Is this needed?
					contact->position += deltaPosition;
					contact->bodyToContact[0] = contact->position - contact->bodies[0]->transform->position;
					if (contact->bodies[1] != nullptr)
					{
						contact->bodyToContact[1] = contact->position - contact->bodies[1]->transform->position;
					}
				}
			}
		}

		contact->CheckValuesAreReasonable();
	}
}


//-------------------------------------------------------------------------------------------------
static void UpdateContactVelocities(Contact* contacts, int numContacts, Vector3* linearVelocityChanges, Vector3* angularVelocityChanges, Contact* resolvedContact, float deltaSeconds)
{
	// For each contact
	for (int contactIndex = 0; contactIndex < numContacts; ++contactIndex)
	{
		Contact* contact = &contacts[contactIndex];
		contact->CheckValuesAreReasonable();

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
					contact->CalculateDesiredVelocityInContactSpace(deltaSeconds);
				}
			}
		}

		contact->CheckValuesAreReasonable();
	}
}


//-------------------------------------------------------------------------------------------------
static void ResolvePenetrations(Contact* contacts, int numContacts, int numIterations, float penetrationEpsilon)
{
	int numIterationsUsed = 0;
	for (numIterationsUsed = 0; numIterationsUsed < numIterations; ++numIterationsUsed)
	{
		Contact* contactToResolve = nullptr;

		for (int contactIndex = 0; contactIndex < numContacts; ++contactIndex)
		{
			Contact* contact = &contacts[contactIndex];

			// Find the contact with the worst pen (< 0.f is no pen, > 0.f is pen)
			if (contact->penetration > penetrationEpsilon && (contactToResolve == nullptr || contact->penetration > contactToResolve->penetration))
			{
				contactToResolve = contact;
			}
		}

		if (contactToResolve == nullptr)
			break;

		Vector3 linearChanges[2];
		Vector3 angularChanges[2];

		contactToResolve->CheckValuesAreReasonable();
		contactToResolve->MatchAwakeState();

		ResolveContactPenetration(contactToResolve, linearChanges, angularChanges);

		// Update all other contacts that may have moved by fixing this contact
		UpdateContactPenetrations(contacts, numContacts, linearChanges, angularChanges, contactToResolve);
	}

	if (numIterationsUsed == numIterations)
	{
		ConsoleWarningf("Resolver hit the max iteration count for penetrations at %i", numIterationsUsed);
	}
}


//-------------------------------------------------------------------------------------------------
static void ResolveVelocities(Contact* contacts, int numContacts, int numIterations, float velocityEpsilon, float deltaSeconds)
{
	Vector3 linearVelocityChanges[2];
	Vector3 angularVelocityChanges[2];

	int numIterationsUsed = 0;
	for (numIterationsUsed; numIterationsUsed < numIterations; ++numIterationsUsed)
	{
		Contact* contactToResolve = nullptr;

		for (int contactIndex = 0; contactIndex < numContacts; ++contactIndex)
		{
			Contact* contact = &contacts[contactIndex];

			// Find the contact the greatest desired change on velocity
			if (contact->desiredDeltaVelocityAlongNormal > velocityEpsilon && (contactToResolve == nullptr || contact->desiredDeltaVelocityAlongNormal > contactToResolve->desiredDeltaVelocityAlongNormal))
			{
				contactToResolve = contact;
			}
		}

		if (contactToResolve == nullptr)
			break;

		contactToResolve->CheckValuesAreReasonable();
		contactToResolve->MatchAwakeState();

		ResolveContactVelocity(contactToResolve, linearVelocityChanges, angularVelocityChanges);
		UpdateContactVelocities(contacts, numContacts, linearVelocityChanges, angularVelocityChanges, contactToResolve, deltaSeconds);
	}

	if (numIterationsUsed == numIterations)
	{
		ConsoleWarningf("Resolver hit the max iteration count for velocities at %i", numIterationsUsed);
	}
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
void ContactResolver::ResolveContacts(Contact* contacts, int numContacts, float deltaSeconds)
{
	PrepareContacts(contacts, numContacts, deltaSeconds);
	ResolveVelocities(contacts, numContacts, m_maxVelocityIterations, m_velocityEpsilon, deltaSeconds);
	ResolvePenetrations(contacts, numContacts, m_maxPenetrationIterations, m_penetrationEpsilon);
}
