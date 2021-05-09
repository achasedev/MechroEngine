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

	void DoCollisionStep();
	void DebugRenderBoundingHierarchy() const;
	void DebugRenderLeafBoundingVolumes() const;


private:
	//-----Private Methods-----

	void UpdateBVH();
	void UpdateNode(BVHNode<BoundingVolumeClass>* node, const BoundingVolumeClass& newVolume);

	BVHNode<BoundingVolumeClass>* GetAndEraseLeafNodeForEntity(Entity* entity);


private:
	//-----Private Static Data

	static constexpr int MAX_POTENTIAL_COLLISION_COUNT = 50;

private:
	//-----Private Data-----

	BVHNode<BoundingVolumeClass>*				m_boundingTreeRoot = nullptr;
	std::vector<BVHNode<BoundingVolumeClass>*>	m_leaves;  // Optimization, faster search

	PotentialCollision							m_potentialCollisions[MAX_POTENTIAL_COLLISION_COUNT];
	int											m_numPotentialCollisions;
	CollisionDetector							m_detector;

};


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void CollisionScene<BoundingVolumeClass>::UpdateBVH()
{
	// For each node, check if the entity's bounding volume changed a significant amount. If so, update it
	for (BVHNode<BoundingVolumeClass>* node : m_leaves)
	{
		BoundingVolumeSphere worldVolume = node->m_entity->GetWorldPhysicsBoundingVolume();

		if (!AreMostlyEqual(node->m_boundingVolume, worldVolume))
		{
			UpdateNode(node, worldVolume);
		}
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
		leaf->m_boundingVolume.DebugRender();
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
	BoundingVolumeSphere worldVolume = entity->GetWorldPhysicsBoundingVolume();
	BVHNode<BoundingVolumeClass>* node = new BVHNode<BoundingVolumeClass>(worldVolume);
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
void CollisionScene<BoundingVolumeClass>::DoCollisionStep()
{
	UpdateBVH();

	m_numPotentialCollisions = m_boundingTreeRoot->GetPotentialCollisions(m_potentialCollisions, MAX_POTENTIAL_COLLISION_COUNT);


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
	node->m_boundingVolume = newVolume;

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
