///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 28th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/Core/Rgba.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Physics/Particle/Particle.h"
#include "Engine/Physics/Particle/ParticleContact.h"
#include "Engine/Physics/Particle/ParticleContactGenerator.h"
#include "Engine/Physics/Particle/ParticleForceGenerator.h"
#include "Engine/Physics/Particle/ParticleWorld.h"
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

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
ParticleWorld::ParticleWorld(int defaultNumIterations, int maxContacts)
	: m_maxContacts(maxContacts)
	, m_defaultNumIterations(defaultNumIterations)
{
	m_contacts = (ParticleContact*)malloc(sizeof(ParticleContact) * m_maxContacts);
}


//-------------------------------------------------------------------------------------------------
ParticleWorld::~ParticleWorld()
{
	SAFE_FREE(m_contacts);
	SafeDeleteVector(m_forceGens);
	SafeDeleteVector(m_contactGens);
	SafeDeleteVector(m_particles);
}


//-------------------------------------------------------------------------------------------------
void ParticleWorld::DebugDrawParticles()
{
	UNIMPLEMENTED();
	//for (Particle* particle : m_particles)
	//{
	//	//DebugDrawPoint3D(particle->GetPosition(), Rgba::RED, 0.f);
	//}
}


//-------------------------------------------------------------------------------------------------
void ParticleWorld::DoPhysicsStep(float deltaSeconds)
{
	// Apply all forces
	m_forceRegistry.GenerateAndApplyForces(deltaSeconds);

	// Update all positions and velocities
	Integrate(deltaSeconds);

	// Check for constraints and collisions
	GenerateContacts();

	if (m_numContactsUsed > 0)
	{
		m_resolver.SetMaxIterations(Max(m_defaultNumIterations, 2 * m_numContactsUsed));
		m_resolver.ResolveContacts(m_contacts, m_numContactsUsed, deltaSeconds);
	}
}


//-------------------------------------------------------------------------------------------------
void ParticleWorld::AddParticle(Particle* particle)
{
	m_particles.push_back(particle);
}


//-------------------------------------------------------------------------------------------------
void ParticleWorld::AddContactGenerator(ParticleContactGenerator* contactGen)
{
	m_contactGens.push_back(contactGen);
}


//-------------------------------------------------------------------------------------------------
void ParticleWorld::AddForceGenerator(ParticleForceGenerator* forceGen, Particle* particleToApplyTo)
{
	// Add the generator and particle of not already added
	if (std::find(m_forceGens.begin(), m_forceGens.end(), forceGen) == m_forceGens.end())
	{
		m_forceGens.push_back(forceGen);
	}

	if (std::find(m_particles.begin(), m_particles.end(), particleToApplyTo) == m_particles.end())
	{
		m_particles.push_back(particleToApplyTo);
	}

	m_forceRegistry.AddRegistration(particleToApplyTo, forceGen);
}


//-------------------------------------------------------------------------------------------------
void ParticleWorld::Integrate(float deltaSeconds)
{
	for (Particle* particle : m_particles)
	{
		particle->Integrate(deltaSeconds);
	}
}


//-------------------------------------------------------------------------------------------------
void ParticleWorld::GenerateContacts()
{
	// Throw away old contacts
	m_numContactsUsed = 0;

	// Let each generator make some new ones
	for (ParticleContactGenerator* generator : m_contactGens)
	{
		int numAdded = generator->GenerateContacts(&m_contacts[m_numContactsUsed], m_maxContacts - m_numContactsUsed);
		m_numContactsUsed += numAdded;

		if (m_numContactsUsed == m_maxContacts)
			break;
	}
}
