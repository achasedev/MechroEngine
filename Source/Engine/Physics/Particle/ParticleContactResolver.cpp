///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 26th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Physics/Particle/ParticleContact.h"
#include "Engine/Physics/Particle/ParticleContactResolver.h"

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
ParticleContactResolver::ParticleContactResolver(int maxIterations)
	: m_maxIterations(maxIterations)
{
}


//-------------------------------------------------------------------------------------------------
// Solves all velocities, then positions. These groups can be done in either order, since they are independent of each other
// Doing them separately allows us to solve each group in the optimal order (by min separating velocity and max penetrations)
void ParticleContactResolver::ResolveContacts(ParticleContact* contacts, int numContacts, float deltaSeconds)
{
	// Velocities!
	for (int iteration = 0; iteration < m_maxIterations; ++iteration)
	{
		// Find the greatest closing velocity (most negative separating velocity)
		float minSepVelocity = FLT_MAX;
		int minIndex = -1;

		for (int contactIndex = 0; contactIndex < numContacts; ++contactIndex)
		{
			float sepVelocity = contacts[contactIndex].CalculateSeparatingVelocity();
			if (sepVelocity < minSepVelocity && sepVelocity < 0.f)
			{
				minSepVelocity = sepVelocity;
				minIndex = contactIndex;
			}
		}

		if (minIndex < 0)
			break;

		contacts[minIndex].ResolveVelocity(deltaSeconds);
	}

	// Positions!
	for (int iteration = 0; iteration < m_maxIterations; ++iteration)
	{
		// Find the greatest closing velocity (most negative separating velocity)
		float maxPenetration = -1.0f;
		int maxIndex = -1;

		for (int contactIndex = 0; contactIndex < numContacts; ++contactIndex)
		{
			float pen = contacts[contactIndex].GetPenetration();
			if (pen > maxPenetration && pen > 0.f)
			{
				maxPenetration = pen;
				maxIndex = contactIndex;
			}
		}

		if (maxIndex < 0)
			break;
		

		ParticleContact* maxContact = &contacts[maxIndex];

		Vector3 correctionA;
		Vector3 correctionB;

		maxContact->ResolveInterpenetration(correctionA, correctionB);

		bool aCorrected = !AreMostlyEqual(correctionA, Vector3::ZERO);
		bool bCorrected = !AreMostlyEqual(correctionB, Vector3::ZERO);

		if (!aCorrected && !bCorrected)
			continue;

		// Update all other contacts' penetrations that may have been changed by the correction
		for (int contactIndex = 0; contactIndex < numContacts; ++contactIndex)
		{
			if (contactIndex == maxIndex)
				continue;

			ParticleContact* contact = &contacts[contactIndex];

			if (aCorrected)
			{
				if (contact->m_particleA == maxContact->m_particleA)
				{
					float moveAlongNormal = DotProduct(contact->m_normal, correctionA);
					contact->m_penetration -= moveAlongNormal;
				}
				else if (contact->m_particleA == maxContact->m_particleB)
				{
					float moveAlongNormal = DotProduct(contact->m_normal, correctionA);
					contact->m_penetration += moveAlongNormal;
				}
			}

			if (bCorrected)
			{
				if (contact->m_particleB == maxContact->m_particleA)
				{
					float moveAlongNormal = DotProduct(contact->m_normal, correctionB);
					contact->m_penetration -= moveAlongNormal;
				}
				else if (contact->m_particleB == maxContact->m_particleB)
				{
					float moveAlongNormal = DotProduct(contact->m_normal, correctionB);
					contact->m_penetration += moveAlongNormal;
				}
			}
		}
	}
}
