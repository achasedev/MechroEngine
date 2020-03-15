///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: February 15th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/DirectX/Mesh.h"
#include "Engine/DirectX/Vertex.h"
#include <vector>

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Rgba;
class Mesh;
class Vector2;
class Vector3;
struct VertexMaster;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
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
	void SetColor(const Rgba& color);
	void SetUV(const Vector2& uv);
	void SetDrawInstruction(const DrawInstruction& instruction);
	void SetDrawInstruction(bool useIndices, uint32 startIndex, uint32 elementCount);

	uint32 PushVertex(const Vector3& position);
	uint32 PushVertex(const VertexMaster& master);
	void PushIndex(uint32 index);

	// Helpers HERE

	uint32 GetVertexCount() const { return (uint32)m_vertices.size(); }
	uint32 GetIndexCount() const { return (uint32)m_indices.size(); }

	//-------------------------------------------------------------------------------------------------
	template <typename VERT_TYPE>
	Mesh* CreateMesh() const
	{
		Mesh* mesh = new Mesh();
		UpdateMesh<VERT_TYPE>(*mesh);

		return mesh;
	}


	//-------------------------------------------------------------------------------------------------
	template <typename VERT_TYPE>
	void UpdateMesh(Mesh& out_mesh) const
	{
		// Convert the list of VertexMasters to the specified vertex type
		uint32 vertexCount = (uint32)m_vertices.size();
		uint32 indexCount = (uint32)m_indices.size();

		ASSERT_OR_DIE(vertexCount > 0, "You're creating a mesh with no vertices! Don't do that.");

		if (m_instruction.m_useIndices)
		{
			ASSERT_OR_DIE(indexCount > 0, "You're creating an indexed mesh with no indices! Don't do that.");
		}

		VERT_TYPE* temp = (VERT_TYPE*)malloc(sizeof(VERT_TYPE) * vertexCount);

		for (uint32 vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
		{
			temp[vertexIndex] = VERT_TYPE(m_vertices[vertexIndex]);
		}

		// Set up the mesh
		out_mesh.SetVertices(temp, vertexCount);

		if (m_instruction.m_useIndices)
		{
			out_mesh.SetIndices(m_indices.data(), indexCount);
		}

		out_mesh.SetDrawInstruction(m_instruction);

		SAFE_FREE_POINTER(temp);
	}


private:
	//-----Private Data-----

	bool						m_isBuilding = false;
	VertexMaster				m_stamp;
	DrawInstruction				m_instruction;
	std::vector<VertexMaster>	m_vertices;
	std::vector<uint32>			m_indices;

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
