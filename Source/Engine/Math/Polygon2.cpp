///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: September 9th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/Polygon2.h"

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
Polygon2::Polygon2(const std::vector<Vector2> vertices)
{
	Clear();
	m_vertices = vertices;
}


//-------------------------------------------------------------------------------------------------
Polygon2::Polygon2(uint32 reserveSize)
{
	Clear();
	m_vertices.reserve(reserveSize);
}


//-------------------------------------------------------------------------------------------------
void Polygon2::AddVertex(const Vector2& vertex)
{
#ifndef DISABLE_ASSERTS
	for (uint32 i = 0; i < (uint32)m_vertices.size(); ++i)
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
void Polygon2::AddVertices(const std::vector<Vector2>& vertices)
{
	uint32 numVertsToAdd = (uint32)vertices.size();

	for (uint32 i = 0; i < numVertsToAdd; ++i)
	{
		AddVertex(vertices[i]);
	}
}


//-------------------------------------------------------------------------------------------------
void Polygon2::Clear()
{
	m_vertices.clear();
}


//-------------------------------------------------------------------------------------------------
Vector2 Polygon2::GetVertexAtIndex(int index) const
{
	ASSERT_OR_DIE(index >= 0 && index < (int)m_vertices.size(), "Index out of bounds!");

	return m_vertices[index];
}


//-------------------------------------------------------------------------------------------------
int Polygon2::GetPreviousVertexToIndex(int index, Vector2& out_prevVertex) const
{
	int prevIndex = GetPreviousValidIndex(index);

	out_prevVertex = m_vertices[prevIndex];
	return prevIndex;
}


//-------------------------------------------------------------------------------------------------
int Polygon2::GetNextVertexToIndex(int index, Vector2& out_nextVertex) const
{
	int nextIndex = GetNextValidIndex(index);

	out_nextVertex = m_vertices[nextIndex];
	return nextIndex;
}


//-------------------------------------------------------------------------------------------------
int Polygon2::GetFarthestVertexInDirection(const Vector2& direction, Vector2& out_vertex) const
{
	ASSERT_OR_DIE(m_vertices.size() > 0, "No vertices to return!");

	// Early out...but this shouldn't happen
	if (m_vertices.size() == 1U)
	{
		out_vertex = m_vertices[0];
		return 0;
	}

	float maxDot = -1.f;
	int bestIndex = -1;

	for (int vertexIndex = 0; vertexIndex < (int)m_vertices.size(); ++vertexIndex)
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

	out_vertex = m_vertices[bestIndex];
	return bestIndex;
}


//-------------------------------------------------------------------------------------------------
Vector2 Polygon2::GetCenter() const
{
	uint32 numVertices = (uint32)m_vertices.size();
	ASSERT_RETURN(numVertices > 0U, Vector2::ZERO, "Polygon2D has no vertices!");

	Vector2 average = Vector2::ZERO;
	for (uint32 i = 0; i < numVertices; ++i)
	{
		average += m_vertices[i];
	}

	average /= static_cast<float>(numVertices);

	return average;
}


//-------------------------------------------------------------------------------------------------
int Polygon2::GetPreviousValidIndex(int index) const
{
	int numVertices = (int)m_vertices.size();
	ASSERT_OR_DIE(index >= 0 && index < numVertices, "Bad index!");
	int prevIndex = (index == 0 ? numVertices - 1 : index - 1);

	return prevIndex;
}


//-------------------------------------------------------------------------------------------------
int Polygon2::GetNextValidIndex(int index) const
{
	int numVertices = (int)m_vertices.size();
	ASSERT_OR_DIE(index >= 0 && index < numVertices, "Bad index!");
	int nextIndex = (index == numVertices - 1 ? 0 : index + 1);

	return nextIndex;
}


//-------------------------------------------------------------------------------------------------
// Works for concave polygons! Even self-intersecting ones!
// i.e figure 8 shaped polygons - it will then report whether it moves more clockwise or not
bool Polygon2::IsWindingClockwise() const
{
	uint32 numVertices = (uint32)m_vertices.size();
	float sum = 0.f;

	for (uint32 index = 0; index < m_vertices.size(); ++index)
	{
		uint32 nextIndex = (index == numVertices - 1 ? 0 : index + 1);

		Vector2 a = m_vertices[index];
		Vector2 b = m_vertices[nextIndex];

		sum += (b.x - a.x) * (a.y + b.y);
	}

	return (sum > 0.f);
}


//-------------------------------------------------------------------------------------------------
// This may not work for self-intersecting polygons
bool Polygon2::IsConvex() const
{
	size_t numVertices = m_vertices.size();
	if (numVertices <= 3)
	{
		return true;
	}

	bool isClockwiseWinding = IsWindingClockwise();

	for (size_t aIndex = 0; aIndex < numVertices; ++aIndex)
	{
		size_t bIndex = (aIndex + 1) % numVertices;
		size_t cIndex = (aIndex + 2) % numVertices;

		Vector2 a = m_vertices[aIndex];
		Vector2 b = m_vertices[bIndex];
		Vector2 c = m_vertices[cIndex];

		Vector2 ab = b - a;
		Vector2 bc = c - b;

		float cross = CrossProduct(ab, bc);

		if (isClockwiseWinding)
		{
			if (cross > 0.f)
			{
				return false;
			}
		}
		else
		{
			if (cross < 0.f)
			{
				return false;
			}
		}
	}

	return true;
}
