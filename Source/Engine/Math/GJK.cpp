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
GJKSolver::GJKSolver()
{
	for (int i = 0; i < MAX_SIMPLEX_VERTS; ++i)
	{
		m_iVert[i] = -1;
	}
}


//-------------------------------------------------------------------------------------------------
float GJKSolver::Solve(const Vector2& point, const Polygon2* poly, Vector2& out_closestPt)
{
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
float GJKSolver::Solve(const Vector3& point, const Polygon3* poly3, Vector3& out_closestPt)
{
	// Get poly and point in 2D basis
	Polygon2 poly2;
	poly3->TransformSelfInto2DBasis(poly2);
	Vector2 point2 = poly3->TransformPointInto2DBasis(point);

	// Solve in 2D
	Vector2 closestPt2;
	Solve(point2, &poly2, closestPt2);

	// Convert back to 3D
	out_closestPt = poly3->TransformPointOutOf2DBasis(closestPt2);
	return (out_closestPt - point).GetLength();
}


//-------------------------------------------------------------------------------------------------
void GJKSolver::StartEvolution()
{
	// Arbitrarily choose a to be the first vertex
	m_iA = 0;
	m_numVerts = 1;
}


//-------------------------------------------------------------------------------------------------
bool GJKSolver::EvolveFromPoint()
{
	Vector2 aToPt = m_point - m_poly->GetVertex(m_iA);

	Vector2 b;
	m_iB = m_poly->GetSupportPoint(aToPt, b);
	m_numVerts = 2;

	if (m_iA == m_iB)
	{
		// Closest point is a vertex
		m_closestPt.xy = b;
		m_minDist = (m_point - m_closestPt.xy).GetLength();
		return true;
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
bool GJKSolver::EvolveFromSegment()
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
		Vector2 closestPt;
		m_minDist = FindNearestPoint(m_point, LineSegment2(a, b), closestPt);
		m_closestPt.xy = closestPt;
		return true;
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
bool GJKSolver::EvolveFromTriangle()
{
	Vector2 a = m_poly->GetVertex(m_iA);
	Vector2 b = m_poly->GetVertex(m_iB);
	Vector2 c = m_poly->GetVertex(m_iC);
	Vector3 baryCoords = ComputeBarycentricCoordinates(m_point, Triangle2(a, b, c));

	if (baryCoords.u > 0.f && baryCoords.v > 0.f && baryCoords.w > 0.f)
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
void GJKSolver::CleanUpVertices()
{
	bool done = false;
	while (!done)
	{
		done = true;
		for (int i = 0; i < MAX_SIMPLEX_VERTS; ++i)
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
