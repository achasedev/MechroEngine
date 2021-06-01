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

	CollisionScene<BoundingVolumeClass>();
	~CollisionScene<BoundingVolumeClass>();

	void AddEntity(Entity* entity);
	void RemoveEntity(Entity* entity);

	void AddHalfSpace(HalfSpaceCollider* halfSpace);
	void AddPlane(PlaneCollider* planeCol);

	void DoCollisionStep(float deltaSeconds);
	void DebugRenderBoundingHierarchy() const;
	void DebugRenderLeafBoundingVolumes() const;
	void DebugDrawContacts() const;


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
	static constexpr int MAX_CONTACT_COUNT = 100;


private:
	//-----Private Data-----

	BVHNode<BoundingVolumeClass>*				m_boundingTreeRoot = nullptr;
	std::vector<BVHNode<BoundingVolumeClass>*>	m_leaves;  // Optimization, faster search

	std::vector<HalfSpaceCollider*>				m_halfspaces;
	std::vector<PlaneCollider*>					m_planes;

	PotentialCollision							m_potentialCollisions[MAX_POTENTIAL_COLLISION_COUNT];
	int											m_numPotentialCollisions = 0;

	int											m_numNewContacts = 0;
	Contact*									m_newContacts = nullptr;

	CollisionDetector							m_detector;

	int											m_defaultNumVelocityIterations = 20;
	int											m_defaultNumPenetrationIterations = 20;
	ContactResolver								m_resolver;

};


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void CollisionScene<BoundingVolumeClass>::AddPlane(PlaneCollider* planeCol)
{
	m_planes.push_back(planeCol);
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void CollisionScene<BoundingVolumeClass>::AddHalfSpace(HalfSpaceCollider* halfSpace)
{
	m_halfspaces.push_back(halfSpace);
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
		DebugDrawSphere(contact.position, 0.1f, Rgba::CYAN, 0.f);
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

	for (HalfSpaceCollider* halfSpace : m_halfspaces)
	{
		m_numPotentialCollisions += m_boundingTreeRoot->GetPotentialCollisionsBetween(halfSpace, m_potentialCollisions + m_numPotentialCollisions, MAX_CONTACT_COUNT - m_numPotentialCollisions);

		if (m_numPotentialCollisions == MAX_POTENTIAL_COLLISION_COUNT)
			break;
	}

	if (m_numPotentialCollisions < MAX_POTENTIAL_COLLISION_COUNT)
	{
		for (PlaneCollider* plane : m_planes)
		{
			m_numPotentialCollisions += m_boundingTreeRoot->GetPotentialCollisionsBetween(plane, m_potentialCollisions + m_numPotentialCollisions, MAX_CONTACT_COUNT - m_numPotentialCollisions);

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

		const Collider* colOne = collision.colliders[0];
		const Collider* colTwo = collision.colliders[1];

		bool oneIsSphere = colOne->IsOfType<SphereCollider>();
		bool twoIsSphere = colTwo->IsOfType<SphereCollider>();
		bool oneIsHalfSpace = colOne->IsOfType<HalfSpaceCollider>();
		bool twoIsHalfSpace = colTwo->IsOfType<HalfSpaceCollider>();
		bool oneIsBox = colOne->IsOfType<BoxCollider>();
		bool twoIsBox = colTwo->IsOfType<BoxCollider>();
		bool oneIsCapsule = colOne->IsOfType<CapsuleCollider>();
		bool twoIsCapsule = colTwo->IsOfType<CapsuleCollider>();
		bool oneIsPlane = colOne->IsOfType<PlaneCollider>();
		bool twoIsPlane = colTwo->IsOfType<PlaneCollider>();

		ASSERT_OR_DIE(oneIsSphere || oneIsHalfSpace || oneIsBox || oneIsCapsule || oneIsPlane, "Invalid collider: %s", colOne->GetTypeAsString());
		ASSERT_OR_DIE(twoIsSphere || twoIsHalfSpace || twoIsBox || twoIsCapsule || twoIsPlane, "Invalid collider: %s", colTwo->GetTypeAsString());

		// Don't do halfspace-halfspace collisions
		if ((oneIsHalfSpace && twoIsHalfSpace) || (oneIsPlane && twoIsPlane))
		{
			ERROR_AND_DIE("Dual plane or dual half space collision attempted to generate contacts!");
		}

		//------------------------------------------Spheres------------------------------------------------
		if (oneIsSphere)
		{
			const SphereCollider* oneAsSphere = colOne->GetAsType<SphereCollider>();

			// Sphere/Sphere
			if (twoIsSphere)
			{
				const SphereCollider* twoAsSphere = colTwo->GetAsType<SphereCollider>();
				m_numNewContacts += m_detector.GenerateContacts(*oneAsSphere, *twoAsSphere, &m_newContacts[m_numNewContacts], MAX_CONTACT_COUNT - m_numNewContacts);
			}

			// Sphere/Box
			else if (twoIsBox)
			{
				const BoxCollider* twoAsBox = colTwo->GetAsType<BoxCollider>();
				m_numNewContacts += m_detector.GenerateContacts(*twoAsBox, *oneAsSphere, &m_newContacts[m_numNewContacts], MAX_CONTACT_COUNT - m_numNewContacts);
			}

			// Sphere/Capsule
			else if (twoIsCapsule)
			{
				const CapsuleCollider* twoAsCapsule = colTwo->GetAsType<CapsuleCollider>();
				m_numNewContacts += m_detector.GenerateContacts(*oneAsSphere, *twoAsCapsule, &m_newContacts[m_numNewContacts], MAX_CONTACT_COUNT - m_numNewContacts);
			}

			// Sphere/HalfSpace
			else if (twoIsHalfSpace)
			{
				const HalfSpaceCollider* twoAsHalfSpace = colTwo->GetAsType<HalfSpaceCollider>();
				m_numNewContacts += m_detector.GenerateContacts(*oneAsSphere, *twoAsHalfSpace, &m_newContacts[m_numNewContacts], MAX_CONTACT_COUNT - m_numNewContacts);
			}

			// Sphere/Plane
			else if (twoIsPlane)
			{
				const PlaneCollider* twoAsPlane = colTwo->GetAsType<PlaneCollider>();
				m_numNewContacts += m_detector.GenerateContacts(*oneAsSphere, *twoAsPlane, &m_newContacts[m_numNewContacts], MAX_CONTACT_COUNT - m_numNewContacts);
			}
			else
			{
				ERROR_AND_DIE("Invalid collider pair!");
			}
		}

		//------------------------------------------Boxes------------------------------------------------
		else if (oneIsBox)
		{
			const BoxCollider* oneAsBox = colOne->GetAsType<BoxCollider>();

			// Box/Sphere
			if (twoIsSphere)
			{
				const SphereCollider* twoAsSphere = colTwo->GetAsType<SphereCollider>();
				m_numNewContacts += m_detector.GenerateContacts(*oneAsBox, *twoAsSphere, &m_newContacts[m_numNewContacts], MAX_CONTACT_COUNT - m_numNewContacts);
			}

			// Box/Box
			else if (twoIsBox)
			{
				const BoxCollider* twoAsBox = colTwo->GetAsType<BoxCollider>();
				m_numNewContacts += m_detector.GenerateContacts(*oneAsBox, *twoAsBox, &m_newContacts[m_numNewContacts], MAX_CONTACT_COUNT - m_numNewContacts);
			}

			// Box/Capsule
			else if (twoIsCapsule)
			{
				const CapsuleCollider* twoAsCapsule = colTwo->GetAsType<CapsuleCollider>();
				m_numNewContacts += m_detector.GenerateContacts(*oneAsBox, *twoAsCapsule, &m_newContacts[m_numNewContacts], MAX_CONTACT_COUNT - m_numNewContacts);
			}

			// Box/HalfSpace
			else if (twoIsHalfSpace)
			{
				const HalfSpaceCollider* twoAsHalfSpace = colTwo->GetAsType<HalfSpaceCollider>();
				m_numNewContacts += m_detector.GenerateContacts(*oneAsBox, *twoAsHalfSpace, &m_newContacts[m_numNewContacts], MAX_CONTACT_COUNT - m_numNewContacts);
			}

			// Box/Plane
			else if (twoIsPlane)
			{
				const PlaneCollider* twoAsPlane = colTwo->GetAsType<PlaneCollider>();
				m_numNewContacts += m_detector.GenerateContacts(*oneAsBox, *twoAsPlane, &m_newContacts[m_numNewContacts], MAX_CONTACT_COUNT - m_numNewContacts);
			}
			else
			{
				ERROR_AND_DIE("Invalid collider pair!");
			}
		}

		//------------------------------------------Capsules------------------------------------------------
		else if (oneIsCapsule)
		{
			const CapsuleCollider* oneAsCapsule = colOne->GetAsType<CapsuleCollider>();

			// Capsule/Sphere
			if (twoIsSphere)
			{
				const SphereCollider* twoAsSphere = colTwo->GetAsType<SphereCollider>();
				m_numNewContacts += m_detector.GenerateContacts(*twoAsSphere, *oneAsCapsule, &m_newContacts[m_numNewContacts], MAX_CONTACT_COUNT - m_numNewContacts);
			}

			// Capsule/Box
			else if (twoIsBox)
			{
				const BoxCollider* twoAsBox = colTwo->GetAsType<BoxCollider>();
				m_numNewContacts += m_detector.GenerateContacts(*twoAsBox, *oneAsCapsule, &m_newContacts[m_numNewContacts], MAX_CONTACT_COUNT - m_numNewContacts);
			}

			// Capsule/Capsule
			else if (twoIsCapsule)
			{
				const CapsuleCollider* twoAsCapsule = colTwo->GetAsType<CapsuleCollider>();
				m_numNewContacts += m_detector.GenerateContacts(*oneAsCapsule, *twoAsCapsule, &m_newContacts[m_numNewContacts], MAX_CONTACT_COUNT - m_numNewContacts);
			}

			// Capsule/HalfSpace
			else if (twoIsHalfSpace)
			{
				const HalfSpaceCollider* twoAsHalfSpace = colTwo->GetAsType<HalfSpaceCollider>();
				m_numNewContacts += m_detector.GenerateContacts(*oneAsCapsule, *twoAsHalfSpace, &m_newContacts[m_numNewContacts], MAX_CONTACT_COUNT - m_numNewContacts);
			}

			// Capsule/Plane
			else if (twoIsPlane)
			{
				const PlaneCollider* twoAsPlane = colTwo->GetAsType<PlaneCollider>();
				m_numNewContacts += m_detector.GenerateContacts(*oneAsCapsule, *twoAsPlane, &m_newContacts[m_numNewContacts], MAX_CONTACT_COUNT - m_numNewContacts);
			}
			else
			{
				ERROR_AND_DIE("Invalid collider pair!");
			}
		}

		//------------------------------------------HalfSpaces------------------------------------------------
		else if (oneIsHalfSpace)
		{
			const HalfSpaceCollider* oneAsHalfspace = colOne->GetAsType<HalfSpaceCollider>();

			// HalfSpace/Sphere
			if (twoIsSphere)
			{
				const SphereCollider* twoAsSphere = colTwo->GetAsType<SphereCollider>();
				m_numNewContacts += m_detector.GenerateContacts(*twoAsSphere, *oneAsHalfspace, &m_newContacts[m_numNewContacts], MAX_CONTACT_COUNT - m_numNewContacts);
			}

			// HalfSpace/Box
			else if (twoIsBox)
			{
				const BoxCollider* twoAsBox = colTwo->GetAsType<BoxCollider>();
				m_numNewContacts += m_detector.GenerateContacts(*twoAsBox, *oneAsHalfspace, &m_newContacts[m_numNewContacts], MAX_CONTACT_COUNT - m_numNewContacts);
			}

			// HalfSpace/Capsule
			else if (twoIsCapsule)
			{
				const CapsuleCollider* twoAsCapsule = colTwo->GetAsType<CapsuleCollider>();
				m_numNewContacts += m_detector.GenerateContacts(*twoAsCapsule, *oneAsHalfspace, &m_newContacts[m_numNewContacts], MAX_CONTACT_COUNT - m_numNewContacts);
			}
			else
			{
				ERROR_AND_DIE("Invalid collider pair!");
			}
		}

		//------------------------------------------Boxes------------------------------------------------
		else if (oneIsPlane)
		{
			const PlaneCollider* oneAsPlane = colOne->GetAsType<PlaneCollider>();

			// Plane/Sphere
			if (twoIsSphere)
			{
				const SphereCollider* twoAsSphere = colTwo->GetAsType<SphereCollider>();
				m_numNewContacts += m_detector.GenerateContacts(*twoAsSphere, *oneAsPlane, &m_newContacts[m_numNewContacts], MAX_CONTACT_COUNT - m_numNewContacts);
			}

			// Plane/Box
			else if (twoIsBox)
			{
				const BoxCollider* twoAsBox = colTwo->GetAsType<BoxCollider>();
				m_numNewContacts += m_detector.GenerateContacts(*twoAsBox, *oneAsPlane, &m_newContacts[m_numNewContacts], MAX_CONTACT_COUNT - m_numNewContacts);
			}

			// Plane/Capsule
			else if (twoIsCapsule)
			{
				const CapsuleCollider* twoAsCapsule = colTwo->GetAsType<CapsuleCollider>();
				m_numNewContacts += m_detector.GenerateContacts(*twoAsCapsule, *oneAsPlane, &m_newContacts[m_numNewContacts], MAX_CONTACT_COUNT - m_numNewContacts);
			}
			else
			{
				ERROR_AND_DIE("Invalid collider pair!");
			}
		}
		else
		{
			ERROR_AND_DIE("Invalid collider pair!");
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
	ASSERT_OR_DIE(!entity->collider->IsOfType<HalfSpaceCollider>(), "Cannot add half space entities!");

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
