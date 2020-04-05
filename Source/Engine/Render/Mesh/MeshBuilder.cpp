///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: February 15th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Render/Mesh/MeshBuilder.h"
#include "Engine/Math/AABB3.h"
#include "Engine/Math/OBB2.h"
#include "Engine/Math/MathUtils.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
MeshBuilder::~MeshBuilder()
{
	Clear();
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::BeginBuilding(bool useIndices)
{
	ASSERT_RECOVERABLE(!m_isBuilding, "You're already building!");

	Clear();

	m_instruction.m_useIndices = useIndices;
	m_isBuilding = true;
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::FinishBuilding()
{
	if (m_instruction.m_useIndices) { m_instruction.m_elementCount = (uint32)m_indices.size(); }
	else							{ m_instruction.m_elementCount = (uint32)m_vertices.size(); }

	m_isBuilding = false;	
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::Clear()
{
	m_stamp = VertexMaster();

	m_vertices.clear();
	m_indices.clear();
	m_isBuilding = false;
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::SetColor(const Rgba& color)
{
	m_stamp.m_color = color;
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::SetUV(const Vector2& uv)
{
	m_stamp.m_uvs = uv;
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::SetNormal(const Vector3& normal)
{
	m_stamp.m_normal = normal;
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::SetTangent(const Vector4& tangent)
{
	m_stamp.m_tangent = tangent;
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::SetDrawInstruction(const DrawInstruction& instruction)
{
	m_instruction = instruction;
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::SetDrawInstruction(bool useIndices, uint32 startIndex, uint32 elementCount)
{
	m_instruction = DrawInstruction(useIndices, startIndex, elementCount);
}

//-------------------------------------------------------------------------------------------------
uint32 MeshBuilder::PushVertex(const Vector3& position)
{
	m_stamp.m_position = position;

	m_vertices.push_back(m_stamp);
	return (uint32)m_vertices.size() - 1;
}


//-------------------------------------------------------------------------------------------------
uint32 MeshBuilder::PushVertex(const VertexMaster& master)
{
	m_stamp = master;
	m_vertices.push_back(m_stamp);
	return (uint32)m_vertices.size() - 1;
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::PushIndex(uint32 index)
{
	ASSERT_RECOVERABLE(m_instruction.m_useIndices, "Pushing indices with a non-indexed builder");
	m_indices.push_back(index);
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::PushTriangle2D(const Vector2& first, const Vector2& second, const Vector2& third, const Rgba& tint /*= Rgba::WHITE*/)
{
	Vector3 first3D		= Vector3(first, 0.f);
	Vector3 second3D	= Vector3(second, 0.f);
	Vector3 third3D		= Vector3(third, 0.f);

	PushTriangle3D(first3D, second3D, third3D, tint);
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::PushQuad2D(const AABB2& quad, const AABB2& uvs /*= AABB2::ZERO_TO_ONE*/, const Rgba& tint /*= Rgba::WHITE*/)
{
	Vector3 bottomLeft		= Vector3(quad.GetBottomLeft(), 0.f);
	Vector3 topLeft			= Vector3(quad.GetTopLeft(), 0.f);
	Vector3 topRight		= Vector3(quad.GetTopRight(), 0.f);
	Vector3 bottomRight		= Vector3(quad.GetBottomRight(), 0.f);

	PushQuad3D(bottomLeft, topLeft, topRight, bottomRight, uvs, tint);
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::PushTriangle3D(const Vector3& first, const Vector3& second, const Vector3& third, const Rgba& tint /*= Rgba::WHITE*/)
{
	SetColor(tint);

	uint32 index = PushVertex(first);
	PushVertex(second);
	PushVertex(third);

	if (m_instruction.m_useIndices)
	{
		PushIndex(index + 0);
		PushIndex(index + 1);
		PushIndex(index + 2);
	}
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::PushQuad3D(const Vector3& bottomLeft, const Vector3& topLeft, const Vector3& topRight, const Vector3& bottomRight, const AABB2& uvs /*= AABB2::ZERO_TO_ONE*/, const Rgba& tint /*= Rgba::WHITE*/)
{
	// Calculate normal
	Vector3 quadRight = (bottomRight - bottomLeft).GetNormalized();
	Vector3 quadUp = (topRight - bottomRight).GetNormalized();
	Vector3 normal = CrossProduct(quadUp, quadRight);

	// Begin adding to the mesh
	SetColor(tint);
	SetNormal(normal);
	SetTangent(Vector4(quadRight, 1.0f));

	if (m_instruction.m_useIndices)
	{
		//-----Push the Vertices-----
		// Bottom Left
		SetUV(uvs.GetBottomLeft());
		uint32 index = PushVertex(bottomLeft);

		// Top Left
		SetUV(uvs.GetTopLeft());
		PushVertex(topLeft);

		// Top Right
		SetUV(uvs.GetTopRight());
		PushVertex(topRight);

		// Bottom Right
		SetUV(uvs.GetBottomRight());
		PushVertex(bottomRight);

		//-----Push the indices-----
		PushIndex(index + 0);
		PushIndex(index + 1);
		PushIndex(index + 2);
		PushIndex(index + 0);
		PushIndex(index + 2);
		PushIndex(index + 3);
	}
	else
	{
		//-----Push the Vertices-----
		// Bottom Left
		SetUV(uvs.GetBottomLeft());
		PushVertex(bottomLeft);

		// Top Left
		SetUV(uvs.GetTopLeft());
		PushVertex(topLeft);

		// Top Right
		SetUV(uvs.GetTopRight());
		PushVertex(topRight);

		// Bottom Left
		SetUV(uvs.GetBottomLeft());
		PushVertex(bottomLeft);

		// Top Right
		SetUV(uvs.GetTopRight());
		PushVertex(topRight);

		// Bottom Right
		SetUV(uvs.GetBottomRight());
		PushVertex(bottomRight);
	}
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::PushQuad3D(
	const Vector3& position, 
	const Vector2& dimensions, 
	const AABB2& uvs /*= AABB2::ZERO_TO_ONE*/,
	const Rgba& tint /*= Rgba::WHITE*/, 
	const Vector3& rightVector /*= Vector3::X_AXIS*/, 
	const Vector3& upVector /*= Vector3::Y_AXIS*/, 
	const Vector2& pivot /*= Vector2(0.5f, 0.5f)*/)
{
	ASSERT_OR_DIE(m_isBuilding, "MeshBuilder not building!");

	//-----Set up the vertices-----
	// Find the min and max values for the AABB2 draw bounds
	float minX = -1.0f * (pivot.x * dimensions.x);
	float maxX = minX + dimensions.x;
	float minY = -1.0f * (pivot.y * dimensions.y);
	float maxY = minY + dimensions.y;

	// Determine corner locations based on the alignment vectors
	Vector3 bottomLeft	= position + minX * rightVector + minY * upVector;
	Vector3 bottomRight = position + maxX * rightVector + minY * upVector;
	Vector3 topLeft		= position + minX * rightVector + maxY * upVector;
	Vector3 topRight	= position + maxX * rightVector + maxY * upVector;

	PushQuad3D(bottomLeft, topLeft, topRight, bottomRight, uvs, tint);
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::PushCube(
	const Vector3& center, 
	const Vector3& dimensions, 
	const AABB2& topUVs /*= AABB2::ZERO_TO_ONE*/, 
	const AABB2& sideUVs /*= AABB2::ZERO_TO_ONE*/, 
	const AABB2& bottomUVs /*= AABB2::ZERO_TO_ONE*/, 
	const Rgba& tint /*= Rgba::WHITE*/)
{
	ASSERT_OR_DIE(m_isBuilding, "MeshBuilder not building!");

	// Set up the corner vertices
	AABB3 cubeBounds = AABB3(center - dimensions * 0.5f, center + dimensions * 0.5f);

	//-------------------------------------Back face--------------------------------------------------------
	{
		Vector3 position = Vector3(center.x, center.y, cubeBounds.mins.z);
		PushQuad3D(position, Vector2(dimensions.x, dimensions.y), sideUVs, tint, Vector3::X_AXIS, Vector3::Y_AXIS, Vector2(0.5f, 0.5f));
	}

	//-------------------------------------Front face--------------------------------------------------------
	{
		Vector3 position = Vector3(center.x, center.y, cubeBounds.maxs.z);
		PushQuad3D(position, Vector2(dimensions.x, dimensions.y), sideUVs, tint, Vector3::MINUS_X_AXIS, Vector3::Y_AXIS, Vector2(0.5f, 0.5f));
	}

	//-------------------------------------Left face--------------------------------------------------------
	{
		Vector3 position = Vector3(cubeBounds.mins.x, center.y, center.z);
		PushQuad3D(position, Vector2(dimensions.z, dimensions.y), sideUVs, tint, Vector3::MINUS_Z_AXIS, Vector3::Y_AXIS, Vector2(0.5f, 0.5f));
	}

	//-------------------------------------Right face--------------------------------------------------------
	{
		Vector3 position = Vector3(cubeBounds.maxs.x, center.y, center.z);
		PushQuad3D(position, Vector2(dimensions.z, dimensions.y), sideUVs, tint, Vector3::Z_AXIS, Vector3::Y_AXIS, Vector2(0.5f, 0.5f));
	}

	//-------------------------------------Top face--------------------------------------------------------
	{
		Vector3 position = Vector3(center.x, cubeBounds.maxs.y, center.z);
		PushQuad3D(position, Vector2(dimensions.x, dimensions.z), topUVs, tint, Vector3::X_AXIS, Vector3::Z_AXIS, Vector2(0.5f, 0.5f));
	}

	//-------------------------------------Bottom face--------------------------------------------------------
	{
		Vector3 position = Vector3(center.x, cubeBounds.mins.y, center.z);
		PushQuad3D(position, Vector2(dimensions.x, dimensions.z), bottomUVs, tint, Vector3::X_AXIS, Vector3::MINUS_Z_AXIS, Vector2(0.5f, 0.5f));
	}
}
