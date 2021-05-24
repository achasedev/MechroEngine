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
	CheckValuesAreReasonable();

	ASSERT_OR_DIE(bodies[0] != nullptr, "First body was nullptr!");

	CalculateBasis();

	// Get R1, R2
	bodyToContact[0] = position - bodies[0]->transform->position;
	if (bodies[1] != nullptr)
	{
		bodyToContact[1] = position - bodies[1]->transform->position;
	}

	// Calculate velocities in contact space
	CalculateClosingVelocityInContactSpace(deltaSeconds);
	CalculateDesiredVelocityInContactSpace(deltaSeconds);
}


//-------------------------------------------------------------------------------------------------
void Contact::CalculateBasis()
{
	CheckValuesAreReasonable();

	ASSERT_OR_DIE(AreMostlyEqual(normal.GetLength(), 1.0f), "Normal isn't unit!");

	Vector3 crossReference	= (!AreMostlyEqual(normal, Vector3::Y_AXIS) ? Vector3::Y_AXIS : Vector3::X_AXIS);
	Vector3 tangent			= CrossProduct(crossReference, normal).GetNormalized();
	Vector3 bitangent		= CrossProduct(normal, tangent);
	ASSERT_OR_DIE(AreMostlyEqual(bitangent.GetLength(), 1.0f), "bitangent isn't unit!");
	contactToWorld			= Matrix3(normal, tangent, bitangent); // X-axis (I vector) is the normal
}


//-------------------------------------------------------------------------------------------------
void Contact::CalculateClosingVelocityInContactSpace(float deltaSeconds)
{
	CheckValuesAreReasonable();

	closingVelocityContactSpace = Vector3::ZERO;

	for (int bodyIndex = 0; bodyIndex < 2; ++bodyIndex)
	{
		RigidBody* thisBody = bodies[bodyIndex];

		if (thisBody == nullptr)
			continue;

		// Work out the velocity of the contact point
		Vector3 velocityWs = CrossProduct(thisBody->GetAngularVelocityRadiansWs(), bodyToContact[bodyIndex]);
		velocityWs += thisBody->GetVelocityWs();
		ASSERT_REASONABLE(velocityWs);

		// Turn the velocity into contact-coordinates
		Vector3 contactVelocity = contactToWorld.GetTranspose() * velocityWs;
		ASSERT_REASONABLE(contactVelocity);

		// Calculate the amount of velocity that is due to forces without reactions
		Vector3 accVelocity = thisBody->GetLastFrameAcceleration() * deltaSeconds;
		ASSERT_REASONABLE(accVelocity);

		// Calculate the velocity in contact-coordinates
		accVelocity = contactToWorld.GetTranspose() * accVelocity;
		ASSERT_REASONABLE(accVelocity);

		// We ignore any component of acceleration in the contact normal
		// direction, we are only interested in planar acceleration
		accVelocity.x = 0.f;

		// Add the planar velocities - if there's enough friction they will
		// be removed during velocity resolution
		contactVelocity += accVelocity;

		// Since contact velocity is relative, we change signs between the bodies
		float sign = (bodyIndex == 0 ? 1.0f : -1.0f);
		closingVelocityContactSpace += (sign * contactVelocity);
	}
}


//-------------------------------------------------------------------------------------------------
void Contact::CalculateDesiredVelocityInContactSpace(float deltaSeconds)
{
	CheckValuesAreReasonable();

	// If the velocity of the body along the normal is below a certain limit (practically resting), then don't apply restitution
	// This helps with slow collisions bouncing too much
	const float minClosingVelocityForRestitution = 0.25f;
	float restitutionToApply = restitution;

	if (Abs(closingVelocityContactSpace.x) < minClosingVelocityForRestitution)
	{
		restitutionToApply = 0.f;
	}

	// Determine the amount of velocity from this frame's acceleration
	float closingVelocityAddedLastIntegrate = 0.f;
	
	if (bodies[0]->IsAwake())
	{
		closingVelocityAddedLastIntegrate += DotProduct(bodies[0]->GetLastFrameAcceleration() * deltaSeconds, normal);
		ASSERT_REASONABLE(closingVelocityAddedLastIntegrate);
	}

	if (bodies[1] != nullptr && bodies[1]->IsAwake())
	{
		closingVelocityAddedLastIntegrate -= DotProduct(bodies[1]->GetLastFrameAcceleration() * deltaSeconds, normal);
		ASSERT_REASONABLE(closingVelocityAddedLastIntegrate);
	}

	// When applying restitution, don't factor in acceleration from last frame
	// This prevents bouncing while resting or pushing against something solid
	desiredDeltaVelocityAlongNormal = -closingVelocityContactSpace.x - restitution * (closingVelocityContactSpace.x - closingVelocityAddedLastIntegrate);
}


//-------------------------------------------------------------------------------------------------
void Contact::CheckValuesAreReasonable() const
{
	ASSERT_REASONABLE(position);
	ASSERT_REASONABLE(normal);
	ASSERT_REASONABLE(penetration);
	ASSERT_REASONABLE(restitution);
	ASSERT_REASONABLE(friction);
	ASSERT_REASONABLE(closingVelocityContactSpace);
	ASSERT_REASONABLE(desiredDeltaVelocityAlongNormal);
	ASSERT_REASONABLE(bodyToContact[0]);
	if (bodies[1] != nullptr)
	{
		ASSERT_REASONABLE(bodyToContact[1]);
	}

	ASSERT_OR_DIE(isValid, "Processing an invalid contact!");
}


//-------------------------------------------------------------------------------------------------
void Contact::MatchAwakeState()
{
	// This is what prevents sleeping bodies colliding with the world from waking up
	if (bodies[1] == nullptr)
		return;

	bool bodyZeroAwake = bodies[0]->IsAwake();
	bool bodyOneAwake = bodies[1]->IsAwake();

	// If both are asleep, don't wake them up
	// If exactly one is asleep, only wake the sleeping one
	if (bodyZeroAwake && !bodyOneAwake)
	{
		bodies[1]->SetIsAwake(true);
	}
	else if (bodyOneAwake && !bodyZeroAwake)
	{
		bodies[0]->SetIsAwake(true);
	}
}


//-------------------------------------------------------------------------------------------------
ContactFeatureRecord::ContactFeatureRecord(ContactRecordType type, const Collider* firstCollider, const Collider* secondCollider, ContactFeatureID firstID, ContactFeatureID secondID)
	: m_type(type)
	, m_firstCollider(firstCollider)
	, m_secondCollider(secondCollider)
	, m_firstID(firstID)
	, m_secondID(secondID)
{
}


//-------------------------------------------------------------------------------------------------
bool ContactFeatureRecord::operator==(const ContactFeatureRecord& other) const
{
	return (m_type == other.m_type) && (m_firstCollider == other.m_firstCollider) && (m_secondCollider == other.m_secondCollider) && (m_firstID == other.m_firstID) && (m_secondID == other.m_secondID);
}


//-------------------------------------------------------------------------------------------------
bool ContactFeatureRecord::operator!=(const ContactFeatureRecord& other) const
{
	return !((*this) == other);
}
