///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 23rd, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Math/Face3.h"
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
Face3::Face3(const Vector3& a, const Vector3& b, const Vector3& c)
{
	AddVertex(a);
	AddVertex(b);
	AddVertex(c);
}


//-------------------------------------------------------------------------------------------------
// Creates it such that the CrossProduct(ab, ac) points in the normalDirection
Face3::Face3(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& normalDirection)
{
	Vector3 abcNormal = CalculateNormalForTriangle(a, b, c);

	float dotProduct = DotProduct(abcNormal, normalDirection);

	if (dotProduct > 0.f)
	{
		AddVertex(a);
		AddVertex(b);
		AddVertex(c);
	}
	else
	{
		AddVertex(a);
		AddVertex(c);
		AddVertex(b);
	}
}


//-------------------------------------------------------------------------------------------------
void Face3::AddVertex(const Vector3& vertex)
{
#ifndef DISABLE_ASSERTS
	// Check for duplicates
	for (int index = 0; index < (int)m_vertices.size(); ++index)
	{
		ASSERT_RETURN(!AreMostlyEqual(vertex, m_vertices[index]), NO_RETURN_VAL, "Duplicate vertex position!");
	}

	// Faces need to exist in a plane, so check for that
	if (m_vertices.size() >= 3)
	{
		Plane supportPlane = GetSupportPlane();
		bool containsPoint = supportPlane.ContainsPoint(vertex);

		if (!containsPoint)
		{
			supportPlane = GetSupportPlane();
			containsPoint = supportPlane.ContainsPoint(vertex);
		}

		ASSERT_RETURN(containsPoint, NO_RETURN_VAL, "Vertex added doesn't lie within the plane!");
	}
#endif

	m_vertices.push_back(vertex);
}


//-------------------------------------------------------------------------------------------------
Vector3 Face3::GetVertex(int vertexIndex) const
{
	ASSERT_OR_DIE(vertexIndex >= 0 && vertexIndex < (int)m_vertices.size(), "Invalid vertex index!");

	return m_vertices[vertexIndex];
}


//-------------------------------------------------------------------------------------------------
Edge3 Face3::GetEdge(int edgeIndex) const
{
	int numVertices = (int)m_vertices.size();
	ASSERT_OR_DIE(edgeIndex >= 0 && edgeIndex < numVertices, "Invalid edge index!");

	const Vector3& firstVertex = m_vertices[edgeIndex];
	const Vector3& secondVertex = m_vertices[(edgeIndex + 1) % numVertices];

	return Edge3(firstVertex, secondVertex);
}


//-------------------------------------------------------------------------------------------------
Plane Face3::GetSupportPlane() const
{
	ASSERT_OR_DIE(m_vertices.size() >= 3, "Cannot get the plane without at least 3 points!");

	// Calculate the normal
	Vector3 ab = m_vertices[1] - m_vertices[0];
	Vector3 ac = m_vertices[2] - m_vertices[0];

	Vector3 normal = CrossProduct(ab, ac);
	normal.Normalize();

	// Get the distance
	float d = DotProduct(normal, m_vertices[0]);

	return Plane(normal, d);
}


//-------------------------------------------------------------------------------------------------
Vector3 Face3::GetNormal() const
{
	ASSERT_RETURN(m_vertices.size() >= 3, Vector3::ZERO, "Not enough vertices to calculate a normal!");

	return CalculateNormalForTriangle(m_vertices[0], m_vertices[1], m_vertices[2]);
}


//-------------------------------------------------------------------------------------------------
float Face3::CalculateArea() const
{
	int numVertices = (int)m_vertices.size();
	ASSERT_RETURN(m_vertices.size() > 2, 0.f, "Not enough vertices to make an area!");

	Vector3 a = m_vertices[0];
	float area = 0.f;

	for (int vertexIndex = 1; vertexIndex < numVertices - 1; ++vertexIndex)
	{
		Vector3 b = m_vertices[vertexIndex];
		Vector3 c = m_vertices[vertexIndex + 1];

		Vector3 ab = b - a;
		Vector3 ac = c - a;

		Vector3 cross = CrossProduct(ab, ac);
		float currArea = 0.5f * cross.GetLength();

		area += currArea;
	}

	return area;
}


//-------------------------------------------------------------------------------------------------
// Modeled after the Polygon2D version that *does* work - not sure if this works...
bool Face3::IsWindingClockwise(const Vector3& normal) const
{
	uint32 numVertices = (uint32)m_vertices.size();

	for (uint32 aIndex = 0; aIndex < m_vertices.size(); ++aIndex)
	{
		uint32 bIndex = (aIndex + 1) % numVertices;
		uint32 cIndex = (aIndex + 2) % numVertices;

		Vector3 a = m_vertices[aIndex];
		Vector3 b = m_vertices[bIndex];
		Vector3 c = m_vertices[cIndex];

		Vector3 ab = b - a;
		Vector3 ac = c - a;

		if (DotProduct(CrossProduct(ab, ac), normal) < 0.f)
		{
			return false;
		}
	}

	return true;
}


//-------------------------------------------------------------------------------------------------
bool Face3::IsEquivalentTo(const Face3& face) const
{
	if (m_vertices.size() != face.m_vertices.size())
	{
		return false;
	}

	// Degenerate case
	if (m_vertices.size() == 0)
	{
		return true;
	}

	// Find my first vertex in the other
	Vector3 startVertex = m_vertices[0];
	int startOffset = -1;

	for (int otherIndex = 0; otherIndex < (int)face.m_vertices.size(); ++otherIndex)
	{
		if (AreMostlyEqual(startVertex, face.m_vertices[otherIndex]))
		{
			startOffset = otherIndex;
			break;
		}
	}

	// Couldn't find my first vertex, so we cannot be the same
	if (startOffset < 0)
	{
		return false;
	}

	// Check each vertex after that in order, ensure all match
	for (int myIndex = 0; myIndex < (int)m_vertices.size(); ++myIndex)
	{
		int otherIndex = (myIndex + startOffset) % (int)(m_vertices.size());

		if (!AreMostlyEqual(m_vertices[myIndex], face.m_vertices[otherIndex]))
		{
			return false;
		}
	}

	return true;
}
