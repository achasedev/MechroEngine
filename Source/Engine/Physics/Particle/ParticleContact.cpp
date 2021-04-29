///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 25th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Physics/Particle/Particle.h"
#include "Engine/Physics/Particle/ParticleContact.h"

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
ParticleContact::ParticleContact(Particle* particleA, Particle* particleB, float restitution, const Vector3& normal, float penetration)
	: m_particleA(particleA)
	, m_particleB(particleB)
	, m_restitution(restitution)
	, m_normal(normal)
	, m_penetration(penetration)
{
}


//-------------------------------------------------------------------------------------------------
float ParticleContact::CalculateSeparatingVelocity() const
{
	ASSERT_OR_DIE(AreMostlyEqual(m_normal.GetLengthSquared(), 1.0f), "No normal!");

	Vector3 relativeVelocity = m_particleA->GetVelocity();
	if (m_particleB != nullptr)
	{
		relativeVelocity -= m_particleB->GetVelocity();
	}

	return DotProduct(relativeVelocity, m_normal);
}


//-------------------------------------------------------------------------------------------------
void ParticleContact::ResolveVelocity(float deltaSeconds)
{
	float seperatingVelocity = CalculateSeparatingVelocity();

	// If > 0.f, then they're already moving apart, so no impulse needed
	if (seperatingVelocity > 0.f)
		return;

	// New separating velocity is opposite sign (along normal), but same magnitude (conserve momentum). Restitution will add velocity for a bounce.
	float newSeparatingVelocity = -seperatingVelocity * m_restitution;

	// Check for resting contact - If the particle's own acceleration contributed to the negative separating velocity (so closing velocity), we remove one frame's worth
	// This fixes the resting case of something having one frame's worth of negative separating velocity ends up being flip, causing a jitter. An object on another object
	// and gravity is the most common use case; a car with 0 velocity trying to drive into a wall is another.
	Vector3 relativeAcceleration = m_particleA->GetAcceleration();
	if (m_particleB != nullptr)
	{
		relativeAcceleration -= m_particleB->GetAcceleration();
	}

	float accCausedSepVelocityLastFrame = DotProduct(relativeAcceleration, m_normal) * deltaSeconds;

	if (accCausedSepVelocityLastFrame < 0.f) // < 0 means against the normal direction, away from A
	{
		// Remove *one frame's worth* of acceleration. Objects that accelerate for some time and then have this collision will still have a separating velocity > 0.f
		newSeparatingVelocity += m_restitution * accCausedSepVelocityLastFrame;
		newSeparatingVelocity = Clamp(newSeparatingVelocity, 0.f, newSeparatingVelocity); // No need to remove more than we have
	}

	float deltaVelocity = newSeparatingVelocity - seperatingVelocity;

	// "Distribute" new velocity proportioned by inverse mass
	float totalIMass = m_particleA->GetInverseMass();
	if (m_particleB != nullptr)
	{
		totalIMass += m_particleB->GetInverseMass();
	}

	// totalIMass == 0 means both particles have infinite mass, so no moving them
	if (totalIMass > 0)
	{
		// Find "impulse per inverse mass," to be multiplied by each's inverse mass to find their change in velocity
		float impulsePerIMassMagnitude = deltaVelocity / totalIMass; // p = mv, since we have m^-1 here divide by it instead	
		Vector3 impulsePerIMass = m_normal * impulsePerIMassMagnitude;

		// Apply impulse
		m_particleA->SetVelocity(m_particleA->GetVelocity() + impulsePerIMass * m_particleA->GetInverseMass());
		if (m_particleB != nullptr)
		{
			m_particleB->SetVelocity(m_particleB->GetVelocity() + (-1.0f * impulsePerIMass * m_particleB->GetInverseMass()));
		}
	}
}


//-------------------------------------------------------------------------------------------------
void ParticleContact::ResolveInterpenetration(Vector3& out_correctionA, Vector3& out_correctionB)
{
	if (m_penetration <= 0.f)
		return;
	
	// "Distribute" the correction proportioned by inverse mass
	float totalIMass = m_particleA->GetInverseMass();
	if (m_particleB != nullptr)
	{
		totalIMass += m_particleB->GetInverseMass();
	}

	// totalIMass == 0 means both particles have infinite mass, so no moving them
	if (totalIMass <= 0.f)
		return;

	Vector3 movePerIMass = m_normal * (m_penetration / totalIMass);

	out_correctionA = movePerIMass * m_particleA->GetInverseMass();
	m_particleA->SetPosition(m_particleA->GetPosition() + out_correctionA);

	if (m_particleB != nullptr)
	{
		out_correctionB = -1.0f * movePerIMass * m_particleB->GetInverseMass();
		m_particleB->SetPosition(m_particleB->GetPosition() + out_correctionB);
	}
	else
	{
		out_correctionB = Vector3::ZERO;
	}

	// TODO: Need to update the penetration of all other contacts that involve these particles
	m_penetration = 0.f;
}
