///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 24th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Physics/Particle/Particle.h"
#include "Engine/Physics/Particle/ParticleBuoyancy.h"

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
ParticleBuoyancy::ParticleBuoyancy(float maxDepth, float objectVolume, float liquidAltitude /*= 0.f*/, float liquidDensity /*= 1000.f*/)
	: m_maxDepth(maxDepth)
	, m_objectVolume(objectVolume)
	, m_liquidAltitude(liquidAltitude)
	, m_liquidDensity(liquidDensity)
{
}

#include "Engine/Core/DevConsole.h"
//-------------------------------------------------------------------------------------------------
void ParticleBuoyancy::GenerateAndApplyForce(Particle* particle, float deltaSeconds) const
{
	float objectAltitude = particle->GetPosition().y;

	// If the object is completely out of the water, do nothing
	if (objectAltitude >= m_liquidAltitude + m_maxDepth)
		return;

	float magnitude = 0.f;
	if (objectAltitude <= m_liquidAltitude - m_maxDepth)
	{
		// If we're fully submerged, we apply the full buoyant force regardless of any "extra" depth we have
		magnitude = m_liquidDensity * m_objectVolume;
	}
	else
	{
		// Not fully submerged, so we add a fraction of the force proportional to how much we're submerged
		float fraction = RangeMapFloat(objectAltitude, m_liquidAltitude - m_maxDepth, m_liquidAltitude + m_maxDepth, 1.f, 0.f);
		magnitude = (m_liquidDensity * m_objectVolume) * fraction;
	}

	ConsolePrintf("%.2f", magnitude);
	particle->AddForce(Vector3(0.f, magnitude, 0.f));
}
