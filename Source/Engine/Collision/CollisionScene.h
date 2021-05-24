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

	// Coherency
	void FinalizeContacts();
	void IncrementCoherentContactAges();
	void MoveNewContactsToCoherentArray();
	void RefreshOrRemoveCoherentContacts();


private:
	//-----Private Static Data

	static constexpr int MAX_POTENTIAL_COLLISION_COUNT = 50;
	static constexpr int MAX_CONTACT_COUNT = 50;


private:
	//-----Private Data-----

	BVHNode<BoundingVolumeClass>*				m_boundingTreeRoot = nullptr;
	std::vector<BVHNode<BoundingVolumeClass>*>	m_leaves;  // Optimization, faster search

	PotentialCollision							m_potentialCollisions[MAX_POTENTIAL_COLLISION_COUNT];
	int											m_numPotentialCollisions = 0;

	bool										m_isDoingCoherency = false;
	int											m_numNewContacts = 0;
	Contact*									m_newContacts = nullptr;
	int											m_numCoherentContacts = 0;
	Contact*									m_coherentContacts = nullptr;

	CollisionDetector							m_detector;

	int											m_defaultNumVelocityIterations = 100;
	int											m_defaultNumPenetrationIterations = 100;

	ContactResolver								m_resolver;

};


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void CollisionScene<BoundingVolumeClass>::RefreshOrRemoveCoherentContacts()
{
	const int maxAge = 5;
	const float penLimit = -2.f * m_resolver.GetPenetrationEpsilon();
	for (int i = m_numCoherentContacts - 1; i >= 0; --i)
	{
		Contact& contact = m_coherentContacts[i];
		ASSERT_OR_DIE(contact.isValid, "Coherent contact array is fragmented!");

		// Don't update new contacts
		if (contact.ageInFrames > 0)
		{
			if (contact.featureRecord.GetType() != CONTACT_RECORD_INVALID)
			{
				if (contact.ageInFrames > 0)
				{
					m_detector.RefreshContact(&contact); // We can't tell if this contact isn't penning anymore without refreshing first

					if (contact.ageInFrames > maxAge || contact.penetration < penLimit)
					{
						if (i < m_numCoherentContacts - 1)
						{
							contact = m_coherentContacts[m_numCoherentContacts - 1];
						}

						m_coherentContacts[m_numCoherentContacts - 1].isValid = false;
						m_numCoherentContacts--;
					}
				}
			}
			else
			{
				// Old contact that can't be refreshed, so just delete it
				if (i < m_numCoherentContacts - 1)
				{
					contact = m_coherentContacts[m_numCoherentContacts - 1];
				}

				m_coherentContacts[m_numCoherentContacts - 1].isValid = false;
				m_numCoherentContacts--;
			}
		}
	}
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void CollisionScene<BoundingVolumeClass>::MoveNewContactsToCoherentArray()
{
	ASSERT_OR_DIE(m_isDoingCoherency, "CollisionScene isn't doing coherency!");

	for (int newContactIndex = 0; newContactIndex < m_numNewContacts; ++newContactIndex)
	{
		Contact& newContact = m_newContacts[newContactIndex];
		bool foundMatch = false;

		// Find if this contact already exists from a recent collision check
		if (newContact.featureRecord.GetType() != CONTACT_RECORD_INVALID)
		{
			for (int coherentContactIndex = 0; coherentContactIndex < m_numCoherentContacts; ++coherentContactIndex)
			{
				if (m_coherentContacts[coherentContactIndex].isValid)
				{
					if (m_coherentContacts[coherentContactIndex].featureRecord == newContact.featureRecord)
					{
						// Update in place
						m_coherentContacts[coherentContactIndex] = newContact;
						newContact.isValid = false;
						ASSERT_OR_DIE(m_coherentContacts[coherentContactIndex].ageInFrames == 0, "Bad age!"); // Sanity check
						foundMatch = true;
						break;
					}
				}
			}
		}

		if (!foundMatch)
		{
			// It's a brand new contact, so add it to the array
			if (m_numCoherentContacts < MAX_CONTACT_COUNT)
			{
				m_coherentContacts[m_numCoherentContacts] = newContact;
				m_numCoherentContacts++;
			}
			else
			{
				ConsoleWarningf("Ran out of room for coherent contacts! Had to drop a contact");
			}

			newContact.isValid = false;
		}
	}

	m_numNewContacts = 0;
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void CollisionScene<BoundingVolumeClass>::IncrementCoherentContactAges()
{
	ASSERT_OR_DIE(m_isDoingCoherency, "CollisionScene isn't doing coherency!");

	for (int i = 0; i < m_numCoherentContacts; ++i)
	{
		if (m_coherentContacts[i].isValid)
		{
			m_coherentContacts[i].ageInFrames++;
		}
	}
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void CollisionScene<BoundingVolumeClass>::FinalizeContacts()
{
	IncrementCoherentContactAges();
	MoveNewContactsToCoherentArray();
	RefreshOrRemoveCoherentContacts();
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
CollisionScene<BoundingVolumeClass>::CollisionScene()
{
	if (m_isDoingCoherency)
	{
		m_newContacts = (Contact*)malloc(sizeof(Contact) * 2 * MAX_CONTACT_COUNT);
		m_coherentContacts = m_newContacts + MAX_CONTACT_COUNT;

		for (int i = 0; i < 2 * MAX_CONTACT_COUNT; ++i)
		{
			m_newContacts[i] = Contact();
		}
	}
	else
	{
		m_newContacts = (Contact*)malloc(sizeof(Contact) * MAX_CONTACT_COUNT);

		for (int i = 0; i < MAX_CONTACT_COUNT; ++i)
		{
			m_newContacts[i] = Contact();
		}
	}
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void CollisionScene<BoundingVolumeClass>::DebugDrawContacts() const
{
	if (m_isDoingCoherency)
	{
		for (int i = 0; i < m_numCoherentContacts; ++i)
		{
			const Contact& contact = m_coherentContacts[i];
			DebugDrawSphere(contact.position, 0.1f, Rgba::CYAN, 0.f);
		}
	}
	else
	{
		for (int i = 0; i < m_numNewContacts; ++i)
		{
			const Contact& contact = m_newContacts[i];
			DebugDrawSphere(contact.position, 0.1f, Rgba::CYAN, 0.f);
		}
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
				m_numNewContacts += m_detector.GenerateContacts(*oneAsSphere, *twoAsSphere, &m_newContacts[m_numNewContacts], MAX_CONTACT_COUNT - m_numNewContacts);
			}
			else if (twoIsHalfSpace)
			{
				HalfSpaceCollider* twoAsHalfSpace = colTwo->GetAsType<HalfSpaceCollider>();
				m_numNewContacts += m_detector.GenerateContacts(*oneAsSphere, *twoAsHalfSpace, &m_newContacts[m_numNewContacts], MAX_CONTACT_COUNT - m_numNewContacts);
			}
			else if (twoIsBox)
			{
				BoxCollider* twoAsBox = colTwo->GetAsType<BoxCollider>();
				m_numNewContacts += m_detector.GenerateContacts(*twoAsBox, *oneAsSphere, &m_newContacts[m_numNewContacts], MAX_CONTACT_COUNT - m_numNewContacts);
			}
		}
		else if (oneIsBox)
		{
			BoxCollider* oneAsBox = colOne->GetAsType<BoxCollider>();

			if (twoIsSphere)
			{
				SphereCollider* twoAsSphere = colTwo->GetAsType<SphereCollider>();
				m_numNewContacts += m_detector.GenerateContacts(*oneAsBox, *twoAsSphere, &m_newContacts[m_numNewContacts], MAX_CONTACT_COUNT - m_numNewContacts);
			}
			else if (twoIsHalfSpace)
			{
				HalfSpaceCollider* twoAsHalfSpace = colTwo->GetAsType<HalfSpaceCollider>();
				m_numNewContacts += m_detector.GenerateContacts(*oneAsBox, *twoAsHalfSpace, &m_newContacts[m_numNewContacts], MAX_CONTACT_COUNT - m_numNewContacts);
			}
			else if (twoIsBox)
			{
				BoxCollider* twoAsBox = colTwo->GetAsType<BoxCollider>();
				m_numNewContacts += m_detector.GenerateContacts(*oneAsBox, *twoAsBox, &m_newContacts[m_numNewContacts], MAX_CONTACT_COUNT - m_numNewContacts);
			}
		}
		else if (oneIsHalfSpace)
		{
			HalfSpaceCollider* oneAsHalfSpace = colOne->GetAsType<HalfSpaceCollider>();

			if (twoIsSphere)
			{
				SphereCollider* twoAsSphere = colTwo->GetAsType<SphereCollider>();
				m_numNewContacts += m_detector.GenerateContacts(*twoAsSphere, *oneAsHalfSpace, &m_newContacts[m_numNewContacts], MAX_CONTACT_COUNT - m_numNewContacts);
			}
			else if (twoIsBox)
			{
				BoxCollider* twoAsBox = colTwo->GetAsType<BoxCollider>();
				m_numNewContacts += m_detector.GenerateContacts(*twoAsBox, *oneAsHalfSpace, &m_newContacts[m_numNewContacts], MAX_CONTACT_COUNT - m_numNewContacts);
			}
		}
	}
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void CollisionScene<BoundingVolumeClass>::ResolveContacts(float deltaSeconds)
{
	Contact* contactsToResolve = m_newContacts;
	int numContactsToResolve = m_numNewContacts;

	if (m_isDoingCoherency && (m_numNewContacts + m_numCoherentContacts) > 0)
	{
		ConsolePrintf("New Contacts: %i", m_numNewContacts);
		ConsolePrintf("Contacts From Last Frame: %i", m_numCoherentContacts);
		FinalizeContacts();
		contactsToResolve = m_coherentContacts;
		numContactsToResolve = m_numCoherentContacts;

		ConsolePrintf("Merged Contacts: %i", m_numCoherentContacts);
	}

	if (numContactsToResolve > 0)
	{	
		m_resolver.SetMaxVelocityIterations(Min(m_defaultNumVelocityIterations, 2 * numContactsToResolve));
		m_resolver.SetMaxPenetrationIterations(Min(m_defaultNumPenetrationIterations, 2 * numContactsToResolve));
		m_resolver.ResolveContacts(contactsToResolve, numContactsToResolve, deltaSeconds);
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
