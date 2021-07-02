///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: July 2nd, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Render/Vertex.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
const VertexAttribute Vertex3D_PC::ATTRIBUTES[] =
{
	VertexAttribute("POSITION",		DATA_TYPE_FLOAT_THREE,	3, offsetof(Vertex3D_PC, m_position)),
	VertexAttribute("COLOR",		DATA_TYPE_RGBA,			4, offsetof(Vertex3D_PC, m_color)),
};

const uint32 Vertex3D_PC::NUM_ATTRIBUTES = (sizeof(ATTRIBUTES) / sizeof(VertexAttribute));
const VertexLayout Vertex3D_PC::LAYOUT = VertexLayout(sizeof(Vertex3D_PC), NUM_ATTRIBUTES, Vertex3D_PC::ATTRIBUTES);


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
VkFormat GetVkFormatFromVertexDataType(VertexDataType dataType)
{
	switch (dataType)
	{
	case DATA_TYPE_FLOAT_THREE:
		return VK_FORMAT_R32G32B32_SFLOAT;
		break;
	case DATA_TYPE_RGBA:
		return VK_FORMAT_R8G8B8A8_UNORM;
		break;
	default:
		ERROR_AND_DIE("Invalid data type!");
		break;
	}
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
VertexLayout::VertexLayout(uint32 stride, uint32 numAttributes, const VertexAttribute* attributes)
	: m_vertexStride(stride)
	, m_numAttributes(numAttributes)
	, m_attributes(attributes)
{
}


//-------------------------------------------------------------------------------------------------
uint32 VertexLayout::GetAttributeCount() const
{
	return m_numAttributes;
}


//-------------------------------------------------------------------------------------------------
const VertexAttribute& VertexLayout::GetAttribute(uint32 index) const
{
	ASSERT_OR_DIE(index >= 0 && index < m_numAttributes, "Error: VertexLayout::GetAttribute index out of range, index was %i", index);
	return m_attributes[index];
}


//-------------------------------------------------------------------------------------------------
uint32 VertexLayout::GetStride() const
{
	return m_vertexStride;
}


//-------------------------------------------------------------------------------------------------
VkVertexInputBindingDescription VertexLayout::GetVkBindingDescription() const
{
	VkVertexInputBindingDescription description{};

	description.binding = 0;
	description.stride = m_vertexStride;
	description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return description;
}


//-------------------------------------------------------------------------------------------------
void VertexLayout::GetVkAttributeDescriptions(std::vector<VkVertexInputAttributeDescription>& out_descriptions) const
{
	out_descriptions.clear();
	out_descriptions.resize(m_numAttributes);

	for (uint32 i = 0; i < m_numAttributes; ++i)
	{
		out_descriptions[i] = m_attributes->GetVkAttributeDescription(i);
	}
}


//-------------------------------------------------------------------------------------------------
VkVertexInputAttributeDescription VertexAttribute::GetVkAttributeDescription(uint32_t location) const
{
	VkVertexInputAttributeDescription description{};

	description.binding = 0;
	description.location = location;
	description.format = GetVkFormatFromVertexDataType(m_dataType);
	description.offset = (uint32_t) m_memberOffset;

	return description;
}
