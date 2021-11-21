///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: Nov 16th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/Math/GJK.h"
#include "Engine/Math/LineSegment2.h"
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
GJKSolver2D::GJKSolver2D()
{
	for (int i = 0; i < 3; ++i)
	{
		m_iVert[i] = -1;
	}
}


//-------------------------------------------------------------------------------------------------
float GJKSolver2D::Solve(const Vector2& point, const Polygon2* poly, Vector2& out_closestPt)
{
	ASSERT_RETURN(poly->IsConvex(), 0.f, "Polygon not convex!");

	m_point = point;
	m_poly = poly;

	bool done = false;
	while (!done)
	{
		switch (m_numVerts)
		{
		case 0:
			StartEvolution();
			break;
		case 1:
			done = EvolveFromPoint();
			break;
		case 2:
			done = EvolveFromSegment();
			break;
		case 3:
			done = EvolveFromTriangle();
			break;
		default:
			ERROR_AND_DIE("Bad number of verts!");
			break;
		}
	}

	out_closestPt = m_closestPt;
	return m_minDist;
}


//-------------------------------------------------------------------------------------------------
float GJKSolver2D::Solve(const Vector3& point, const Polygon3* poly3, Vector3& out_closestPt)
{
	ASSERT_RETURN(poly3->IsConvex(), 0.f, "Polygon not convex!");

	// Get poly and point in 2D basis
	Polygon2 poly2;
	poly3->TransformSelfInto2DBasis(poly2);
	Vector2 point2 = poly3->TransformPointInto2DBasis(point);
	Vector3 backtoWorld = poly3->TransformPointOutOf2DBasis(point2);
	// Solve in 2D
	Vector2 closestPt2;
	Solve(point2, &poly2, closestPt2);

	// Convert back to 3D
	out_closestPt = poly3->TransformPointOutOf2DBasis(closestPt2);
	return (out_closestPt - point).GetLength();
}


//-------------------------------------------------------------------------------------------------
void GJKSolver2D::StartEvolution()
{
	// Arbitrarily choose a to be the first vertex
	m_iA = 0;
	m_numVerts = 1;
}


