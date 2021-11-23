///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: Nov 21st, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/Math/GJK.h"
#include "Engine/Math/LineSegment3.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/Matrix3.h"

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
template <class A, class B>
GJKSolver3D<A, B>::GJKSolver3D(const A& a, const B& b)
	: m_a(a), m_b(b)
{
	for (int i = 0; i < 4; ++i)
	{
		m_simplexPts[i].Invalidate();
	}
}


//-------------------------------------------------------------------------------------------------
template <class A, class B>
GJKSolver3D<A, B>::GJKSolver3D(const GJKSolver3D<A, B>& copy)
	: m_a(copy.m_a), m_b(copy.m_b)
{
	m_simplexA = copy.m_simplexA;
	m_simplexB = copy.m_simplexB;
	m_simplexC = copy.m_simplexC;
	m_simplexD = copy.m_simplexD;
	m_numVerts = copy.m_numVerts;
	m_separationNormal = copy.m_separationNormal;
	m_separation = copy.m_separation;
}


//-------------------------------------------------------------------------------------------------
template <class A, class B>
void GJKSolver3D<A, B>::Solve()
{
	bool done = false;
	while (!done)
	{
		ExpandSimplex();

		switch (m_numVerts)
		{
		case 1:
			// Do nothing
			break;
		case 2:
			done = CheckSimplexLineSegment();
			break;
		case 3:
			done = CheckSimplexTriangle();
			break;
		case 4:
			done = CheckSimplexTetrahedron();
			break;
		default:
			ERROR_AND_DIE("Bad number of verts!");
			break;
		}
	}

	if (m_separation > 0.f)
	{
		ComputeClosestPoints();
	}
}


