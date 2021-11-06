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
#include "Engine/Render/Debug/DebugRenderSystem.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
enum CollisionDebugBit : uint32
{
	COLLISION_DEBUG_COLLIDERS = BIT_FLAG(1),
	COLLISION_DEBUG_CONTACTS = BIT_FLAG(2),
	COLLISION_DEBUG_BOUNDING_VOLUMES = BIT_FLAG(3),
	COLLISION_DEBUG_LEAF_BOUNDING_VOLUMES = BIT_FLAG(4)
};

typedef uint32 CollisionDebugFlags;

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

	CollisionScene<BoundingVolumeClass>();
	~CollisionScene<BoundingVolumeClass>();

	void AddEntity(Entity* entity);
	void RemoveEntity(Entity* entity);

	void DoCollisionStep(float deltaSeconds);
	void SetDebugFlags(CollisionDebugFlags flags);


private:
	//-----Private Methods-----

	void UpdateBVH();
	void PerformBroadphase();
	void GenerateContacts();
	void ResolveContacts(float deltaSeconds);

	void ShowDebugColliders();
	void HideDebugColliders();
	void DebugDrawBoundingVolumeHierarchy() const;
	void DebugDrawLeafBoundingVolumes() const;
	void DebugDrawContacts() const;

	void UpdateNode(BVHNode<BoundingVolumeClass>* node, const BoundingVolumeClass& newVolume);
	BVHNode<BoundingVolumeClass>* GetAndEraseLeafNodeForEntity(Entity* entity);
	BoundingVolumeClass MakeBoundingVolumeForPrimitive(const Collider* primitive) const;


private:
	//-----Private Static Data

	static constexpr int MAX_POTENTIAL_COLLISION_COUNT = 50;
	static constexpr int MAX_CONTACT_COUNT = 100;


