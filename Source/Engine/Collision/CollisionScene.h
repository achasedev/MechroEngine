///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: May 7th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include <vector>
#include "Engine/Collision/BoundingVolumeHierarchy/BVHNode.h"
#include "Engine/Collision/CollisionDetector.h"
#include "Engine/Collision/Contact.h"
#include "Engine/Collision/ContactResolver.h"
#include "Engine/Core/DevConsole.h"
#include "Engine/Core/Entity.h"
#include "Engine/Math/MathUtils.h"

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
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
class CollisionScene
{
public:
	//-----Public Methods-----

	CollisionScene<BoundingVolumeClass>() {}
	~CollisionScene<BoundingVolumeClass>();

	void AddEntity(Entity* entity);
	void RemoveEntity(Entity* entity);

	void DoCollisionStep(float deltaSeconds);
	void DebugRenderBoundingHierarchy() const;
	void DebugRenderLeafBoundingVolumes() const;


private:
	//-----Private Methods-----

	void UpdateBVH();
	void PerformBroadphase();
	void GenerateContacts();
	void ResolveContacts(float deltaSeconds);

	void UpdateNode(BVHNode<BoundingVolumeClass>* node, const BoundingVolumeClass& newVolume);
	BVHNode<BoundingVolumeClass>* GetAndEraseLeafNodeForEntity(Entity* entity);
	BoundingVolumeClass MakeBoundingVolumeForPrimitive(const Collider* primitive) const;


private:
	//-----Private Static Data

	static constexpr int MAX_POTENTIAL_COLLISION_COUNT = 50;


private:
	//-----Private Data-----

	BVHNode<BoundingVolumeClass>*				m_boundingTreeRoot = nullptr;
	std::vector<BVHNode<BoundingVolumeClass>*>	m_leaves;  // Optimization, faster search

	PotentialCollision							m_potentialCollisions[MAX_POTENTIAL_COLLISION_COUNT];
	int											m_numPotentialCollisions = 0;

