///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 18th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Collision/3D/CollisionSystem3d.h"

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
void CollisionSystem3d::AddCollider(Collider3d* collider)
{
#ifndef DISABLE_ASSERTS
	// Check for duplicates
	int numColliders = (int)m_colliders.size();
	for (int colliderIndex = 0; colliderIndex < numColliders; ++colliderIndex)
	{
		ASSERT_RETURN(m_colliders[colliderIndex] != collider, NO_RETURN_VAL, "Duplicate collider!");
	}
#endif

	m_colliders.push_back(collider);
}


//-------------------------------------------------------------------------------------------------
void CollisionSystem3d::RemoveCollider(Collider3d* collider)
{
	bool colliderFound = false;
	int numColliders = (int)m_colliders.size();

	for (int colliderIndex = 0; colliderIndex < numColliders; ++colliderIndex)
	{
		if (m_colliders[colliderIndex] == collider)
		{
			m_colliders.erase(m_colliders.begin() + colliderIndex);
			colliderFound = true;
		}
	}

	ASSERT_RECOVERABLE(colliderFound, "Collider not found!");
}


//-------------------------------------------------------------------------------------------------
void CollisionSystem3d::PerformBroadPhase()
{
	// O(n^2) broad-phase
	// TODO: Make this less garbage
	int numColliders = (int)m_colliders.size();
	for (int i = 0; i < m_colliders.size() - 1; ++i)
	{
		Collider3d* colA = m_colliders[i];

		for (int j = i + 1; j < (int)m_colliders.size(); ++j)
		{
			Collider3d* colB = m_colliders[j];

			ContactManifold3d manifold = ContactManifold3d(colA, colB);
			manifold.Collide();

			ManifoldKey3D key(colA, colB);

			if (manifold.HasCollision())
			{
				m_manifolds[key] = manifold;
			}
			else
			{
				m_manifolds.erase(key);
			}
		}
	}
}
