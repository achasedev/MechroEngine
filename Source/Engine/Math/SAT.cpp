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
	Vector3 aCenter = a.GetCenter();
	Vector3 bCenter = b.GetCenter();
	Vector3 aToB = bCenter - aCenter;

	// Project the half-size of one onto axis
	float aProj = ComputeAxisProjection(a, axis);
	float bProj = ComputeAxisProjection(b, axis);

	// Project this onto the axis
	float aToBProj = Abs(DotProduct(aToB, axis));

	// Return the overlap (i.e. positive indicates
	// overlap, negative indicates separation).
	return aProj + bProj - aToBProj;
}


//-------------------------------------------------------------------------------------------------
template <class A>
static inline float ComputeAxisProjection(const A& shape, const Vector3 &axis)
{
	Vector3 supportPt;
	shape.GetSupportPoint(axis, supportPt);
	Vector3 centerPt = shape.GetCenter();

	return Abs(DotProduct(supportPt - centerPt, axis));
}

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
bool SAT::GetMinPenAxis(const Capsule3& capsule, const Polyhedron& polyhedron, Vector3& out_axis, float& out_minPen)
{
	out_minPen = FLT_MAX;
	out_axis = Vector3::ZERO;

	// Assemble the list of axes to check
	std::vector<Vector3> axes;

	// Push face normals
	int numFaces = polyhedron.GetNumFaces();
	for (int iFace = 0; iFace < numFaces; ++iFace)
	{
		Vector3 normal = polyhedron.GetFaceNormal(iFace);
		axes.push_back(normal);
	}

	// Push spine/edge cross product normals
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
			axes.push_back(axis);
		}

		edge = edgeIter.GetNext();
	}

	int numAxes = (int)axes.size();
	for (int iAxis = 0; iAxis < numAxes; ++iAxis)
	{
		float pen = ComputePenetrationOnAxis(capsule, polyhedron, axes[iAxis]);

		if (pen < 0.f)
			return false;

		if (pen < out_minPen)
		{
			out_minPen = pen;
			out_axis = axes[iAxis];
		}
	}

	// Keep axis pointing at A for consistency
	if (DotProduct(polyhedron.GetCenter() - capsule.GetCenter(), out_axis) > 0.f)
	{
		out_axis *= -1.0f;
	}

	return true;
}