//-------------------------------------------------------------------------------------------------
template <class A, class B>
bool GJKSolver3D<A, B>::CheckSimplexLineSegment()
{
	if (IsSimplexDegenerate())
	{
		m_simplexB.Invalidate();
		m_numVerts--;

		// Closest point is a vertex (simplexA == simplexB)
		m_separationNormal = (m_simplexA.Get() - Vector3::ZERO);
		m_separation = m_separationNormal.Normalize();
		return true;
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
template <class A, class B>
bool GJKSolver3D<A, B>::CheckSimplexTriangle()
{
	if (IsSimplexDegenerate())
	{
		m_simplexC.Invalidate();
		m_numVerts--;

		// Closest point will be on this segment
		Vector3 closestPt;
		m_separation = FindNearestPoint(Vector3::ZERO, LineSegment3(m_simplexA.Get(), m_simplexB.Get()), closestPt);
		m_separationNormal = (closestPt - Vector3::ZERO) / m_separation;
		return true;
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
template <class A, class B>
bool GJKSolver3D<A, B>::CheckSimplexTetrahedron()
{
	Vector3 a = m_simplexA.Get();
	Vector3 b = m_simplexB.Get();
	Vector3 c = m_simplexC.Get();
	Vector3 d = m_simplexD.Get();

	if (IsSimplexDegenerate())
	{
		// Tetrahedron is collapsed (D was on the plane of ABC), so reduce it to iterate
		m_simplexD.Invalidate();
		m_numVerts--;

		Vector3 triBaryCoords = ComputeBarycentricCoordinates(Vector3::ZERO, Triangle3(a, b, c));

		if (triBaryCoords.u >= 0.f && triBaryCoords.v >= 0.f && triBaryCoords.w >= 0.f)
		{
			// Since D made it degenerate and we're inside this triangle, closest point will be the projection to this face
			Vector3 trianglePt = triBaryCoords.u * a + triBaryCoords.v * b + triBaryCoords.w * c;
			m_separationNormal = (trianglePt - Vector3::ZERO);
			m_separation = m_separationNormal.Normalize();
			return true;
		}

		// Remove the least contributing point
		// Don't remove more than 1 point, as it creates infinite loop issues (cone case)
		float minCoord = Min(triBaryCoords.u, triBaryCoords.v, triBaryCoords.w);

		if (minCoord == triBaryCoords.u)
		{
			m_simplexA.Invalidate();
		}
		else if (minCoord == triBaryCoords.v)
		{
			m_simplexB.Invalidate();
		}
		else if (minCoord == triBaryCoords.w)
		{
			m_simplexC.Invalidate();
		}
	}
	else
	{
		// Tetrahedron is valid - see if it contains the origin
		Tetrahedron tetra(a, b, c, d);

		Vector4 baryCoords = ComputeBarycentricCoordinates(Vector3::ZERO, tetra);

		if (baryCoords.x >= 0.f && baryCoords.y >= 0.f && baryCoords.z >= 0.f && baryCoords.w >= 0.f)
		{
			// Origin is inside tetrahedron == Origin is inside the Minkowski Difference
			// If desired, the user of the solver can perform EPA to get the pen and normal
			m_separation = 0.f;
			return true;
		}

		// Origin is outside tetrahedron, so evolve
		// Only drop one tetrahedron point since there's infinite loop issues related to dropping multiple points (cone issue)
		// So - if there's more than one negative coordinate, choose to drop the one that leaves behind a triangle with a normal that'd be the best search direction for the next iteration
		float bestDot = -1.f;
		int coordinateToRemove = -1;

		if (baryCoords.x < 0.f) // A doesn't contribute
		{
			Vector3 bc = c - b;
			Vector3 bd = d - b;
			Vector3 normal = CrossProduct(bc, bd).GetNormalized();
			float dot = Abs(DotProduct(normal, Vector3::ZERO - b)); // Abs since we don't care which direction, we flip as needed later

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
			float dot = Abs(DotProduct(normal, Vector3::ZERO - a));

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
			float dot = Abs(DotProduct(normal, Vector3::ZERO - a));

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
			float dot = Abs(DotProduct(normal, Vector3::ZERO - a));

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
			m_simplexA.Invalidate();
			break;
		case 1:
			m_simplexB.Invalidate();
			break;
		case 2:
			m_simplexC.Invalidate();
			break;
		case 3:
			m_simplexD.Invalidate();
			break;
		default:
			break;
		}
	}

	m_numVerts--;
	CleanUpSimplexPoints();
	return false;
}


//-------------------------------------------------------------------------------------------------
template <class A, class B>
void GJKSolver3D<A, B>::CleanUpSimplexPoints()
{
	bool done = false;
	while (!done)
	{
		done = true;
		for (int i = 0; i < (4 - 1); ++i)
		{
			if (!m_simplexPts[i].IsValid() && m_simplexPts[i + 1].IsValid())
			{
				m_simplexPts[i] = m_simplexPts[i + 1];
				m_simplexPts[i + 1].Invalidate();

				// Also shift inputs
				m_minkowskiInputs[2 * i] = m_minkowskiInputs[2 * (i + 1)];
				m_minkowskiInputs[2 * i + 1] = m_minkowskiInputs[2 * (i + 1) + 1];

				m_minkowskiInputs[2 * (i + 1)] = Vector3::ZERO;
				m_minkowskiInputs[2 * (i + 1) + 1] = Vector3::ZERO;

				done = false;
			}
		}
	}
}


//-------------------------------------------------------------------------------------------------
template <class A, class B>
bool GJKSolver3D<A, B>::IsSimplexDegenerate() const
{
	bool isDegenerate = false;

	switch (m_numVerts)
	{
	case 2:
		isDegenerate = AreMostlyEqual(m_simplexA.Get(), m_simplexB.Get());
		break;
	case 3: 
		isDegenerate = (AreMostlyEqual(m_simplexC.Get(), m_simplexA.Get()) || AreMostlyEqual(m_simplexC.Get(), m_simplexB.Get()));
		break;
	case 4:
	{
		Tetrahedron tetra(m_simplexA.Get(), m_simplexB.Get(), m_simplexC.Get(), m_simplexD.Get());
		isDegenerate = (AreMostlyEqual(tetra.CalculateUnsignedVolume(), 0.f, 0.001f));
	}
		break;
	default:
		break;
	}

	return isDegenerate;
}


//-------------------------------------------------------------------------------------------------
template <class A, class B>
void GJKSolver3D<A, B>::ExpandSimplex()
{
	switch (m_numVerts)
	{
	case 0:
	{
		Vector3 bToA = m_a.GetCenter() - m_b.GetCenter();
		m_simplexA.Set(GetMinkowskiSupportPoint(bToA));
	}
	break;
	case 1:
	{
		Vector3 aToB = m_b.GetCenter() - m_a.GetCenter();
		m_simplexB.Set(GetMinkowskiSupportPoint(aToB));
	}
	break;
	case 2:
	{
		Vector3 aToB = m_simplexB.Get() - m_simplexA.Get();
		Vector3 aToOrigin = Vector3::ZERO - m_simplexA.Get();
		Vector3 ref = CrossProduct(aToOrigin, aToB);
		Vector3 searchDir = CrossProduct(aToB, ref);

		// Make sure we search towards the origin
		if (DotProduct(aToOrigin, searchDir) < 0.f)
		{
			searchDir *= -1.0f;
		}
		m_simplexC.Set(GetMinkowskiSupportPoint(searchDir));
	}
		break;
	case 3:
	{
		Vector3 ab = m_simplexB.Get() - m_simplexA.Get();
		Vector3 ac = m_simplexC.Get() - m_simplexA.Get();
		Vector3 normal = CrossProduct(ab, ac);

		Vector3 aToOrigin = Vector3::ZERO - m_simplexA.Get();

		// Make sure we search towards the point
		if (DotProduct(aToOrigin, normal) < 0.f)
		{
			normal *= -1.0f;
		}

		m_simplexD.Set(GetMinkowskiSupportPoint(normal));
	}
		break;
	default:
		ERROR_AND_DIE("Expanding too much!");
		break;
	}

	m_numVerts++;
}


//-------------------------------------------------------------------------------------------------
template <class A, class B>
Vector3 GJKSolver3D<A, B>::GetMinkowskiSupportPoint(const Vector3& direction)
{
	Vector3 aSupport, bSupport;

	m_a.GetSupportPoint(direction, aSupport);
	m_b.GetSupportPoint(-1.0f * direction, bSupport);

	m_minkowskiInputs[2 * m_numVerts] = aSupport;
	m_minkowskiInputs[2 * m_numVerts + 1] = bSupport;

	return aSupport - bSupport;
}


//-------------------------------------------------------------------------------------------------
template <class A, class B>
void GJKSolver3D<A, B>::ComputeClosestPoints()
{
	switch (m_numVerts)
	{
	case 1:
		m_closestPtA = m_minkowskiInputs[0];
		m_closestPtB = m_minkowskiInputs[1];
		break;
	case 2:
	{
		LineSegment3 simplexEdge(m_simplexA.Get(), m_simplexB.Get());
		Vector2 baryCoords = ComputeBarycentricCoordinates(Vector3::ZERO, simplexEdge);

		if (baryCoords.u < 0.f)
		{
			m_closestPtA = m_minkowskiInputs[2];
			m_closestPtB = m_minkowskiInputs[3];
		}
		else if (baryCoords.v < 0.f)
		{
			m_closestPtA = m_minkowskiInputs[0];
			m_closestPtB = m_minkowskiInputs[1];
		}
		else
		{
			m_closestPtA = m_minkowskiInputs[0] * baryCoords.u + m_minkowskiInputs[2] * baryCoords.v;
			m_closestPtB = m_minkowskiInputs[1] * baryCoords.u + m_minkowskiInputs[3] * baryCoords.v;
		}
	}
		break;
	case 3:
	{
		Triangle3 simplexFace(m_simplexA.Get(), m_simplexB.Get(), m_simplexC.Get());
		Vector3 baryCoords = ComputeBarycentricCoordinates(Vector3::ZERO, simplexFace);
		m_closestPtA = m_minkowskiInputs[0] * baryCoords.u + m_minkowskiInputs[2] * baryCoords.v + m_minkowskiInputs[4] * baryCoords.w;
		m_closestPtB = m_minkowskiInputs[1] * baryCoords.u + m_minkowskiInputs[3] * baryCoords.v + m_minkowskiInputs[5] * baryCoords.w;
	}
		break;
	default:
		break;
	}

	/*Vector3 a = m_simplexA.Get();
	Vector3 b = m_simplexB.Get();
	Vector3 c = m_simplexC.Get();
	Vector3 d = m_simplexD.Get();

	Matrix3 mat;
	Vector3 vecInput = Vector3(1.f, 0.f, 0.f);

	mat.Ix = 1.f;
	mat.Iy = DotProduct(b - a, a);
	mat.Iz = DotProduct(c - a, a);

	mat.Jx = 1.f;
	mat.Jy = DotProduct(b - a, b);
	mat.Jz = DotProduct(c - a, b);

	mat.Kx = 1.f;
	mat.Ky = DotProduct(b - a, c);
	mat.Kz = DotProduct(c - a, c);

	float det = mat.GetDeterminant();
	Vector3 lambas;

	for (int i = 0; i < m_numVerts; ++i)
	{
		Matrix3 mati = mat;
		mati.columnVectors[i] = vecInput;

		float deti = mati.GetDeterminant();
		lambas.data[i] = deti / det;
	}

	for (int i = 0; i < m_numVerts; ++i)
	{
		m_closestPtA += lambas.data[i] * m_minkowskiInputs[2 * i];
		m_closestPtB += lambas.data[i] * m_minkowskiInputs[2 * i + 1];
	}*/
}
