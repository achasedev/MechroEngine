///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: February 15th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** INCLUDES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/DirectX/Mesh.h"
#include "Engine/DirectX/Vertex.h"
#include <vector>

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** DEFINES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                              *** TYPES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Color;
class Mesh;
class Vector2;
class Vector3;
struct VertexMaster;

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** STRUCTS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                        *** GLOBALS AND STATICS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** CLASSES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class MeshBuilder
{
public:
	//-----Public Methods-----

	MeshBuilder() {};
	~MeshBuilder();

	void BeginBuilding(bool useIndices);
	void FinishBuilding();
	void Clear();
	
	// VertexMaster Stamp
	void SetColor(const Color& color);
	void SetUV(const Vector2& uv);
	void SetDrawInstruction(const DrawInstruction& instruction);
	void SetDrawInstruction(bool useIndices, uint startIndex, uint elementCount);

	uint PushVertex(const Vector3& position);
	uint PushVertex(const VertexMaster& master);

	// Helpers HERE

	uint GetVertexCount() const { return (uint)m_vertices.size(); }
	uint GetIndexCount() const { return (uint)m_indices.size(); }

	template <typename VERT_TYPE>
	Mesh* CreateMesh() const
	{
		Mesh* mesh = new Mesh();
		UpdateMesh<VERT_TYPE>(*mesh);

		return mesh;
	}

	template <typename VERT_TYPE>
	Mesh* UpdateMesh(Mesh& out_mesh) const
	{
		// Convert the list of VertexMasters to the specified vertex type
		uint vertexCount = (uint)m_vertices.size();
		uint indexCount = (uint)m_indices.size();

		ASSERT_OR_DIE(vertexCount > 0, "You're creating a mesh with no vertices! Don't do that.");

		if (m_instruction.m_useIndices)
		{
			ASSERT_OR_DIE(indexCount > 0, "You're creating an indexed mesh with no indices! Don't do that.");
		}

		VERT_TYPE* temp = (VERT_TYPE*)malloc(sizeof(VERT_TYPE) * vertexCount);

		for (uint vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
		{
			temp[vertexIndex] = VERT_TYPE(m_vertices[vertexIndex]);
		}

		// Set up the mesh
		out_mesh.SetVertices(vertexCount, temp);
		out_mesh.SetIndices(indexCount, m_indices.data());

		free(temp);
	}

private:
	//-----Private Data-----

	bool m_isBuilding = false;
	VertexMaster m_stamp;
	DrawInstruction m_instruction;

	std::vector<VertexMaster>	m_vertices;
	std::vector<uint>	m_indices;

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                           *** C FUNCTIONS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------
