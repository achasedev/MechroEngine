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

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Entity;

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

	CollisionScene() {}
	~CollisionScene();

	void Update();

	void AddEntity(Entity* entity);
	void RemoveEntity(Entity* entity);


private:
	//-----Private Methods-----

	void UpdateNode(BVHNode* node, const BoundingVolumeClass& newVolume);

	BVHNode<BoundingVolumeClass>* GetAndEraseLeafNodeForEntity(Entity* entity);


private:
	//-----Private Data-----

	BVHNode<BoundingVolumeClass>*				m_boundingTreeRoot = nullptr;
	std::vector<BVHNode<BoundingVolumeClass>*>	m_leaves;  // Optimization, faster search

};


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

	m_boundingTreeRoot->Insert(node);
	m_leaves.push_back(node);
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void CollisionScene<BoundingVolumeClass>::RemoveEntity(Entity* entity)
{
	BVHNode<BoundingVolumeClass>* node = GetAndEraseLeafNodeForEntity(entity);
	m_boundingTreeRoot->Remove(node);
	SAFE_DELETE(node);
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void CollisionScene<BoundingVolumeClass>::Update()
{
	// For each node, check if the entity's bounding volume changed a significant amount. If so, update it
	for (BVHNode* node : m_leaves)
	{
		BoundingVolumeClass worldVolume = node->m_entity->GetWorldPhysicsBoundingVolume<BoundingVolumeClass>();

		if (!AreMostlyEqual(node->m_boundingVolume, worldVolume))
		{
			UpdateNode(node, worldVolume);
		}
	}
}


//-------------------------------------------------------------------------------------------------
template <class BoundingVolumeClass>
void CollisionScene<BoundingVolumeClass>::UpdateNode(BVHNode<BoundingVolumeClass>* node, const BoundingVolumeClass& newVolume)
{
	// Remove the node from the tree, recursively updating bounding volumes
	m_boundingTreeRoot->Remove(node);

	// Set the node's new bounding volume
	node->m_boundingVolume = newVolume;

	// Re-insert the node into the tree, based on "best fit"
	m_boundingTreeRoot->Insert(node);
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
