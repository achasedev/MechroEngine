///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: February 15th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** INCLUDES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/DirectX/Vertex.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** DEFINES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                              *** TYPES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** STRUCTS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                        *** GLOBALS AND STATICS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
const VertexAttribute Vertex3D_PCU::ATTRIBUTES[] =
{
	VertexAttribute("POSITION", RDT_FLOAT, 3, false, offsetof(Vertex3D_PCU, m_position)),
	VertexAttribute("COLOR", RDT_FLOAT, 4, false, offsetof(Vertex3D_PCU, m_color)),
	VertexAttribute("UV", RDT_FLOAT, 2, false, offsetof(Vertex3D_PCU, m_texUVs)),
};

const uint Vertex3D_PCU::NUM_ATTRIBUTES = (sizeof(ATTRIBUTES) / sizeof(VertexAttribute));
const VertexLayout Vertex3D_PCU::LAYOUT = VertexLayout(sizeof(Vertex3D_PCU), NUM_ATTRIBUTES, Vertex3D_PCU::ATTRIBUTES);


//-------------------------------------------------------------------------------------------------
const VertexAttribute VertexLit::ATTRIBUTES[] =
{
	VertexAttribute("POSITION",		RDT_FLOAT,			3,		false,		offsetof(VertexLit, m_position)),
	VertexAttribute("COLOR",		RDT_FLOAT,			4,		false,		offsetof(VertexLit, m_color)),
	VertexAttribute("UV",			RDT_FLOAT,			2,		false,		offsetof(VertexLit, m_texUVs)),
	VertexAttribute("NORMAL",		RDT_FLOAT,			3,		false,		offsetof(VertexLit, m_normal)),
	VertexAttribute("TANGENT",		RDT_FLOAT,			3,		false,		offsetof(VertexLit, m_tangent))
};

const uint VertexLit::NUM_ATTRIBUTES = (sizeof(ATTRIBUTES) / sizeof(VertexAttribute));
const VertexLayout VertexLit::LAYOUT = VertexLayout(sizeof(VertexLit), NUM_ATTRIBUTES, VertexLit::ATTRIBUTES);

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                           *** C FUNCTIONS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** CLASSES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
VertexLayout::VertexLayout(uint stride, uint numAttributes, const VertexAttribute* attributes)
	: m_vertexStride(stride)
	, m_numAttributes(numAttributes)
	, m_attributes(attributes)
{
}


//-------------------------------------------------------------------------------------------------
uint VertexLayout::GetAttributeCount() const
{
	return m_numAttributes;
}


//-------------------------------------------------------------------------------------------------
const VertexAttribute& VertexLayout::GetAttribute(uint index) const
{
	ASSERT_OR_DIE(index >= 0 && index < m_numAttributes, "Error: VertexLayout::GetAttribute index out of range, index was %i", index);
	return m_attributes[index];
}


//-------------------------------------------------------------------------------------------------
uint VertexLayout::GetStride() const
{
	return m_vertexStride;
}
