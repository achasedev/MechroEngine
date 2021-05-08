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
	//-----Friend Classes-----
	friend class CollisionScene<BoundingVolumeClass>;

public:
	//-----Public Methods-----

	int		GetPotentialCollisions(PotentialCollision* out_collisions, int limit) const; // Intended to be called on the root node to get total collisions
	bool	IsLeaf() const { return m_entity != nullptr; }


private:
	//-----Private Methods-----

	BVHNode(const BoundingVolumeClass& boundingVolume);
	~BVHNode();

	// Recursively updates the tree to add/remove the given node
	void	Insert(BVHNode<BoundingVolumeClass>* node);
	void	Remove(BVHNode<BoundingVolumeClass>* node);
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
BVHNode<BoundingVolumeClass>::~BVHNode()
{
	ASSERT_OR_DIE(m_children[0] == nullptr && m_children[1] == nullptr, "BVHNode being deleted but has children!");
	ASSERT_OR_DIE(m_parent == nullptr, "BVHNode being deleted but has parent!");
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
BVHNode<BoundingVolumeClass>::BVHNode(const BoundingVolumeClass& boundingVolume)
{
	m_boundingVolume = boundingVolume;
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
void BVHNode<BoundingVolumeClass>::Insert(BVHNode<BoundingVolumeClass>* node)
{
	ASSERT_OR_DIE(node->m_entity != nullptr, "Only insert nodes that could be leaves - actual entities!");

	if (IsLeaf())
	{
		// Convert this node to a parent of what this node was + the new node

		// First make child 0 us
		m_children[0] = new BVHNode<BoundingVolumeClass>(m_boundingVolume);
		m_children[0]->m_entity = m_entity;
		m_children[0]->m_parent = this;

		// Now set up child 1
		m_children[1] = node;
		m_children[1]->m_parent = this;

		// Now clean us up
		m_entity = nullptr;
		RecalculateBoundingVolume();
	}
	else
	{
		// Recurse down the path that would grow less to encompass this node
		if (m_children[0]->m_boundingVolume.GetGrowth(node->m_boundingVolume) < m_children[1]->m_boundingVolume.GetGrowth(node->m_boundingVolume))
		{
			m_children[0]->Insert(node);
		}
		else
		{
			m_children[1]->Insert(node);
		}
	}
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void BVHNode<BoundingVolumeClass>::Remove(BVHNode<BoundingVolumeClass>* node)
{
	ASSERT_OR_DIE(m_entity != nullptr, "Only remove nodes that could be leaves - actual entities!");
	ASSERT_OR_DIE(m_children[0] == nullptr && m_children[1] == nullptr, "Node being removed has children!");

	// We need to put our sibling in our parent's place
	BVHNode<BoundingVolumeClass>* sibling = (m_parent->m_children[0] == this ? m_children[1] : m_children[0]);

	BVHNode<BoundingVolumeClass>* toDelete = sibling->m_parent;
	sibling->m_parent = toDelete->m_parent;

	// Not necessary, but to ensure we're cleaning up all lose ends and pass our asserts in the destructor
	toDelete->m_parent = nullptr;
	toDelete->m_children[0] = nullptr;
	toDelete->m_children[1] = nullptr;
	SAFE_DELETE(toDelete);

	// Update the nodes above this
	sibling->m_parent->RecalculateBoundingVolume();

	// Clean up the removed node
	node->m_parent = nullptr;
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void BVHNode<BoundingVolumeClass>::RecalculateBoundingVolume()
{
	m_boundingVolume = BoundingVolumeClass(m_children[0]->m_boundingVolume, m_children[1]->m_boundingVolume);
	if (m_parent != nullptr)
	{
		m_parent->RecalculateBoundingVolume();
	}
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
