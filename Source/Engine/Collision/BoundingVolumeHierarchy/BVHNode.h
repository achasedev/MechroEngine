///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: May 6th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------

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
public:
	//-----Public Methods-----

	int		GetPotentialCollisions(PotentialCollision* out_collisions, int limit) const;
	bool	IsLeaf() const { return m_entity != nullptr; }


private:
	//-----Private Methods-----

	int		GetPotentialCollisionsBetween(const BVHNode<BoundingVolumeClass>* other, PotentialCollision* out_collisions, int limit) const;


private:
	//-----Private Data-----

	BVHNode*				m_children[2];
	BoundingVolumeClass		m_boundingVolume; // Encompasses all entities at or below this level
	Entity*					m_entity = nullptr; // Only set on leaf nodes

};


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

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
