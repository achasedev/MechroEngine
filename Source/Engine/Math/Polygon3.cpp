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
	m_vertices = vertices;
}


//-------------------------------------------------------------------------------------------------
void Polygon3::PerformChecks(bool checkForDuplicates, bool checkForCoplanarity, bool checkForConvexity, bool checkForSelfIntersections, bool checkForColinearPoints) const
{
#ifndef DISABLE_ASSERTS
	if (checkForDuplicates)
	{
		ASSERT_OR_DIE(!HasDuplicateVertices(), "Duplicate vertex found!");
	}

	if (checkForCoplanarity)
	{
		ASSERT_OR_DIE(ArePointsCoplanar(), "Points not coplanar!");
	}

	if (checkForConvexity)
	{
		ASSERT_OR_DIE(IsConvex(), "Polygon not convex!");
	}

	if (checkForSelfIntersections)
	{
		ASSERT_OR_DIE(!IsSelfIntersecting(), "Polygon self intersects!");
	}

	if (checkForColinearPoints)
	{
		ASSERT_OR_DIE(!HasColinearPoints(), "Colinear point found!");
	}
#endif
}


//-------------------------------------------------------------------------------------------------
void Polygon3::Clear()
{
	m_vertices.clear();
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
bool Polygon3::HasDuplicateVertices() const
{
	for (int iFirst = 0; iFirst < (int)m_vertices.size() - 1; ++iFirst)
	{
		for (int iSecond = iFirst + 1; iSecond < (int)m_vertices.size(); ++iSecond)
		{
			if (AreMostlyEqual(m_vertices[iFirst], m_vertices[iSecond]))
			{
				return true;
			}
		}
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
bool Polygon3::HasColinearPoints() const
{
	if (m_vertices.size() > 2)
	{
		for (int iVertex = 0; iVertex < (int)m_vertices.size() - 2; ++iVertex)
		{
			Vector3 a = m_vertices[iVertex];
			Vector3 b = m_vertices[iVertex + 1];
			Vector3 c = m_vertices[iVertex + 2];
			bool pointsColinear = ArePointsColinear(a, b, c);

			if (pointsColinear)
				return true;
		}
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
bool Polygon3::ArePointsCoplanar() const
{
	if (m_vertices.size() > 3)
	{
		Vector3 ab = m_vertices[1] - m_vertices[0];
		Vector3 bc = m_vertices[2] - m_vertices[0];
		Vector3 normal = CrossProduct(ab, bc);
		Plane3 plane(normal, m_vertices[0]);

		for (int iVertex = 3; iVertex < (int)m_vertices.size(); ++iVertex)
		{
			float dist = plane.GetDistanceFromPlane(m_vertices[iVertex]);
			if (!AreMostlyEqual(dist, 0.f))
				return false;
		}
	}

	return true;
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
