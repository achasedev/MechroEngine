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

	MeshBuilder();
	~MeshBuilder();

	void Reset();
	
	// VertexMaster Stamp
	void SetColor(const Color& color);
	void SetUV(const Vector2& uv);
	void SetDrawInstruction(const DrawInstruction& instruction);
	void SetDrawInstruction(bool useIndices, unsigned int startIndex, unsigned int elementCount);

	unsigned int PushVertex(const Vector3& position);
	unsigned int PushVertex(const VertexMaster& master);

	// Helpers
	//unsigned int PushTriangle();

	unsigned int GetVertexCount() const { return m_vertices.size(); }
	unsigned int GetIndexCount() const { return m_indices.size(); }

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
		unsigned int vertexCount = (unsigned int)m_vertices.size();
		VERT_TYPE* temp = (VERT_TYPE*)malloc(sizeof(VERT_TYPE) * vertexCount);

		for (unsigned int vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
		{
			temp[vertexIndex] = VERT_TYPE(m_vertices[vertexIndex]);
		}

		// Set up the mesh
		out_mesh.SetVertices(vertexCount, temp);
		out_mesh.SetIndices((unsigned int)m_indices.size(), m_indices.data());

		free(temp);
	}

private:
	//-----Private Data-----

	VertexMaster m_stamp;
	DrawInstruction m_instruction;

	std::vector<VertexMaster>	m_vertices;
	std::vector<unsigned int>	m_indices;

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                           *** C FUNCTIONS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------
