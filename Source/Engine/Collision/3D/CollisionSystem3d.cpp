///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 18th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Framework/Entity.h"
#include "Engine/Collision/3D/Collider3d.h"
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
void CollisionSystem3d::RemoveCollider(const Collider3d* collider)
{
	bool colliderFound = false;
	int numColliders = (int)m_colliders.size();

	for (int colliderIndex = 0; colliderIndex < numColliders; ++colliderIndex)
	{
		if (m_colliders[colliderIndex] == collider)
		{
			m_colliders.erase(m_colliders.begin() + colliderIndex);
			colliderFound = true;
			break;
		}
	}

	ASSERT_RECOVERABLE(colliderFound, "Collider not found!");
}


//-------------------------------------------------------------------------------------------------
const BoxCollider3d* CollisionSystem3d::AddEntity(Entity* entity, const OBB3& colliderBounds)
{
	BoxCollider3d* boxCol = new BoxCollider3d(colliderBounds);
	boxCol->m_owner = entity;
	boxCol->m_transform.SetParentTransform(&entity->transform);
	entity->m_collider = boxCol;

	AddCollider(boxCol);
	return boxCol;
}


//-------------------------------------------------------------------------------------------------
void CollisionSystem3d::RemoveEntity(Entity* entity)
{
	if (entity->m_collider != nullptr)
	{
		RemoveCollider(entity->m_collider);
		SAFE_DELETE(entity->m_collider);
	}
}


//-------------------------------------------------------------------------------------------------
void CollisionSystem3d::PerformBroadPhase()
{
	// O(n^2) broad-phase
	// TODO: Make this less garbage
	int numColliders = (int)m_colliders.size();
	for (int i = 0; i < numColliders - 1; ++i)
	{
		Collider3d* colA = m_colliders[i];

		for (int j = i + 1; j < numColliders; ++j)
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


//-------------------------------------------------------------------------------------------------
void CollisionSystem3d::PerformNarrowPhase()
{
	Manifold3dIter itr = m_manifolds.begin();
	for (itr; itr != m_manifolds.end(); itr++)
	{
		itr->second.GenerateContacts();
	}
}


//-------------------------------------------------------------------------------------------------
const ContactManifold3d* CollisionSystem3d::GetManifoldForColliders(Collider3d* a, Collider3d* b) const
{
	ManifoldKey3D key(a, b);
	bool manifoldExists = m_manifolds.find(key) != m_manifolds.end();

	if (manifoldExists)
	{
		return &m_manifolds.at(key);
	}

	return nullptr;
}