private:
	//-----Private Data-----

	BVHNode<BoundingVolumeClass>*				m_boundingTreeRoot = nullptr;
	std::vector<BVHNode<BoundingVolumeClass>*>	m_leaves;  // Optimization, faster search

	std::vector<HalfSpaceCollider*>				m_halfSpaces;
	std::vector<PlaneCollider*>					m_planes;

	PotentialCollision							m_potentialCollisions[MAX_POTENTIAL_COLLISION_COUNT];
	int											m_numPotentialCollisions = 0;

	int											m_numNewContacts = 0;
	Contact*									m_newContacts = nullptr;

	CollisionDetector							m_detector;

	int											m_defaultNumVelocityIterations = 20;
	int											m_defaultNumPenetrationIterations = 20;
	ContactResolver								m_resolver;

	// Debug
	CollisionDebugFlags							m_debugFlags = 0;

};


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void CollisionScene<BoundingVolumeClass>::SetDebugFlags(CollisionDebugFlags flags)
{
	m_debugFlags = flags;

	if (AreBitsSet(m_debugFlags, COLLISION_DEBUG_COLLIDERS))
	{
		ShowDebugColliders();
	}
	else
	{
		HideDebugColliders();
	}
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void CollisionScene<BoundingVolumeClass>::HideDebugColliders()
{
	for (BVHNode<BoundingVolumeClass>* leaf : m_leaves)
	{
		leaf->m_entity->collider->HideDebug();
	}

	for (HalfSpaceCollider* halfSpace : m_halfSpaces)
	{
		halfSpace->HideDebug();
	}

	for (PlaneCollider* plane : m_planes)
	{
		plane->HideDebug();
	}
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void CollisionScene<BoundingVolumeClass>::ShowDebugColliders()
{
	for (BVHNode<BoundingVolumeClass>* leaf : m_leaves)
	{
		leaf->m_entity->collider->ShowDebug();
	}

	for (HalfSpaceCollider* halfSpace : m_halfSpaces)
	{
		halfSpace->ShowDebug();
	}

	for (PlaneCollider* plane : m_planes)
	{
		plane->ShowDebug();
	}
}

//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
CollisionScene<BoundingVolumeClass>::CollisionScene()
{
	m_newContacts = (Contact*)malloc(sizeof(Contact) * MAX_CONTACT_COUNT);

	for (int i = 0; i < MAX_CONTACT_COUNT; ++i)
	{
		m_newContacts[i] = Contact();
	}
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void CollisionScene<BoundingVolumeClass>::DebugDrawContacts() const
{
	for (int i = 0; i < m_numNewContacts; ++i)
	{
		const Contact& contact = m_newContacts[i];

		DebugRenderOptions options;
		options.m_startColor = Rgba::RED;
		options.m_endColor = Rgba::RED;
		options.m_lifetime = 0.f;
		options.m_fillMode = FILL_MODE_WIREFRAME;
		options.m_debugRenderMode = DEBUG_RENDER_MODE_XRAY;

		DebugDrawSphere(contact.position, 0.05f, options);
	}
}


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
	else if (primitive->IsOfType<CapsuleCollider>())
	{
		const CapsuleCollider* primAsCapsule = primitive->GetAsType<CapsuleCollider>();
		return BoundingVolumeClass(*primAsCapsule);
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
		BoundingVolumeClass currVolumeWs = MakeBoundingVolumeForPrimitive(node->m_entity->collider);

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
	m_numPotentialCollisions = 0;

	for (HalfSpaceCollider* halfSpace : m_halfSpaces)
	{
		m_numPotentialCollisions += m_boundingTreeRoot->GetPotentialCollisionsBetween(halfSpace, m_potentialCollisions + m_numPotentialCollisions, MAX_POTENTIAL_COLLISION_COUNT - m_numPotentialCollisions);

		if (m_numPotentialCollisions == MAX_POTENTIAL_COLLISION_COUNT)
			break;
	}

	if (m_numPotentialCollisions < MAX_POTENTIAL_COLLISION_COUNT)
	{
		for (PlaneCollider* plane : m_planes)
		{
			m_numPotentialCollisions += m_boundingTreeRoot->GetPotentialCollisionsBetween(plane, m_potentialCollisions + m_numPotentialCollisions, MAX_POTENTIAL_COLLISION_COUNT - m_numPotentialCollisions);

			if (m_numPotentialCollisions == MAX_POTENTIAL_COLLISION_COUNT)
				break;
		}
	}

	if (m_numPotentialCollisions < MAX_POTENTIAL_COLLISION_COUNT)
	{
		m_numPotentialCollisions += m_boundingTreeRoot->GetPotentialNodeCollisions(m_potentialCollisions + m_numPotentialCollisions, MAX_POTENTIAL_COLLISION_COUNT - m_numPotentialCollisions);
	}

	if (m_numPotentialCollisions == MAX_POTENTIAL_COLLISION_COUNT)
	{
		ConsoleWarningf("Collision scene hit the limit for number of potential collisions per frame at: %i", m_numPotentialCollisions);
	}
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void CollisionScene<BoundingVolumeClass>::GenerateContacts()
{
	m_numNewContacts = 0;

	for (int i = 0; i < m_numPotentialCollisions; ++i)
	{
		if (m_numNewContacts >= MAX_CONTACT_COUNT)
		{
			ConsoleWarningf("CollisionDetector ran out of room for contacts!");
			return;
		}

		PotentialCollision& collision = m_potentialCollisions[i];

		const Collider* a = collision.colliders[0];
		const Collider* b = collision.colliders[1];

		// Don't generate contacts between colliders without bodies, sleeping bodies, or static bodies
		// At least 1 collider needs to be an awake, movable entity to make the work here worth it
		// Otherwise we're generating contacts we're dong nothing with
		// This will need to be updated when overlap volumes come into play...
		bool aDoesntNeedContacts = a->m_entity->rigidBody == nullptr || !a->m_entity->rigidBody->IsAwake() || a->m_entity->rigidBody->IsStatic();
		bool bDoesntNeedContacts = b->m_entity->rigidBody == nullptr || !b->m_entity->rigidBody->IsAwake() || b->m_entity->rigidBody->IsStatic();

		if (!(aDoesntNeedContacts && bDoesntNeedContacts))
		{
			m_numNewContacts += m_detector.GenerateContacts(a, b, &m_newContacts[m_numNewContacts], MAX_CONTACT_COUNT - m_numNewContacts);
		}
	}
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void CollisionScene<BoundingVolumeClass>::ResolveContacts(float deltaSeconds)
{
	if (m_numNewContacts > 0)
	{	
		m_resolver.SetMaxVelocityIterations(Min(m_defaultNumVelocityIterations, 2 * m_numNewContacts));
		m_resolver.SetMaxPenetrationIterations(Min(m_defaultNumPenetrationIterations, 2 * m_numNewContacts));
		m_resolver.ResolveContacts(m_newContacts, m_numNewContacts, deltaSeconds);
	}
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void CollisionScene<BoundingVolumeClass>::DebugDrawBoundingVolumeHierarchy() const
{
	if (m_boundingTreeRoot != nullptr)
	{
		m_boundingTreeRoot->DebugRender();
	}
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void CollisionScene<BoundingVolumeClass>::DebugDrawLeafBoundingVolumes() const
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
	ASSERT_OR_DIE(entity != nullptr, "Null entity!");
	ASSERT_OR_DIE(entity->collider != nullptr, "Null collider!");

	if (entity->collider->IsOfType<HalfSpaceCollider>())
	{
		m_halfSpaces.push_back(entity->collider->GetAsType<HalfSpaceCollider>());
	}
	else if (entity->collider->IsOfType<PlaneCollider>())
	{
		m_planes.push_back(entity->collider->GetAsType<PlaneCollider>());
	}
	else
	{
		BoundingVolumeClass boundingVolume = MakeBoundingVolumeForPrimitive(entity->collider);
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

	// Ensure we create the debug draw for the collider
	if (AreBitsSet(m_debugFlags, COLLISION_DEBUG_COLLIDERS))
	{
		entity->collider->ShowDebug();
	}
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void CollisionScene<BoundingVolumeClass>::RemoveEntity(Entity* entity)
{
	ASSERT_OR_DIE(entity != nullptr, "Null entity!");
	ASSERT_OR_DIE(entity->collider != nullptr, "Null collider!");

	if (entity->collider->IsOfType<HalfSpaceCollider>())
	{
		for (int i = 0; i < m_halfSpaces.size(); ++i)
		{
			if (m_halfSpaces[i] == entity->collider)
			{
				m_halfSpaces.erase(m_halfSpaces.begin() + i);
				break;
			}
		}
	}
	else if (entity->collider->IsOfType<PlaneCollider>())
	{
		for (int i = 0; i < m_planes.size(); ++i)
		{
			if (m_planes[i] == entity->collider)
			{
				m_planes.erase(m_planes.begin() + i);
				break;
			}
		}
	}
	else
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

	// Debug
	if (AreBitsSet(m_debugFlags, COLLISION_DEBUG_CONTACTS))
	{
		DebugDrawContacts();
	}

	if (AreBitsSet(m_debugFlags, COLLISION_DEBUG_BOUNDING_VOLUMES))
	{
		DebugDrawBoundingVolumeHierarchy();
	}
	else if (AreBitsSet(m_debugFlags, COLLISION_DEBUG_LEAF_BOUNDING_VOLUMES))
	{
		DebugDrawLeafBoundingVolumes();
	}
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void CollisionScene<BoundingVolumeClass>::UpdateNode(BVHNode<BoundingVolumeClass>* node, const BoundingVolumeClass& newVolume)
{
	if (node->m_parent != nullptr)
	{
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
	else
	{
		node->m_boundingVolumeWs = newVolume;
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
