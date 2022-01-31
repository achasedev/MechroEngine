///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: February 15th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Render/Buffer/IndexBuffer.h"
#include "Engine/Render/Buffer/VertexBuffer.h"
#include "Engine/Resource/Resource.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
enum MeshTopology
{
	TOPOLOGY_TRIANGLE_LIST,
	TOPOLOGY_LINE_LIST
};

struct DrawInstruction
{
	DrawInstruction() {}
	explicit DrawInstruction(MeshTopology topology, bool useIndices, unsigned int startIndex, unsigned int elementCount)
		: m_topology(topology), m_startIndex(startIndex), m_elementCount(elementCount), m_useIndices(useIndices) {}

	DrawInstruction(const DrawInstruction& other)
		: m_topology(other.m_topology), m_startIndex(other.m_startIndex), m_elementCount(other.m_elementCount), m_useIndices(other.m_useIndices) {}

	unsigned int	m_startIndex = 0;
	unsigned int	m_elementCount = 0;
	bool			m_useIndices = true;
	MeshTopology	m_topology = TOPOLOGY_TRIANGLE_LIST;

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class Mesh : public Resource
{
public:
	//-----Public Methods-----

	~Mesh();

	void Load(const char* filepath);
	void Clear();

	template <typename VERT_TYPE>
	void SetVertices(const VERT_TYPE* vertices, uint32 vertexCount)
	{
		bool succeeded = m_vertexBuffer.CopyToGPU<VERT_TYPE>(vertices, vertexCount);

		if (succeeded)
		{
			m_vertexLayout = &VERT_TYPE::LAYOUT;
		}
	}

	void				SetIndices(const uint32* indices, uint32 indexCount);
	void				SetDrawInstruction(DrawInstruction instruction);
	void				SetDrawInstruction(bool useIndices, uint32 startIndex, uint32 elementCount);

	const VertexBuffer*	GetVertexBuffer() const;
	const IndexBuffer*	GetIndexBuffer() const;
	DrawInstruction		GetDrawInstruction() const;
	const VertexLayout*	GetVertexLayout() const;


private:
	//-----Private Data-----

	VertexBuffer		m_vertexBuffer;
	IndexBuffer			m_indexBuffer;

	const VertexLayout* m_vertexLayout = nullptr;
	DrawInstruction		m_instruction;

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
