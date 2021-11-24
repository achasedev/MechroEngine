///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: Nov 23rd, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/Math/Capsule3.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/Polyhedron.h"
#include "Engine/Math/SAT.h"

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
template <class A, class B>
static float ComputePenetrationOnAxis(const A& a, const B& b, const Vector3 &axis)
{
	// Project the half-size of one onto axis
	Vector2 aProj = ComputeAxisProjection(a, axis);
	Vector2 bProj = ComputeAxisProjection(b, axis);

	// x is min, y is max
	float pen1 = aProj.y - bProj.x;
	float pen2 = bProj.y - aProj.x;

	return Min(pen1, pen2);
}


//-------------------------------------------------------------------------------------------------
template <class A>
static inline Vector2 ComputeAxisProjection(const A& shape, const Vector3 &axis)
{
	Vector3 supportPtA, supportPtB;
	shape.GetSupportPoint(axis, supportPtA);
	shape.GetSupportPoint(-1.0f * axis, supportPtB);

	float dot1 = DotProduct(supportPtA, axis);
	float dot2 = DotProduct(supportPtB, axis);

	return Vector2(Min(dot1, dot2), Max(dot1, dot2));
}

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
bool SAT::GetMinPenAxis(const Capsule3& capsule, const Polyhedron& polyhedron, SATResult_CapsuleHull& out_result)
{
	out_result.m_pen = FLT_MAX;
	out_result.m_axis = Vector3::ZERO;
	out_result.m_iFaceOrEdge = -1;
	out_result.m_isFaceAxis = false;

	// Compute pens on face normals
	int numFaces = polyhedron.GetNumFaces();
	for (int iFace = 0; iFace < numFaces; ++iFace)
	{
		Vector3 normal = polyhedron.GetFaceNormal(iFace);

		// If this face has a normal colinear but opposite one we've already seen, choose it if
		// the capsule is penning this face less
		if (AreMostlyEqual(normal, -1.0f * out_result.m_axis))
		{
			Plane3 currPlane = polyhedron.GetFaceSupportPlane(iFace);
			Plane3 minPlane = polyhedron.GetFaceSupportPlane(out_result.m_iFaceOrEdge);

			Vector3 currFurthest, minFurthest;
			capsule.GetSupportPoint(-1.0f * normal, currFurthest);
			capsule.GetSupportPoint(-1.0f * out_result.m_axis, minFurthest);

			if (Abs(currPlane.GetDistanceFromPlane(currFurthest)) < Abs(minPlane.GetDistanceFromPlane(minFurthest)))
			{
				out_result.m_axis = normal;
				out_result.m_iFaceOrEdge = iFace;
			}
		}
		else
		{
			float pen = ComputePenetrationOnAxis(capsule, polyhedron, normal);

			if (pen < 0.f)
				return false;

			if (pen < out_result.m_pen)
			{
				out_result.m_pen = pen;
				out_result.m_axis = normal;
				out_result.m_isFaceAxis = true;
				out_result.m_iFaceOrEdge = iFace;
			}
		}
	}

	// No separating axis exists on face normals
	// Now compute for spine/edge axes
	UniqueHalfEdgeIterator edgeIter(polyhedron);
	const HalfEdge* edge = edgeIter.GetNext();
	Vector3 spineDir = (capsule.end - capsule.start).GetNormalized();
	
	while (edge != nullptr)
	{
		Vector3 edgeDir = polyhedron.GetEdgeDirectionNormalized(edge->m_edgeIndex);
		Vector3 axis = CrossProduct(spineDir, edgeDir);

		// Don't check axes created from colinear inputs
		if (!AreMostlyEqual(axis.GetLengthSquared(), 0.f))
		{
			// Keep axis pointing at A for consistency
			if (DotProduct(polyhedron.GetCenter() - capsule.GetCenter(), axis) > 0.f)
			{
				axis *= -1.0f;
			}

			axis.Normalize();
			float pen = ComputePenetrationOnAxis(capsule, polyhedron, axis);

			if (pen < 0.f)
				return false;

			if (pen < out_result.m_pen)
			{
				out_result.m_pen = pen;
				out_result.m_axis = axis;
				out_result.m_iFaceOrEdge = edge->m_edgeIndex;
				out_result.m_isFaceAxis = false;
			}
		}

		edge = edgeIter.GetNext();
	}

	return true;
}