//-------------------------------------------------------------------------------------------------
bool GJKSolver2D::EvolveFromPoint()
{
	Vector2 aToPt = m_point - m_poly->GetVertex(m_iA);

	Vector2 b;
	m_iB = m_poly->GetSupportPoint(aToPt, b);
	m_numVerts = 2;

	if (m_iA == m_iB)
	{
		// Closest point is a vertex
		m_closestPt = b;
		m_minDist = (m_point - m_closestPt).GetLength();
		return true;
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
bool GJKSolver2D::EvolveFromSegment()
{
	Vector2 a = m_poly->GetVertex(m_iA);
	Vector2 b = m_poly->GetVertex(m_iB);

	Vector2 aToB = b - a;
	Vector2 searchDir = Vector2(-1.0f * aToB.y, aToB.x);
	Vector2 aToPt = m_point - a;

	// Make sure we search towards the point
	if (DotProduct(aToPt, searchDir) < 0.f)
	{
		searchDir *= -1.0f;
	}
	
	Vector2 c;
	m_iC = m_poly->GetSupportPoint(searchDir, c);
	m_numVerts = 3;

	if (m_iC == m_iA || m_iC == m_iB)
	{
		// Closest point will be on this segment
		m_minDist = FindNearestPoint(m_point, LineSegment2(a, b), m_closestPt);
		return true;
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
bool GJKSolver2D::EvolveFromTriangle()
{
	Vector2 a = m_poly->GetVertex(m_iA);
	Vector2 b = m_poly->GetVertex(m_iB);
	Vector2 c = m_poly->GetVertex(m_iC);
	Vector3 baryCoords = ComputeBarycentricCoordinates(m_point, Triangle2(a, b, c));

	if (baryCoords.u >= 0.f && baryCoords.v >= 0.f && baryCoords.w >= 0.f)
	{
		// Point is inside the simplex
		m_minDist = 0.f;
		m_closestPt.xy = m_point;
		return true;
	}

	// A doesn't contribute
	if (baryCoords.u <= 0.f)
	{
		m_iA = -1;
		m_numVerts--;
	}

	// B doesn't contribute
	if (baryCoords.v <= 0.f)
	{
		m_iB = -1;
		m_numVerts--;
	}

	// C doesn't contribute
	if (baryCoords.w <= 0.f)
	{
		m_iC = -1;
		m_numVerts--;
	}

	ASSERT_OR_DIE(m_numVerts > 0, "Removed all the points?");
	CleanUpVertices();

	return false;
}


//-------------------------------------------------------------------------------------------------
// Shift elements to fill in gaps, maintaining order
void GJKSolver2D::CleanUpVertices()
{
	bool done = false;
	while (!done)
	{
		done = true;
		for (int i = 0; i < 2; ++i)
		{
			if (m_iVert[i] == -1 && m_iVert[i + 1] != -1)
			{
				m_iVert[i] = m_iVert[i + 1];
				m_iVert[i + 1] = -1;
				done = false;
			}
		}
	}
}


//-------------------------------------------------------------------------------------------------
GJKSolver3D::GJKSolver3D()
{
	for (int i = 0; i < 4; ++i)
	{
		m_iVert[i] = -1;
	}
}


//-------------------------------------------------------------------------------------------------
float GJKSolver3D::Solve(const Vector3& point, const Polyhedron* poly, Vector3& out_closestPt)
{
	if (!poly->IsConvex())
	{
		bool test = poly->IsConvex();
		ERROR_RETURN(0.f, "Polygon not convex!");
	}

	m_point = point;
	m_poly = poly;

	bool done = false;
	while (!done)
	{
		switch (m_numVerts)
		{
		case 0:
			StartEvolution();
			break;
		case 1:
			done = EvolveFromPoint();
			break;
		case 2:
			done = EvolveFromSegment();
			break;
		case 3:
			done = EvolveFromTriangle();
			break;
		case 4:
			done = EvolveFromTetrahedron();
			break;
		default:
			ERROR_AND_DIE("Bad number of verts!");
			break;
		}
	}

	out_closestPt = m_closestPt;
	return m_minDist;
}


//-------------------------------------------------------------------------------------------------
void GJKSolver3D::StartEvolution()
{
	int numVerts = m_poly->GetNumVertices();
	float minDistSqr = -1.f;
	int minIndex = -1;
	for (int i = 0; i < numVerts; ++i)
	{
		float distSqr = (m_point - m_poly->GetVertexPosition(i)).GetLengthSquared();

		if (i == 0 || distSqr < minDistSqr)
		{
			minDistSqr = distSqr;
			minIndex = i;
		}
	}

	m_iA = minIndex;
	m_numVerts = 1;
}


//-------------------------------------------------------------------------------------------------
bool GJKSolver3D::EvolveFromPoint()
{
	Vector3 aToPt = m_point - m_poly->GetVertexPosition(m_iA);

	Vector3 b;
	m_iB = m_poly->GetSupportPoint(aToPt, b);
	m_numVerts = 2;

	if (m_iA == m_iB)
	{
		// Closest point is a vertex
		m_closestPt = b;
		m_minDist = (m_point - m_closestPt).GetLength();
		return true;
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
bool GJKSolver3D::EvolveFromSegment()
{
	Vector3 a = m_poly->GetVertexPosition(m_iA);
	Vector3 b = m_poly->GetVertexPosition(m_iB);

	Vector3 aToB = b - a;
	Vector3 aToPt = m_point - a;
	Vector3 ref = CrossProduct(aToPt, aToB);
	Vector3 searchDir = CrossProduct(aToB, ref);

	// Make sure we search towards the point
	if (DotProduct(aToPt, searchDir) < 0.f)
	{
		searchDir *= -1.0f;
	}

	Vector3 c;
	m_iC = m_poly->GetSupportPoint(searchDir, c);
	m_numVerts = 3;

	if (m_iC == m_iA || m_iC == m_iB)
	{
		// Closest point will be on this segment
		m_minDist = FindNearestPoint(m_point, LineSegment3(a, b), m_closestPt);
		return true;
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
bool GJKSolver3D::EvolveFromTriangle()
{
	Vector3 a = m_poly->GetVertexPosition(m_iA);
	Vector3 b = m_poly->GetVertexPosition(m_iB);
	Vector3 c = m_poly->GetVertexPosition(m_iC);

	Vector3 ab = b - a;
	Vector3 ac = c - a;
	Vector3 normal = CrossProduct(ab, ac);

	Vector3 aToPt = m_point - a;

	// Make sure we search towards the point
	if (DotProduct(aToPt, normal) < 0.f)
	{
		normal *= -1.0f;
	}

	Vector3 d;
	m_iD = m_poly->GetSupportPoint(normal, d);
	m_numVerts = 4;

	Tetrahedron tetra(a, b, c, d);
	bool dIsDuplicate = (m_iD == m_iA) || (m_iD == m_iB) || (m_iD == m_iC);
	bool tetraDegenerate = (AreMostlyEqual(tetra.CalculateUnsignedVolume(), 0.f));

	if (dIsDuplicate || tetraDegenerate)
	{
		m_iD = -1;
		m_numVerts--;

		Vector3 triBaryCoords = ComputeBarycentricCoordinates(m_point, Triangle3(a, b, c));

		if (triBaryCoords.u >= 0.f && triBaryCoords.v >= 0.f && triBaryCoords.w >= 0.f)
		{
			m_closestPt = triBaryCoords.u * a + triBaryCoords.v * b + triBaryCoords.w * c;
			m_minDist = (m_closestPt - m_point).GetLength();
			return true;
		}

		// Remove the least contributing point
		// Don't remove more than 1 point, as it creates infinite loop issues (cone case)
		float minCoord = Min(triBaryCoords.u, triBaryCoords.v, triBaryCoords.w);

		if (minCoord == triBaryCoords.u)
		{
			m_iA = -1;
			m_numVerts--;
		}
		else if (minCoord == triBaryCoords.v)
		{
			m_iB = -1;
			m_numVerts--;
		}
		else if (minCoord == triBaryCoords.w)
		{
			m_iC = -1;
			m_numVerts--;
		}

		CleanUpVertices();
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
bool GJKSolver3D::EvolveFromTetrahedron()
{
	Vector3 a = m_poly->GetVertexPosition(m_iA);
	Vector3 b = m_poly->GetVertexPosition(m_iB);
	Vector3 c = m_poly->GetVertexPosition(m_iC);
	Vector3 d = m_poly->GetVertexPosition(m_iD);
	Tetrahedron tetra(a, b, c, d);

	Vector4 baryCoords = ComputeBarycentricCoordinates(m_point, tetra);

	if (baryCoords.x >= 0.f && baryCoords.y >= 0.f && baryCoords.z >= 0.f && baryCoords.w >= 0.f)
	{
		// Point is inside tetrahedron == point is inside the polygon
		// We don't know the penetration, so just return 0
		m_minDist = 0.f;
		m_closestPt = m_point;
		return true;
	}

	// Point is outside tetrahedron, so evolve
	// Only drop one point since there's infinite loop issues related to dropping multiple points (cone issue)
	// So - if there's more than one negative coordinate, choose to drop the one that leaves behind a triangle with a normal that'd be the best search direction for the next iteration
	float bestDot = -1.f;
	int coordinateToRemove = -1;

	if (baryCoords.x < 0.f) // A doesn't contribute
	{
		Vector3 bc = c - b;
		Vector3 bd = d - b;
		Vector3 normal = CrossProduct(bc, bd).GetNormalized();
		float dot = Abs(DotProduct(normal, m_point - b)); // Abs since we don't care which direction, we flip as needed later

		if (dot > bestDot)
		{
			bestDot = dot;
			coordinateToRemove = 0;
		}
	}
	
	if (baryCoords.y < 0.f) // B doesn't contribute
	{
		Vector3 ac = c - a;
		Vector3 ad = d - a;
		Vector3 normal = CrossProduct(ac, ad).GetNormalized();
		float dot = Abs(DotProduct(normal, m_point - a));

		if (dot > bestDot)
		{
			bestDot = dot;
			coordinateToRemove = 1;
		}
	}
	
	if (baryCoords.z < 0.f) // C doesn't contribute
	{
		Vector3 ab = b - a;
		Vector3 ad = d - a;
		Vector3 normal = CrossProduct(ab, ad).GetNormalized();
		float dot = Abs(DotProduct(normal, m_point - a));

		if (dot > bestDot)
		{
			bestDot = dot;
			coordinateToRemove = 2;
		}
	}
	
	if (baryCoords.w < 0.f) // D doesn't contribute
	{
		Vector3 ab = b - a;
		Vector3 ac = c - a;
		Vector3 normal = CrossProduct(ab, ac).GetNormalized();
		float dot = Abs(DotProduct(normal, m_point - a));

		if (dot > bestDot)
		{
			bestDot = dot;
			coordinateToRemove = 3;
		}
	}

	ASSERT_OR_DIE(coordinateToRemove >= 0, "No coordinate selected?");

	switch (coordinateToRemove)
	{
	case 0:
		m_iA = -1;
		break;
	case 1: 
		m_iB = -1;
		break;
	case 2: 
		m_iC = -1;
		break;
	case 3: 
		m_iD = -1;
		break;
	default:
		break;
	}

	m_numVerts--;

	ASSERT_OR_DIE(m_numVerts > 0, "Removed all the points?");
	CleanUpVertices();

	return false;
}


//-------------------------------------------------------------------------------------------------
void GJKSolver3D::CleanUpVertices()
{
	bool done = false;
	while (!done)
	{
		done = true;
		for (int i = 0; i < (4 - 1); ++i)
		{
			if (m_iVert[i] == -1 && m_iVert[i + 1] != -1)
			{
				m_iVert[i] = m_iVert[i + 1];
				m_iVert[i + 1] = -1;
				done = false;
			}
		}
	}
}
