///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: May 6th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Utility/Assert.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Entity;
struct PotentialCollision
{
	Entity* entities[2];
};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
class BVHNode
{
	template<class BoundingVolumeClass> friend class CollisionScene;

public:
	//-----Public Methods-----

	int		GetPotentialCollisions(PotentialCollision* out_collisions, int limit) const; // Intended to be called on the root node to get total collisions
	bool	IsLeaf() const;
	bool	IsRoot() const { return m_parent == nullptr; }


private:
	//-----Private Methods-----

	BVHNode();
	BVHNode(const BoundingVolumeClass& boundingVolume);
	~BVHNode();

	void DebugRender() const;

	// Recursively updates the tree to add/remove the given node
	BVHNode<BoundingVolumeClass>*	Insert(BVHNode<BoundingVolumeClass>* node); // Returns the root of the tree, in case insert moves it around
	BVHNode<BoundingVolumeClass>*	RemoveSelf();
	void	RecalculateBoundingVolume();
	int		GetPotentialCollisionsBetween(const BVHNode<BoundingVolumeClass>* other, PotentialCollision* out_collisions, int limit) const;


private:
	//-----Private Data-----

	BVHNode*				m_parent = nullptr;
	BVHNode*				m_children[2];
	BoundingVolumeClass		m_boundingVolume; // Encompasses all entities at or below this level
	Entity*					m_entity = nullptr; // Only set on leaf nodes

};


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
bool BVHNode<BoundingVolumeClass>::IsLeaf() const
{
	bool bothNull = (m_children[0] == nullptr && m_children[1] == nullptr);
	bool bothNotNull = (m_children[0] != nullptr && m_children[1] != nullptr);
	ASSERT_OR_DIE(bothNull || bothNotNull, "Uneven children on node!");

	bool isLeaf = bothNull; // Leafness defined as not having children

	// Safety checks
	if (isLeaf)
	{
		ASSERT_OR_DIE(m_entity != nullptr, "Leaf doesn't have an entity!");
	}

	return isLeaf;
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void BVHNode<BoundingVolumeClass>::DebugRender() const
{
	m_boundingVolume.DebugRender();
	if (!IsLeaf())
	{
		m_children[0]->DebugRender();
		m_children[1]->DebugRender();
	}
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
BVHNode<BoundingVolumeClass>::BVHNode()
{
	m_children[0] = nullptr;
	m_children[1] = nullptr;
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
BVHNode<BoundingVolumeClass>::~BVHNode()
{
	ASSERT_OR_DIE(m_children[0] == nullptr && m_children[1] == nullptr, "BVHNode being deleted but has children!");
	ASSERT_OR_DIE(m_parent == nullptr, "BVHNode being deleted but has parent!");
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
BVHNode<BoundingVolumeClass>::BVHNode(const BoundingVolumeClass& boundingVolume)
	: m_boundingVolume(boundingVolume)
{
	m_children[0] = nullptr;
	m_children[1] = nullptr;
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
int BVHNode<BoundingVolumeClass>::GetPotentialCollisions(PotentialCollision* out_collisions, int limit) const
{
	// If we hit the end or ran out of room, stop recursing
	if (limit == 0 || IsLeaf())
		return 0;

	// Check for collisions between our children
	// The way the hierarchy is built, all non-leafs will always have 2 non-null children
	return m_children[0]->GetPotentialCollisionsBetween(m_children[1], out_collisions, limit);
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
int BVHNode<BoundingVolumeClass>::GetPotentialCollisionsBetween(const BVHNode<BoundingVolumeClass>* other, PotentialCollision* out_collisions, int limit) const
{
	if (limit == 0 || !m_boundingVolume.Overlaps())
		return 0;

	// These two nodes overlap - if they're leaves, then the two entities could overlap
	if (IsLeaf() && other->IsLeaf())
	{
		out_collisions->entities[0] = m_entity;
		out_collisions->entities[1] = other->m_entity;
		return 1;
	}

	// At least one of these aren't a leaf - recursively descend
	// If both aren't leaves, just choose the bounding volume with larger size
	if (other->IsLeaf() || !IsLeaf() && m_boundingVolume.GetSize() >= other->m_boundingVolume.GetSize())
	{
		// Recurse on ourself
		int numAdded = m_children[0]->GetPotentialCollisionsBetween(other, out_collisions, limit);

		if (limit > numAdded)
		{
			return numAdded + m_children[1]->GetPotentialCollisionsBetween(other, out_collisions + numAdded, limit - numAdded);
		}
		else
		{
			// The first child took up all the remaining room, so just stop here
			return numAdded;
		}
	}
	else
	{
		// Recurse on other
		int numAdded = GetPotentialCollisionsBetween(other->m_children[0], out_collisions, limit);

		if (limit > numAdded)
		{
			return numAdded + GetPotentialCollisionsBetween(other->m_children[1], out_collisions + numAdded, limit - numAdded);
		}
		else
		{
			// The first child took up all the remaining room, so just stop here
			return numAdded;
		}
	}
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
BVHNode<BoundingVolumeClass>* BVHNode<BoundingVolumeClass>::Insert(BVHNode<BoundingVolumeClass>* node)
{
	ASSERT_OR_DIE(node->m_entity != nullptr, "Only insert nodes that could be leaves - actual entities!");

	if (IsLeaf())
	{
		// Create a new node to be our parent
		BVHNode<BoundingVolumeClass>* parentNode = new BVHNode<BoundingVolumeClass>();

		// Parent <-> Grandparent
		parentNode->m_parent = m_parent;
		if (parentNode->m_parent != nullptr)
		{
			if (parentNode->m_parent->m_children[0] == this)
			{
				parentNode->m_parent->m_children[0] = parentNode;
			}
			else
			{
				parentNode->m_parent->m_children[1] = parentNode;
			}
		}

		// Parent <-> Child (this)
		parentNode->m_children[0] = this;
		parentNode->m_children[1] = node;

		node->m_parent = parentNode;
		m_parent = parentNode;

		// Update the parent now to have the right bounding volume
		m_parent->RecalculateBoundingVolume();

		// If the root changed, return it, otherwise return nullptr
		if (m_parent->IsRoot())
		{
			return m_parent;
		}
		else
		{
			return nullptr;
		}
	}
	else
	{
		// Recurse down the path that would grow less to encompass this node
		if (m_children[0]->m_boundingVolume.GetGrowth(node->m_boundingVolume) < m_children[1]->m_boundingVolume.GetGrowth(node->m_boundingVolume))
		{
			return m_children[0]->Insert(node);
		}
		else
		{
			return m_children[1]->Insert(node);
		}
	}
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
BVHNode<BoundingVolumeClass>* BVHNode<BoundingVolumeClass>::RemoveSelf()
{
	ASSERT_OR_DIE(m_entity != nullptr, "Only remove nodes that could be leaves - actual entities!");
	ASSERT_OR_DIE(m_children[0] == nullptr && m_children[1] == nullptr, "Node being removed has children!");
	ASSERT_OR_DIE(!IsRoot(), "Can't remove root - the owner of the tree needs to delete us in that case!");

	// We need to put our sibling in our parent's place, and then delete the parent
	BVHNode<BoundingVolumeClass>* sibling = (m_parent->m_children[0] == this ? m_parent->m_children[1] : m_parent->m_children[0]);
	BVHNode<BoundingVolumeClass>* toDelete = sibling->m_parent;

	// Connecting sibling to new parent (previously was grandparent)
	sibling->m_parent = toDelete->m_parent;
	if (sibling->m_parent != nullptr)
	{
		if (sibling->m_parent->m_children[0] == toDelete)
		{
			sibling->m_parent->m_children[0] = sibling;
		}
		else
		{
			sibling->m_parent->m_children[1] = sibling;
		}
	}

	// Not necessary, but to ensure we're cleaning up all lose ends and pass our asserts in the destructor
	toDelete->m_parent = nullptr;
	toDelete->m_children[0] = nullptr;
	toDelete->m_children[1] = nullptr;
	delete toDelete;
	toDelete = nullptr;

	// Clean up the removed node
	m_parent = nullptr;

	// If the sibling is the new root, return it
	if (sibling->IsRoot())
	{
		return sibling;
	}

	// Otherwise tell the new parent to account for not having us anymore
	sibling->m_parent->RecalculateBoundingVolume();
	return nullptr;
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void BVHNode<BoundingVolumeClass>::RecalculateBoundingVolume()
{
	ASSERT_OR_DIE(m_children[0] != nullptr && m_children[1] != nullptr, "No children to use for recalculation!");
	ASSERT_OR_DIE(!IsLeaf(), "Leaf nodes should not be recalculated!");
	ASSERT_OR_DIE(this != m_parent, "We are our own parent!");

	m_boundingVolume = BoundingVolumeClass(m_children[0]->m_boundingVolume, m_children[1]->m_boundingVolume);
	if (m_parent != nullptr)
	{
		m_parent->RecalculateBoundingVolume();
	}
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
