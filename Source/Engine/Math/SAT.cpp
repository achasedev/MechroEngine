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
static float ComputeAxisOverlap(const A& a, const B& b, const Vector3 &axis)
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
		Plane3 facePlane = polyhedron.GetFaceSupportPlane(iFace);
		Vector3 capPt;
		capsule.GetSupportPoint(-1.0f * facePlane.GetNormal(), capPt);

		float pen = -1.0f * facePlane.GetDistanceFromPlane(capPt);

		if (pen < 0.f)
			return false;

		if (pen < out_result.m_pen)
		{
			out_result.m_pen = pen;
			out_result.m_axis = facePlane.GetNormal();
			out_result.m_isFaceAxis = true;
			out_result.m_iFaceOrEdge = iFace;
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
			axis.Normalize();
			float pen = ComputeAxisOverlap(capsule, polyhedron, axis);

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


//-------------------------------------------------------------------------------------------------
static void QueryFaceDirections(const Polyhedron& faceHull, const Polyhedron& pointHull, bool faceHullIsA, SATResult_HullHull& out_result)
{
	int numFaces = faceHull.GetNumFaces();
	for (int iFace = 0; iFace < numFaces; ++iFace)
	{
		Plane3 facePlane = faceHull.GetFaceSupportPlane(iFace);
		Vector3 pt;
		pointHull.GetSupportPoint(-1.0f * facePlane.m_normal, pt);

		float pen = -1.0f * facePlane.GetDistanceFromPlane(pt);

		if (pen < out_result.m_pen)
		{
			out_result.m_pen = pen;
			out_result.m_axis = facePlane.m_normal;

			if (faceHullIsA)
			{
				out_result.m_iFaceOrEdgeA = iFace;
				out_result.m_iFaceOrEdgeB = -1;
			}
			else
			{
				out_result.m_iFaceOrEdgeB = iFace;
				out_result.m_iFaceOrEdgeA = -1;
			}

			out_result.m_isFaceAxis = true;

			// This axis is a separating axis so just signal to stop
			if (pen < 0.f)
				return;
		}
	}
}


//-------------------------------------------------------------------------------------------------
static void QueryEdgeDirections(const Polyhedron& a, const Polyhedron& b, SATResult_HullHull& out_result)
{
	UniqueHalfEdgeIterator aEdgeIter(a);
	UniqueHalfEdgeIterator bEdgeIter(b);

	const HalfEdge* aEdge = aEdgeIter.GetNext();
	const HalfEdge* bEdge = bEdgeIter.GetNext();

	while (aEdge != nullptr)
	{
		Vector3 aEdgeDir = a.GetEdgeDirection(aEdge);
		Vector3 aEdgePt = a.GetVertexPosition(aEdge->m_vertexIndex);

		while (bEdge != nullptr)
		{
			Vector3 bEdgeDir = b.GetEdgeDirection(bEdge);

			Vector3 axis = CrossProduct(aEdgeDir, bEdgeDir);

			if (AreMostlyEqual(axis, Vector3::ZERO))
			{
				bEdge = bEdgeIter.GetNext();
				continue;
			}

			axis.Normalize();

			// Ensure the axis points towards a
			Vector3 outOfA = a.GetVertexPosition(aEdge->m_vertexIndex) - a.GetCenter();
			if (DotProduct(axis, outOfA) > 0.f)
			{
				axis *= -1.0f;
			}

			// Since normal points into A, put the plane on B
			Vector3 bEdgePt = b.GetVertexPosition(bEdge->m_vertexIndex);
			Plane3 plane(axis, bEdgePt);

			// If this plane bisects B, don't even consider it
			Vector3 bSupportPt;
			b.GetSupportPoint(axis, bSupportPt);
			if (plane.GetDistanceFromPlane(bSupportPt) > 0.f)
			{
				return;
			}

			// Get the furthest a point behind the plane 
			Vector3 aSupportPt;
			a.GetSupportPoint(-1.0f * axis, aSupportPt);

			{
				Plane3 aPlane(-1.0f * axis, aEdgePt);
				if (aPlane.GetDistanceFromPlane(aSupportPt) > 0.f)
					return;
			}

			float pen = -1.0f * plane.GetDistanceFromPlane(aSupportPt);

			if (pen < out_result.m_pen)
			{
				out_result.m_pen = pen;
				out_result.m_axis = axis;
				out_result.m_isFaceAxis = false;
				out_result.m_iFaceOrEdgeA = aEdge->m_edgeIndex;
				out_result.m_iFaceOrEdgeB = bEdge->m_edgeIndex;

				// Early out if a separating axis was found
				if (pen < 0.f)
					return;
			}

			bEdge = bEdgeIter.GetNext();
		}

		aEdge = aEdgeIter.GetNext();
	}
}


//-------------------------------------------------------------------------------------------------
bool SAT::GetMinPenAxis(const Polyhedron& a, const Polyhedron& b, SATResult_HullHull& out_result)
{
	out_result = SATResult_HullHull();

	QueryFaceDirections(a, b, true, out_result);
	if (out_result.m_pen < 0.f)
		return false;

	QueryFaceDirections(b, a, false, out_result);
	if (out_result.m_pen < 0.f)
		return false;

	QueryEdgeDirections(a, b, out_result);

	if (out_result.m_isFaceAxis)
	{
		ASSERT_OR_DIE(out_result.m_iFaceOrEdgeA == -1 && out_result.m_iFaceOrEdgeB != -1 || out_result.m_iFaceOrEdgeA != -1 && out_result.m_iFaceOrEdgeB == -1, "Both face indices set!");
	}

	return (out_result.m_pen > 0.f);
}
