///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 23rd, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Math/Face3D.h"
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
Face3D::Face3D(const Vector3& a, const Vector3& b, const Vector3& c)
{
	AddVertex(a);
	AddVertex(b);
	AddVertex(c);
}


//-------------------------------------------------------------------------------------------------
// Creates it such that the CrossProduct(ab, ac) points in the normalDirection
Face3D::Face3D(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& normalDirection)
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
void Face3D::AddVertex(const Vector3& vertex)
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
		ASSERT_RETURN(supportPlane.ContainsPoint(vertex), NO_RETURN_VAL, "Vertex added doesn't lie within the plane!");
	}
#endif

	m_vertices.push_back(vertex);
}


//-------------------------------------------------------------------------------------------------
Vector3 Face3D::GetVertex(int vertexIndex) const
{
	ASSERT_OR_DIE(vertexIndex >= 0 && vertexIndex < (int)m_vertices.size(), "Invalid vertex index!");

	return m_vertices[vertexIndex];
}


//-------------------------------------------------------------------------------------------------
Plane Face3D::GetSupportPlane() const
{
	ASSERT_OR_DIE(m_vertices.size() >= 3, "Cannot get the plane without at least 3 points!");

	// Calculate the normal
	Vector3 ab = m_vertices[1] - m_vertices[0];
	Vector3 bc = m_vertices[2] - m_vertices[1];

	Vector3 normal = CrossProduct(ab, bc);
	normal.Normalize();

	// Get the distance
	float d = DotProduct(normal, m_vertices[0]);

	return Plane(normal, d);
}


//-------------------------------------------------------------------------------------------------
Vector3 Face3D::GetNormal() const
{
	ASSERT_RETURN(m_vertices.size() >= 3, Vector3::ZERO, "Not enough vertices to calculate a normal!");

	return CalculateNormalForTriangle(m_vertices[0], m_vertices[1], m_vertices[2]);
}


//-------------------------------------------------------------------------------------------------
// Modeled after the Polygon2D version that *does* work - not sure if this works...
bool Face3D::IsWindingClockwise(const Vector3& normal) const
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
