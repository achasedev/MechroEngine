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

#ifndef DISABLE_ASSERTS
	// Check for clockwise winding order
	ASSERT_RECOVERABLE(IsWindingClockwise(), "Vertices not winding clockwise!");
#endif
}


//-------------------------------------------------------------------------------------------------
Vector3 Face3D::GetVertex(int vertexIndex)
{
	ASSERT_OR_DIE(vertexIndex >= 0 && vertexIndex < (int)m_vertices.size(), "Invalid vertex index!");

	return m_vertices[vertexIndex];
}


//-------------------------------------------------------------------------------------------------
Plane Face3D::GetSupportPlane() const
{
	ASSERT_OR_DIE(m_vertices.size() >= 3, "Cannot get the plane without at least 3 points!");
	ASSERT_OR_DIE(IsWindingClockwise(), "Keep the winding clockwise for now!");

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
// Modeled after the Polygon2D version that *does* work - not sure if this works...
bool Face3D::IsWindingClockwise() const
{
	uint32 numVertices = (uint32)m_vertices.size();
	float sum = 0.f;

	for (uint32 index = 0; index < m_vertices.size(); ++index)
	{
		uint32 nextIndex = (index == numVertices - 1 ? 0 : index + 1);

		Vector3 a = m_vertices[index];
		Vector3 b = m_vertices[nextIndex];

		sum += (b.x - a.x) * (a.y + b.y) * (a.z + b.z);
	}

	return (sum > 0.f);
}
