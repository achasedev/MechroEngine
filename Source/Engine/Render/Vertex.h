///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: July 2nd, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Render/VulkanCommon.h"
#include "Engine/Core/Rgba.h"
#include "Engine/Core/EngineCommon.h"
#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector4.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class VertexLayout;

enum VertexDataType
{
	DATA_TYPE_FLOAT_THREE,
	DATA_TYPE_RGBA,
	NUM_RDTS
};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Description for a single attribute of a vertex, a layout is made up of a collection of these
class VertexAttribute
{
public:
	//-----Public Methods-----

	VertexAttribute() : m_name("") {} // For null terminator in ATTRIBUTES array (unused)
	VertexAttribute(const std::string name, VertexDataType type, unsigned int elementCount, unsigned int memberOffset)
		: m_name(name), m_dataType(type), m_elementCount(elementCount), m_memberOffset(memberOffset)
	{}

	VkVertexInputAttributeDescription GetVkAttributeDescription(uint32_t location) const;


private:
	//-----Private Data-----

	std::string		m_name;
	VertexDataType	m_dataType;
	unsigned int	m_elementCount;
	size_t			m_memberOffset;

};


//-------------------------------------------------------------------------------------------------
// Set of attributes for a vertex type
class VertexLayout
{
public:
	//-----Public Methods-----

	VertexLayout(uint32 stride, uint32 numAttributes, const VertexAttribute* attributes);

	uint32					GetAttributeCount() const;
	const VertexAttribute&	GetAttribute(uint32 index) const;
	uint32					GetStride() const;

	VkVertexInputBindingDescription GetVkBindingDescription() const;
	void GetVkAttributeDescriptions(std::vector<VkVertexInputAttributeDescription>& out_descriptions) const;


private:
	//-----Private Data-----

	const VertexAttribute*			m_attributes;
	uint32							m_numAttributes;
	uint32							m_vertexStride;
};


//-------------------------------------------------------------------------------------------------
// Vertex with position and color
//
struct Vertex3D_PC
{
	// Constructors
	Vertex3D_PC() {};
	Vertex3D_PC(const Vector3& position, const Rgba& color)
		: m_position(position), m_color(color) {}

	Vector3 m_position;
	Rgba	m_color;

	static const VertexAttribute	ATTRIBUTES[];
	static const VertexLayout		LAYOUT;
	static const unsigned int		NUM_ATTRIBUTES;
};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

