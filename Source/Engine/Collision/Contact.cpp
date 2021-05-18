///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: May 17th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Collision/Contact.h"
#include "Engine/Core/EngineCommon.h"
#include "Engine/Math/MathUtils.h"
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
Contact::Contact()
{
	bodies[0] = nullptr;
	bodies[1] = nullptr;
	bodyToContact[0] = Vector3::ZERO;
	bodyToContact[1] = Vector3::ZERO;
}


//-------------------------------------------------------------------------------------------------
void Contact::CalculateInternals(float deltaSeconds)
{
	ASSERT_OR_DIE(bodies[0] != nullptr, "First body was nullptr!");

	CalculateBasis();


	// Get R1, R2
	bodyToContact[0] = position - bodies[0]->transform->position;
	if (bodies[1] != nullptr)
	{
		bodyToContact[1] = position - bodies[1]->transform->position;
	}

	// Calculate velocities in contact space
	CalculateClosingVelocityInContactSpace();
	CalculateDesiredVelocityInContactSpace();
}


//-------------------------------------------------------------------------------------------------
void Contact::CalculateBasis()
{
	Vector3 crossReference = (!AreMostlyEqual(normal, Vector3::Y_AXIS) ? Vector3::Y_AXIS : Vector3::X_AXIS);

	Vector3 tangent = CrossProduct(crossReference, normal);
	Vector3 bitangent = CrossProduct(normal, tangent);

	contactToWorld = Matrix3(normal, tangent, bitangent); // X-axis (I vector) is the normal
}


//-------------------------------------------------------------------------------------------------
void Contact::CalculateClosingVelocityInContactSpace()
{
	closingVelocityContactSpace = Vector3::ZERO;

	for (int bodyIndex = 0; bodyIndex < 2; ++bodyIndex)
	{
		RigidBody* body = bodies[bodyIndex];
		if (body == nullptr)
			continue;

		Vector3 velocityWs = CrossProduct(body->GetAngularVelocityRadians(), bodyToContact[bodyIndex]) + body->GetLinearVelocity();
		Vector3 velocityContactSpace = contactToWorld.GetTranspose() * velocityWs; // Transpose == inverse

		// TODO: Add in velocity from acceration, when friction is added
		float sign = (bodyIndex == 0 ? 1.0f : -1.0f);
		closingVelocityContactSpace += sign * velocityContactSpace;
	}
}


//-------------------------------------------------------------------------------------------------
void Contact::CalculateDesiredVelocityInContactSpace()
{
	// TODO: Limit restitution on low velocities
	desiredDeltaVelocity = -closingVelocityContactSpace.x - restitution * closingVelocityContactSpace.x;
}