	CollisionData								m_collisionData;
	CollisionDetector							m_detector;
	ContactResolver								m_resolver;

};


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
BoundingVolumeClass CollisionScene<BoundingVolumeClass>::MakeBoundingVolumeForPrimitive(const Collider* primitive) const
{
	if (primitive->IsOfType<SphereCollider>())
	{
		const SphereCollider* primAsSphere = primitive->GetAsType<SphereCollider>();
		return BoundingVolumeClass(*primAsSphere);
	}
	else if (primitive->IsOfType<BoxCollider>())
	{
		const BoxCollider* primAsBox = primitive->GetAsType<BoxCollider>();
		return BoundingVolumeClass(*primAsBox);
	}
	else if (primitive->IsOfType<HalfSpaceCollider>())
	{
		const HalfSpaceCollider* primAsHalfSpace = primitive->GetAsType<HalfSpaceCollider>();
		return BoundingVolumeClass(*primAsHalfSpace);
	}
	else
	{
		ERROR_AND_DIE("Unsupported primitive type: %s", primitive->GetTypeAsString());
	}
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void CollisionScene<BoundingVolumeClass>::UpdateBVH()
{
	// For each node, check if the entity's bounding volume changed a significant amount. If so, update it
	for (BVHNode<BoundingVolumeClass>* node : m_leaves)
	{
		BoundingVolumeClass currVolumeWs = MakeBoundingVolumeForPrimitive(node->m_entity->collisionPrimitive);

		if (!AreMostlyEqual(node->m_boundingVolumeWs, currVolumeWs))
		{
			UpdateNode(node, currVolumeWs);
		}
	}
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void CollisionScene<BoundingVolumeClass>::PerformBroadphase()
{
	m_numPotentialCollisions = m_boundingTreeRoot->GetPotentialCollisions(m_potentialCollisions, MAX_POTENTIAL_COLLISION_COUNT);
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void CollisionScene<BoundingVolumeClass>::GenerateContacts()
{
	m_collisionData.numContacts = 0;

	for (int i = 0; i < m_numPotentialCollisions; ++i)
	{
		if (m_collisionData.numContacts >= MAX_CONTACT_COUNT)
		{
			ConsoleWarningf("CollisionDetector ran out of room for contacts!");
			return;
		}

		PotentialCollision& collision = m_potentialCollisions[i];

		Collider* colOne = collision.entities[0]->collisionPrimitive;
		Collider* colTwo = collision.entities[1]->collisionPrimitive;

		bool oneIsSphere = colOne->IsOfType<SphereCollider>();
		bool twoIsSphere = colTwo->IsOfType<SphereCollider>();
		bool oneIsHalfSpace = colOne->IsOfType<HalfSpaceCollider>();
		bool twoIsHalfSpace = colTwo->IsOfType<HalfSpaceCollider>();
		bool oneIsBox = colOne->IsOfType<BoxCollider>();
		bool twoIsBox = colTwo->IsOfType<BoxCollider>();

		ASSERT_OR_DIE(oneIsSphere || oneIsHalfSpace || oneIsBox, "Invalid collider: %s", colOne->GetTypeAsString());
		ASSERT_OR_DIE(twoIsSphere || twoIsHalfSpace || twoIsBox, "Invalid collider: %s", colTwo->GetTypeAsString());

		// Don't do halfspace-halfspace collisions
		if (oneIsHalfSpace && twoIsHalfSpace)
			continue;

		if (oneIsSphere)
		{
			SphereCollider* oneAsSphere = colOne->GetAsType<SphereCollider>();

			if (twoIsSphere)
			{
				SphereCollider* twoAsSphere = colTwo->GetAsType<SphereCollider>();
				m_detector.GenerateContacts(*oneAsSphere, *twoAsSphere, m_collisionData);
			}
			else if (twoIsHalfSpace)
			{
				HalfSpaceCollider* twoAsHalfSpace = colTwo->GetAsType<HalfSpaceCollider>();
				m_detector.GenerateContacts(*oneAsSphere, *twoAsHalfSpace, m_collisionData);
			}
			else if (twoIsBox)
			{
				BoxCollider* twoAsBox = colTwo->GetAsType<BoxCollider>();
				m_detector.GenerateContacts(*twoAsBox, *oneAsSphere, m_collisionData);
			}
		}
		else if (oneIsBox)
		{
			BoxCollider* oneAsBox = colOne->GetAsType<BoxCollider>();

			if (twoIsSphere)
			{
				SphereCollider* twoAsSphere = colTwo->GetAsType<SphereCollider>();
				m_detector.GenerateContacts(*oneAsBox, *twoAsSphere, m_collisionData);
			}
			else if (twoIsHalfSpace)
			{
				HalfSpaceCollider* twoAsHalfSpace = colTwo->GetAsType<HalfSpaceCollider>();
				m_detector.GenerateContacts(*oneAsBox, *twoAsHalfSpace, m_collisionData);
			}
			else if (twoIsBox)
			{
				BoxCollider* twoAsBox = colTwo->GetAsType<BoxCollider>();
				m_detector.GenerateContacts(*oneAsBox, *twoAsBox, m_collisionData);
			}
		}
		else if (oneIsHalfSpace)
		{
			HalfSpaceCollider* oneAsHalfSpace = colOne->GetAsType<HalfSpaceCollider>();

			if (twoIsSphere)
			{
				SphereCollider* twoAsSphere = colTwo->GetAsType<SphereCollider>();
				m_detector.GenerateContacts(*twoAsSphere, *oneAsHalfSpace, m_collisionData);
			}
			else if (twoIsBox)
			{
				BoxCollider* twoAsBox = colTwo->GetAsType<BoxCollider>();
				m_detector.GenerateContacts(*twoAsBox, *oneAsHalfSpace, m_collisionData);
			}
		}
	}
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void CollisionScene<BoundingVolumeClass>::ResolveContacts(float deltaSeconds)
{
	if (m_collisionData.numContacts > 0)
	{
		m_resolver.ResolveContacts(m_collisionData.contacts, m_collisionData.numContacts, deltaSeconds);
	}
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void CollisionScene<BoundingVolumeClass>::DebugRenderBoundingHierarchy() const
{
	if (m_boundingTreeRoot != nullptr)
	{
		m_boundingTreeRoot->DebugRender();
	}
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void CollisionScene<BoundingVolumeClass>::DebugRenderLeafBoundingVolumes() const
{
	for (BVHNode<BoundingVolumeClass>* leaf : m_leaves)
	{
		leaf->m_boundingVolumeWs.DebugRender();
	}
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
CollisionScene<BoundingVolumeClass>::~CollisionScene()
{
	ASSERT_OR_DIE(m_boundingTreeRoot == nullptr, "Tree wasn't cleaned up before deleting!");
	ASSERT_OR_DIE(m_leaves.size() == 0, "Levaes weren't cleaned up properly!");
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void CollisionScene<BoundingVolumeClass>::AddEntity(Entity* entity)
{
	BoundingVolumeClass boundingVolume = MakeBoundingVolumeForPrimitive(entity->collisionPrimitive);
	BVHNode<BoundingVolumeClass>* node = new BVHNode<BoundingVolumeClass>(boundingVolume);
	node->m_entity = entity;

	if (m_boundingTreeRoot != nullptr)
	{
		BVHNode<BoundingVolumeClass>* newRoot = m_boundingTreeRoot->Insert(node);
		if (newRoot != nullptr)
		{
			m_boundingTreeRoot = newRoot;
		}
	}
	else
	{
		m_boundingTreeRoot = node;
	}

	m_leaves.push_back(node);
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void CollisionScene<BoundingVolumeClass>::RemoveEntity(Entity* entity)
{
	BVHNode<BoundingVolumeClass>* node = GetAndEraseLeafNodeForEntity(entity);

	if (node != m_boundingTreeRoot)
	{
		BVHNode<BoundingVolumeClass>* newRoot = node->RemoveSelf();
		if (newRoot != nullptr)
		{
			m_boundingTreeRoot = newRoot;
		}

		SAFE_DELETE(node);
	}
	else
	{
		SAFE_DELETE(m_boundingTreeRoot);
	}
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void CollisionScene<BoundingVolumeClass>::DoCollisionStep(float deltaSeconds)
{
	// Ensure the BVH is up to date, then get the potential collisions
	UpdateBVH();
	PerformBroadphase();
	GenerateContacts();
	ResolveContacts(deltaSeconds);
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void CollisionScene<BoundingVolumeClass>::UpdateNode(BVHNode<BoundingVolumeClass>* node, const BoundingVolumeClass& newVolume)
{
	ASSERT_OR_DIE(node->m_parent != nullptr, "No Parent!");

	// Remove the node from the tree, recursively updating bounding volumes
	BVHNode<BoundingVolumeClass>* newRoot = node->RemoveSelf();
	if (newRoot != nullptr)
	{
		m_boundingTreeRoot = newRoot;
	}

	// Set the node's new bounding volume
	node->m_boundingVolumeWs = newVolume;

	// Re-insert the node into the tree, based on "best fit"
	newRoot = m_boundingTreeRoot->Insert(node);
	if (newRoot != nullptr)
	{
		m_boundingTreeRoot = newRoot;
	}
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
BVHNode<BoundingVolumeClass>* CollisionScene<BoundingVolumeClass>::GetAndEraseLeafNodeForEntity(Entity* entity)
{
	BVHNode<BoundingVolumeClass>* node = nullptr;

	for (int i = 0; i < (int)m_leaves.size(); ++i)
	{
		if (m_leaves[i]->m_entity == entity)
		{
			node = m_leaves[i];
			m_leaves.erase(m_leaves.begin() + i);
			break;
		}
	}

	return node;
}

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
