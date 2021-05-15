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

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
void ContactResolver::ResolveContacts(Contact* contacts, int numContacts, float deltaSeconds)
{
	
}


//-------------------------------------------------------------------------------------------------
void ContactResolver::ResolvePenetration(Contact* contact)
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
		RigidBody* body = (bodyIndex == 0 ? contact->bodyA : contact->bodyB);

		if (body == nullptr)
		{
			linearInertia[bodyIndex] = 0.f;
			angularInertia[bodyIndex] = 0.f;
			continue;
		}

		// Calculate angular inertia at the contact
		Matrix3 inverseInertiaTensor;
		body->GetWorldInverseInertiaTensor(inverseInertiaTensor);

		Vector3 relativePosition = contact->position - body->transform->position;
		Vector3 torquePerImpulse = CrossProduct(relativePosition, contact->normal);
		deltaAngularVelocityPerImpulse[bodyIndex] = inverseInertiaTensor * torquePerImpulse;
		Vector3 linearVelocityPerImpulse = CrossProduct(deltaAngularVelocityPerImpulse[bodyIndex], relativePosition);

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
		RigidBody* body = (bodyIndex == 0 ? contact->bodyA : contact->bodyB);
		if (body == nullptr)
			continue;

		float sign = (bodyIndex == 0 ? 1.0f : -1.0f);

		// Calculate the amount of movement from the linear component of impulse
		linearMove[bodyIndex] = sign * contact->penetration * linearInertia[bodyIndex] * inverseTotalInverseInertia;

		// To move this contact by the linear amount, we can just add it to the object (linear move of the object linearly moves the contact)
		body->transform->position += contact->normal * linearMove[bodyIndex];

		// For the movement from angular component of impulse, we need to calculate the amount of rotation that would create this much movement
		Matrix3 inverseInertiaTensor;
		body->GetWorldInverseInertiaTensor(inverseInertiaTensor);

		Vector3 rotationPerMovement = (deltaAngularVelocityPerImpulse[bodyIndex] / angularInertia[bodyIndex]); // angularInertia === deltaLinearVelocityFromRotationPerUnitImpulse - "Per Impulse" cancels out, so this becomes a ratio of angular change per linear change :)
		Vector3 deltaRotation = angularMove[bodyIndex] * rotationPerMovement;

		// Now apply delta rotations
		body->transform->rotation *= Quaternion::CreateFromEulerAnglesRadians(deltaRotation);
	}
}
