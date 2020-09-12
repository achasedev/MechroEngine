///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: September 9th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/Polygon2D.h"

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
Polygon2D::Polygon2D(const std::vector<Vector2> vertices)
{
	Clear();
	m_vertices = vertices;
}


//-------------------------------------------------------------------------------------------------
Polygon2D::Polygon2D(uint32 reserveSize)
{
	Clear();
	m_vertices.reserve(reserveSize);
}


//-------------------------------------------------------------------------------------------------
void Polygon2D::AddVertex(const Vector2& vertex)
{
#ifndef DISABLE_ASSERTS
	for (uint32 i = 0; i < m_vertices.size(); ++i)
	{
		if (m_vertices[i] == vertex)
		{
			ERROR_RETURN(, "Duplicate vertex pushed!");
		}
	}
#endif

	m_vertices.push_back(vertex);
}


//-------------------------------------------------------------------------------------------------
void Polygon2D::AddVertices(const std::vector<Vector2>& vertices)
{
	uint32 numVertsToAdd = vertices.size();

	for (uint32 i = 0; i < numVertsToAdd; ++i)
	{
		AddVertex(vertices[i]);
	}
}


//-------------------------------------------------------------------------------------------------
void Polygon2D::Clear()
{
	m_vertices.clear();
}


//-------------------------------------------------------------------------------------------------
void Polygon2D::Translate(const Vector2& translation)
{
	for (uint32 vertexIndex = 0; vertexIndex < m_vertices.size(); ++vertexIndex)
	{
		m_vertices[vertexIndex] += translation;
	}
}


//-------------------------------------------------------------------------------------------------
Vector2 Polygon2D::GetVertexAtIndex(uint32 index) const
{
	ASSERT_OR_DIE(index < m_vertices.size(), "Index out of bounds!");

	return m_vertices[index];
}


//-------------------------------------------------------------------------------------------------
Vector2 Polygon2D::GetFarthestVertexInDirection(const Vector2& direction) const
{
	ASSERT_OR_DIE(m_vertices.size() > 0, "No vertices to return!");

	// Early out...but this shouldn't happen
	if (m_vertices.size() == 1U)
	{
		return m_vertices[0];
	}

	float maxDot = -1.f;
	int bestIndex = -1;

	for (uint32 vertexIndex = 0; vertexIndex < m_vertices.size(); ++vertexIndex)
	{
		// Treat the vertex position as a vector from 0,0
		const Vector2& currVector = m_vertices[vertexIndex];

		float dot = DotProduct(currVector, direction);
		if (bestIndex == -1 || dot > maxDot)
		{
			maxDot = dot;
			bestIndex = vertexIndex;
		}
	}

	return m_vertices[bestIndex];
}


//-------------------------------------------------------------------------------------------------
Vector2 Polygon2D::GetCenter() const
{
	uint32 numVertices = m_vertices.size();
	ASSERT_RETURN(numVertices > 0U, Vector2::ZERO, "Polygon2D has no vertices!");

	Vector2 average = Vector2::ZERO;
	for (uint32 i = 0; i < numVertices; ++i)
	{
		average += m_vertices[i];
	}

	average /= static_cast<float>(numVertices);

	return average;
}
