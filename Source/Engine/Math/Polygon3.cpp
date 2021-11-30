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

//-------------------------------------------------------------------------------------------------
// "Flattens" the point to the plane x/y/z = 0 depending on compToFlatten
// The point must be on the plane before flattening, hence the plane parameter
static Vector2 FlattenPoint(const Vector3& point, int compToFlatten, const Plane3& plane)
{
	// Need to project onto plane first
	Vector3 projPt = plane.GetProjectedPointOntoPlane(point);

	Vector2 flatPt;

	switch (compToFlatten)
	{
	case 0:
		// Flatten to x = 0 plane
		flatPt = projPt.yz;
		break;
	case 1:
		// Flatten to y = 0 plane
		flatPt = projPt.xz;
		break;
	case 2:
		// Flatten to z = 0 plane
		flatPt = projPt.xy;
		break;
	default:
		ERROR_AND_DIE("Bad component index!");
		break;
	}

	return flatPt;
}

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

	Plane3 plane;
	int compToFlatten = GetComponentToFlatten(plane);

	int numVerts = (int)m_vertices.size();
	for (int iVertex = 0; iVertex < numVerts; ++iVertex)
	{
		Vector2 vert2 = FlattenPoint(m_vertices[iVertex], compToFlatten, plane);
		out_poly2.AddVertex(vert2);
	}
}


//-------------------------------------------------------------------------------------------------
Vector2 Polygon3::TransformPointInto2DBasis(const Vector3& point) const
{
	Plane3 plane;
	int compToFlatten = GetComponentToFlatten(plane);

	return FlattenPoint(point, compToFlatten, plane);
}


//-------------------------------------------------------------------------------------------------
Vector3 Polygon3::TransformPointOutOf2DBasis(const Vector2& point) const
{
	Plane3 plane;
	int zeroComp = GetComponentToFlatten(plane);

	Line3 line;
	switch (zeroComp)
	{
	case 0:
		// Flatten to x = 0 plane
		line.m_origin = Vector3(0.f, point.x, point.y);
		line.m_direction = Vector3::X_AXIS;
		break;
	case 1:
		// Flatten to y = 0 plane
		line.m_origin = Vector3(point.x, 0.f, point.y);
		line.m_direction = Vector3::Y_AXIS;
		break;
	case 2:
		// Flatten to z = 0 plane
		line.m_origin = Vector3(point.x, point.y, 0.f);
		line.m_direction = Vector3::Z_AXIS;
		break;
	default:
		ERROR_AND_DIE("Bad component index!");
		break;
	}

	Maybe<Vector3> ptOnPlane = SolveLinePlaneIntersection(line, plane);
	ASSERT_OR_DIE(ptOnPlane.IsValid(), "No solution?");

	return ptOnPlane.Get();
}


//-------------------------------------------------------------------------------------------------
void Polygon3::GetSupportPlane(Plane3& out_plane) const
{
	ASSERT_OR_DIE(m_vertices.size() > 2, "Not enough points!");

	Vector3 ab = m_vertices[1] - m_vertices[0];
	Vector3 ac = m_vertices[2] - m_vertices[0];
	Vector3 normal = CrossProduct(ab, ac);
	normal.SafeNormalize(Vector3::ZERO);
	ASSERT_OR_DIE(!AreMostlyEqual(normal, Vector3::ZERO), "Degenerate triangle!");
	out_plane = Plane3(normal, m_vertices[0]);
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
		Vector3 ac = m_vertices[2] - m_vertices[0];
		Vector3 normal = CrossProduct(ab, ac);
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
int Polygon3::GetComponentToFlatten(Plane3& out_plane) const
{
	GetSupportPlane(out_plane);

	float max = Max(Abs(out_plane.m_normal.x), Abs(out_plane.m_normal.y), Abs(out_plane.m_normal.z));

	if (max == Abs(out_plane.m_normal.y))
	{
		// Give priority to flattening on Y first
		return 1;
	}
	else if (max == Abs(out_plane.m_normal.z))
	{
		// Then prioritize z
		return 2;
	}
	else
	{
		return 0;
	}
}
