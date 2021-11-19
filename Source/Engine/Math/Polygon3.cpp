///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: Nov 18th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/Matrix3.h"
#include "Engine/Math/Polygon3.h"
#include "Engine/Math/Vector2.h"

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
Polygon3::Polygon3(const std::vector<Vector3>& vertices)
{
	for (Vector3 vertex : vertices)
	{
		AddVertex(vertex);
	}
}


//-------------------------------------------------------------------------------------------------
int Polygon3::AddVertex(const Vector3& vertex)
{
#ifndef DISABLE_ASSERTS

	// Check for duplicates
	for (int i = 0; i < (int)m_vertices.size(); ++i)
	{
		ASSERT_OR_DIE(!AreMostlyEqual(m_vertices[i], vertex), "Duplicate vertex!");	
	}

	if (m_vertices.size() > 2)
	{
		// Check all are coplanar
		Vector3 ab = m_vertices[1] - m_vertices[0];
		Vector3 bc = m_vertices[2] - m_vertices[0];
		Vector3 normal = CrossProduct(ab, bc);
		normal.Normalize();

		Plane3 plane(normal, m_vertices[0]);
		float dist = plane.GetDistanceFromPlane(vertex);
		ASSERT_OR_DIE(AreMostlyEqual(dist, 0.f), "Vertex not in plane!");
	}

	// Check that this point isn't inline with the previous 2
	if (m_vertices.size() > 1)
	{
		int iLast = (int)m_vertices.size() - 1;
		Vector3 a = m_vertices[iLast - 1];
		Vector3 b = m_vertices[iLast];

		Vector3 ab = (b - a).GetNormalized();
		Vector3 ac = (vertex - a).GetNormalized();

		ASSERT_OR_DIE(!AreMostlyEqual(DotProduct(ab, ac), 1.0f), "Vertices in line!");
	}

#endif

	m_vertices.push_back(vertex);
	ASSERT_OR_DIE(!IsSelfIntersecting(), "Polygon self intersects!");

	return (int)m_vertices.size() - 1;
}


//-------------------------------------------------------------------------------------------------
void Polygon3::TransformSelfInto2DBasis(Polygon2& out_poly2) const
{
	out_poly2.Clear();

	Matrix3 basis;
	GetBasis(basis);
	basis.Invert();

	for (Vector3 vertex3 : m_vertices)
	{
		Vector2 vertex2 = (basis * (vertex3 - m_vertices[0])).xy;
		out_poly2.AddVertex(vertex2);
	}

	// Sanity checks
	ASSERT_OR_DIE(AreMostlyEqual(out_poly2.GetVertex(0), Vector2::ZERO), "First point isn't origin!");
	ASSERT_OR_DIE(AreMostlyEqual(out_poly2.GetVertex(1).y, 0.f), "Second point isn't I vector!");
}


//-------------------------------------------------------------------------------------------------
Vector2 Polygon3::TransformPointInto2DBasis(const Vector3& point) const
{
	Matrix3 basis;
	GetBasis(basis);
	basis.Invert();

	return (basis * (point - m_vertices[0])).xy;
}


//-------------------------------------------------------------------------------------------------
Vector3 Polygon3::TransformPointOutOf2DBasis(const Vector2& point) const
{
	Matrix3 basisVectors;
	GetBasis(basisVectors);

	return basisVectors.iBasis * point.x + basisVectors.jBasis * point.y + m_vertices[0];
}


//-------------------------------------------------------------------------------------------------
bool Polygon3::IsSelfIntersecting() const
{
	if (m_vertices.size() <= 3)
		return false;

	Polygon2 poly2;
	TransformSelfInto2DBasis(poly2);

	return poly2.IsSelfIntersecting();
}


//-------------------------------------------------------------------------------------------------
bool Polygon3::IsConvex() const
{
	if (m_vertices.size() <= 3)
		return true;

	Polygon2 poly2;
	TransformSelfInto2DBasis(poly2);

	return poly2.IsConvex();
}


//-------------------------------------------------------------------------------------------------
void Polygon3::GetBasis(Matrix3& out_bases) const
{
	ASSERT_OR_DIE(m_vertices.size() > 2, "Not enough vertices!");

	Vector3 i = (m_vertices[1] - m_vertices[0]).GetNormalized();
	Vector3 inPlaneRef = (m_vertices[2] - m_vertices[0]); // Keep things ortho
	Vector3 k = CrossProduct(i, inPlaneRef).GetNormalized();
	Vector3 j = CrossProduct(k, i);

	out_bases = Matrix3(i, j, k);
}
