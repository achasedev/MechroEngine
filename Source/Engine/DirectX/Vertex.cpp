///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: February 15th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/DirectX/DX11Common.h"
#include "Engine/DirectX/Vertex.h"

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
const VertexAttribute Vertex3D_PCU::ATTRIBUTES[] =
{
	VertexAttribute("POSITION", RDT_FLOAT, 3, false, offsetof(Vertex3D_PCU, m_position)),
	VertexAttribute("COLOR", RDT_FLOAT, 4, false, offsetof(Vertex3D_PCU, m_color)),
	VertexAttribute("UV", RDT_FLOAT, 2, false, offsetof(Vertex3D_PCU, m_texUVs)),
};

const uint32 Vertex3D_PCU::NUM_ATTRIBUTES = (sizeof(ATTRIBUTES) / sizeof(VertexAttribute));
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

const uint32 VertexLit::NUM_ATTRIBUTES = (sizeof(ATTRIBUTES) / sizeof(VertexAttribute));
const VertexLayout VertexLit::LAYOUT = VertexLayout(sizeof(VertexLit), NUM_ATTRIBUTES, VertexLit::ATTRIBUTES);

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
uint32 GetDXFormatForAttribute(const VertexAttribute& attribute)
{
	switch (attribute.m_dataType)
	{
	case RDT_FLOAT:
	{
		switch (attribute.m_elementCount)
		{
		case 1: return DXGI_FORMAT_R32_FLOAT; break;
		case 2: return DXGI_FORMAT_R32G32_FLOAT; break;
		case 3: return DXGI_FORMAT_R32G32B32_FLOAT; break;
		case 4: return DXGI_FORMAT_R32G32B32A32_FLOAT; break;
		default:
			ERROR_AND_DIE("VertexAttribute has more than 4 elements, that's not supported!");
			break;
		}
	}
		break;
	case RDT_UNSIGNED_INT:
	{
		switch (attribute.m_elementCount)
		{
		case 1: return DXGI_FORMAT_R32_UINT; break;
		case 2: return DXGI_FORMAT_R32G32_UINT; break;
		case 3: return DXGI_FORMAT_R32G32B32_UINT; break;
		case 4: return DXGI_FORMAT_R32G32B32A32_UINT; break;
		default:
			ERROR_AND_DIE("VertexAttribute has more than 4 elements, that's not supported!");
			break;
		}
	}
		break;
	case RDT_SIGNED_INT:
	{
		switch (attribute.m_elementCount)
		{
		case 1: return DXGI_FORMAT_R32_SINT; break;
		case 2: return DXGI_FORMAT_R32G32_SINT; break;
		case 3: return DXGI_FORMAT_R32G32B32_SINT; break;
		case 4: return DXGI_FORMAT_R32G32B32A32_SINT; break;
		default:
			ERROR_AND_DIE("VertexAttribute has more than 4 elements, that's not supported!");
			break;
		}
	}
		break;
	case RDT_UNSIGNED_BYTE:
	{
		switch (attribute.m_elementCount)
		{
		case 1: return DXGI_FORMAT_R8_UINT; break;
		case 2: return DXGI_FORMAT_R8G8_UINT; break;
		case 3: ERROR_AND_DIE("VertexAttribute of 3 bytes not supported?");
		case 4: return DXGI_FORMAT_R8G8B8A8_UINT; break;
		default:
			ERROR_AND_DIE("VertexAttribute has more than 4 elements, that's not supported!");
			break;
		}
	}
		break;
	default:
		ERROR_AND_DIE("VertexAttribute has an invalid type!");
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
