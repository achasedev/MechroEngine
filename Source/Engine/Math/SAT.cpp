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
			// Keep axis pointing at A for consistency
			if (DotProduct(polyhedron.GetCenter() - capsule.GetCenter(), out_axis) > 0.f)
			{
				out_axis *= -1.0f;
			}

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

	return true;
}
