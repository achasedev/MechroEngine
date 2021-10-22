///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: December 14th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
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
struct VertexMaster;
class VertexLayout;

enum RenderDataType
{
	RDT_FLOAT,
	RDT_UNSIGNED_BYTE,
	RDT_UNSIGNED_BYTE_NORM_TO_FLOAT,
	RDT_UNSIGNED_INT,
	RDT_SIGNED_INT,
	NUM_RDTS
};

//-------------------------------------------------------------------------------------------------
// Description for a single attribute of a vertex, a layout is made up of a collection of these
struct VertexAttribute
{
	VertexAttribute() : m_name("") {} // For null terminator in ATTRIBUTES array (unused)
	VertexAttribute(const std::string name, RenderDataType type, unsigned int elementCount, unsigned int memberOffset)
		: m_name(name), m_dataType(type), m_elementCount(elementCount), m_memberOffset(memberOffset)
	{}

	std::string		m_name;
	RenderDataType	m_dataType;
	unsigned int	m_elementCount;
	size_t			m_memberOffset;
};

//-------------------------------------------------------------------------------------------------
// VERTEX TYPES

// Used to construct all vertex types in MeshBuilder
struct VertexMaster
{
	Vector3 m_position = Vector3::ZERO;
	Vector2 m_uvs = Vector2::ZERO;
	Rgba	m_color = Rgba::WHITE;
	Vector3 m_normal = Vector3::ZERO;
	Vector4 m_tangent = Vector4::ZERO;
};


//-------------------------------------------------------------------------------------------------
// Basis Vertex
//
struct Vertex3D_PCU
{
	// Constructors
	Vertex3D_PCU() {};
	Vertex3D_PCU(const Vector3& position, const Rgba& color, const Vector2& texUVs)
		: m_position(position), m_color(color), m_texUVs(texUVs) {}

	Vertex3D_PCU(const VertexMaster& master)
		: m_position(master.m_position), m_color(master.m_color), m_texUVs(master.m_uvs) {}

	Vector3 m_position;
	Rgba	m_color;
	Vector2 m_texUVs;

	static const VertexAttribute	ATTRIBUTES[];
	static const VertexLayout		LAYOUT;
	static const unsigned int		NUM_ATTRIBUTES;
};


//-------------------------------------------------------------------------------------------------
// Lit Vertex
//
struct VertexLit
{
	// Constructors
	VertexLit() {};
	VertexLit(const Vector3& position, const Rgba& color, const Vector2& texUVs, const Vector3& normal, const Vector4& tangent)
		: m_position(position), m_color(color), m_texUVs(texUVs), m_normal(normal), m_tangent(tangent) {}

	// Construction from the master
	VertexLit(const VertexMaster& master)
		: m_position(master.m_position), m_color(master.m_color), m_texUVs(master.m_uvs), m_normal(master.m_normal), m_tangent(master.m_tangent)
	{
	}

	Vector3 m_position;
	Rgba	m_color;
	Vector2 m_texUVs;

	Vector3 m_normal;
	Vector4 m_tangent;	// w = 1 or -1 signals the cross direction for the bitangent

	static const VertexAttribute	ATTRIBUTES[];
	static const VertexLayout		LAYOUT;
	static const uint32				NUM_ATTRIBUTES;
};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class VertexLayout
{
public:
	//-----Public Data-----

	VertexLayout(uint32 stride, uint32 numAttributes, const VertexAttribute* attributes);

	uint32					GetAttributeCount() const;
	const VertexAttribute&	GetAttribute(uint32 index) const;
	uint32					GetStride() const;


private:
	//-----Private Data-----

	const VertexAttribute*	m_attributes;
	uint32					m_numAttributes;
	uint32					m_vertexStride;
};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
uint32 GetDXFormatForAttribute(const VertexAttribute& attribute);
