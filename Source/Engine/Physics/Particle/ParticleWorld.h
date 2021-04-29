///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 28th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include <vector>
#include "Engine/Physics/Particle/ParticleContactResolver.h"
#include "Engine/Physics/Particle/ParticleForceRegistry.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Particle;
class ParticleContact;
class ParticleContactGenerator;
class ParticleForceGenerator;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class ParticleWorld
{
public:
	//-----Public Methods-----

	ParticleWorld(int numIterations, int maxContacts);
	~ParticleWorld();

	void DoPhysicsStep(float deltaSeconds);
	void DebugDrawParticles();

	void AddParticle(Particle* particle);
	void AddContactGenerator(ParticleContactGenerator* contactGen);
	void AddForceGenerator(ParticleForceGenerator* forceGen, Particle* particleToApplyTo);



private:
	//-----Private Methods-----

	void Integrate(float deltaSeconds);
	void GenerateContacts();


private:
	//-----Private Data-----

	std::vector<Particle*> m_particles;
	std::vector<ParticleForceGenerator*> m_forceGens;
	ParticleForceRegistry m_forceRegistry;
	std::vector<ParticleContactGenerator*> m_contactGens;
	int m_numContactsUsed = 0;
	int m_maxContacts = 0;
	int m_defaultNumIterations = 0;
	ParticleContact* m_contacts = nullptr;
	ParticleContactResolver m_resolver;

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
