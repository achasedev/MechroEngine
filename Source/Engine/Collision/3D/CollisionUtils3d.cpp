///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 20th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Collision/3D/CollisionUtils3d.h"
#include "Engine/Collision/3D/ContactManifold3d.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/Polygon3d.h"
#include "Engine/Render/Debug/DebugRenderSystem.h"

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
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
BroadphaseResult3d CollisionUtils3d::Collide(SphereCollider3d* colA, SphereCollider3d* colB)
{
	Sphere3d sphereA = colA->GetWorldShape();
	Sphere3d sphereB = colB->GetWorldShape();

	Vector3 aToB = sphereB.m_center - sphereA.m_center;
	float distanceSquared = aToB.GetLengthSquared();
	float radiusSquared = (sphereA.m_radius + sphereB.m_radius) * (sphereA.m_radius + sphereB.m_radius);

	BroadphaseResult3d result;

	// Degenerate case - sphere centers are on top of eachother
	if (AreMostlyEqual(distanceSquared, 0.f))
	{
		// Choose an arbitrary normal
		result.m_collisionFound = true;
		result.m_direction = Vector3::Y_AXIS;
		result.m_penetration = sphereA.m_radius + sphereB.m_radius;
	}
	else if (distanceSquared < radiusSquared)
	{
		float distance = aToB.Normalize();

		result.m_collisionFound = true;
		result.m_direction = aToB;
		result.m_penetration = distance - sphereA.m_radius - sphereB.m_radius;
	}

	return result;
}


//-------------------------------------------------------------------------------------------------
int CollisionUtils3d::CalculateContacts(PolytopeCollider3d* colA, CapsuleCollider3d* colB, const BroadphaseResult3d& broadResult, ContactPoint3D* out_contacts)
{
	UNIMPLEMENTED();
	UNUSED(colA);
	UNUSED(colB);
	UNUSED(broadResult);
	UNUSED(out_contacts);
}


//-------------------------------------------------------------------------------------------------
int CollisionUtils3d::CalculateContacts(CapsuleCollider3d* colA, PolytopeCollider3d* colB, const BroadphaseResult3d& broadResult, ContactPoint3D* out_contacts)
{
	UNIMPLEMENTED();
	UNUSED(colA);
	UNUSED(colB);
	UNUSED(broadResult);
	UNUSED(out_contacts);
}


//-------------------------------------------------------------------------------------------------
int CollisionUtils3d::CalculateContacts(PolytopeCollider3d* colA, SphereCollider3d* colB, const BroadphaseResult3d& broadResult, ContactPoint3D* out_contacts)
{
	UNIMPLEMENTED();
	UNUSED(colA);
	UNUSED(colB);
	UNUSED(broadResult);
	UNUSED(out_contacts);
}


//-------------------------------------------------------------------------------------------------
int CollisionUtils3d::CalculateContacts(SphereCollider3d* colA, PolytopeCollider3d* colB, const BroadphaseResult3d& broadResult, ContactPoint3D* out_contacts)
{
	UNIMPLEMENTED();
	UNUSED(colA);
	UNUSED(colB);
	UNUSED(broadResult);
	UNUSED(out_contacts);
}


//-------------------------------------------------------------------------------------------------
int CollisionUtils3d::CalculateContacts(CapsuleCollider3d* colA, SphereCollider3d* colB, const BroadphaseResult3d& broadResult, ContactPoint3D* out_contacts)
{
	UNIMPLEMENTED();
	UNUSED(colA);
	UNUSED(colB);
	UNUSED(broadResult);
	UNUSED(out_contacts);
}


//-------------------------------------------------------------------------------------------------
int CollisionUtils3d::CalculateContacts(SphereCollider3d* colA, CapsuleCollider3d* colB, const BroadphaseResult3d& broadResult, ContactPoint3D* out_contacts)
{
	UNIMPLEMENTED();
	UNUSED(colA);
	UNUSED(colB);
	UNUSED(broadResult);
	UNUSED(out_contacts);
}


//-------------------------------------------------------------------------------------------------
int CollisionUtils3d::CalculateContacts(PolytopeCollider3d* colA, PolytopeCollider3d* colB, const BroadphaseResult3d& broadResult, ContactPoint3D* out_contacts)
{
	const PolytopeCollider3d* refCol = broadResult.m_refCol->GetAsType<PolytopeCollider3d>();
	const PolytopeCollider3d* incCol = broadResult.m_incCol->GetAsType<PolytopeCollider3d>();

	const Polygon3d* refShape = refCol->GetWorldShape();
	const Polygon3d* incShape = incCol->GetWorldShape();

	std::vector<Vector3> finalPositions;
	std::vector<float> finalPens;

	if (broadResult.m_isFaceCollision)
	{
		// Get all the side planes to the reference face
		// These are all planes that
		// - Positioned on each edge of the reference face
		// - Perpendicular to the reference face plane
		// - Point outward
		std::vector<Plane3> refSidePlanes;
		refShape->GetAllSidePlanesForFace(broadResult.m_refFaceIndex, refSidePlanes);

		// Choose the incident face
		// The incident face is the face who's normal is mostly against the reference face's normal
		int incFaceIndex = incShape->GetIndexOfFaceMostInDirection(-1.0f * broadResult.m_direction);

		// Get all the vertices in the incident face to be clipped
		incShape->GetAllVerticesInFace(incFaceIndex, finalPositions);

		// Clip the incident vertices against the reference side planes using Sutherland Hodgman clipping
		std::vector<Vector3> input; // Need a staging vector for each iteration, as we can't change points in the middle of an iteration

		// Iterate over all the side planes that will clip the incident face
		int numSidePlanes = (int) refSidePlanes.size();
		for (int sidePlaneIndex = 0; sidePlaneIndex < numSidePlanes; ++sidePlaneIndex)
		{
			Plane3& plane = refSidePlanes[sidePlaneIndex];

			input = finalPositions;
			finalPositions.clear();

			// For each "edge starting" incident face vertex, find out how to push the end vertex in...
			for (int posIndex = 0; posIndex < (int)input.size(); ++posIndex)
			{
				// Get the edge as points
				Vector3 start = input[posIndex];
				Vector3 end = input[(posIndex + 1) % input.size()];

				// Check the end points for this plane
				bool startInside = plane.IsPointBehind(start);
				bool endInside = plane.IsPointBehind(end);

				if (startInside)
				{
					if (endInside)
					{
						// Edge is totally inside this plane, so just push the end point as is
						finalPositions.push_back(end);
					}
					else
					{
						// Starts inside, ends outside, so clip the end point along the edge until it meets the plane
						Line3 line(start, end - start);

						Vector3 intersection = SolveLinePlaneIntersection(line, plane);
						finalPositions.push_back(intersection);
					}
				}
				else if (endInside)
				{
					// Starts outside, but ends inside, so we clip by moving the end point back along the edge to the plane...
					Line3 line(start, end - start);

					Vector3 intersection = SolveLinePlaneIntersection(line, plane);
					finalPositions.push_back(intersection);

					// ...and also add the end itself, since it's inside and needs to be kept
					finalPositions.push_back(end);
				}
			}
		}

		// Now remove all points in front of the reference face, and those behind the face move onto the face
		Plane3 refPlane = refShape->GetFaceSupportPlane(broadResult.m_refFaceIndex);
		finalPens.resize(finalPositions.size());
		for (int clipIndex = (int)finalPositions.size() - 1; clipIndex >= 0; --clipIndex)
		{
			float pen = refPlane.GetDistanceFromPlane(finalPositions[clipIndex]);
			if (pen > 0.f)
			{
				finalPositions.erase(finalPositions.begin() + clipIndex);
				finalPens.erase(finalPens.begin() + clipIndex);
			}
			else
			{
				finalPositions[clipIndex] = refPlane.GetProjectedPointOntoPlane(finalPositions[clipIndex]);
				finalPens[clipIndex] = pen;
			}
		}
	}
	else
	{
		Vector3 refEdgeStart, refEdgeEnd, incEdgeStart, incEdgeEnd;
		refShape->GetEdgeEndPoints(broadResult.m_refEdgeIndex, refEdgeStart, refEdgeEnd);
		incShape->GetEdgeEndPoints(broadResult.m_incEdgeIndex, incEdgeStart, incEdgeEnd);

		// Edge contact collision
		Vector3 refEdgePoint;
		Vector3 incEdgePoint;
		float separation = FindClosestPointsOnLineSegments(refEdgeStart, refEdgeEnd, incEdgeStart, incEdgeEnd, refEdgePoint, incEdgePoint);

		// Choose the contact point to be in the middle of these two points
		finalPositions.push_back(-0.5f * (refEdgePoint + incEdgePoint));
		finalPens.push_back(separation);
	}

	ASSERT_OR_DIE(finalPositions.size() <= ContactManifold3d::MAX_CONTACTS, "Too many contacts!");
	ASSERT_OR_DIE(finalPositions.size() == finalPens.size(), "Position and pen mismatch!");

	for (int contactIndex = 0; contactIndex < (int)finalPositions.size(); ++contactIndex)
	{
		out_contacts[contactIndex].m_position = finalPositions[contactIndex];
		out_contacts[contactIndex].m_normal = broadResult.m_direction;
		out_contacts[contactIndex].m_pen = finalPens[contactIndex];

		DebugDrawPoint3D(out_contacts[contactIndex].m_position, Rgba::MAGENTA, 0.f);
	}

	return (int)finalPositions.size();
}


//-------------------------------------------------------------------------------------------------
int CollisionUtils3d::CalculateContacts(CapsuleCollider3d* colA, CapsuleCollider3d* colB, const BroadphaseResult3d& broadResult, ContactPoint3D* out_contacts)
{
	UNIMPLEMENTED();
	UNUSED(colA);
	UNUSED(colB);
	UNUSED(broadResult);
	UNUSED(out_contacts);
}


//-------------------------------------------------------------------------------------------------
int CollisionUtils3d::CalculateContacts(SphereCollider3d* colA, SphereCollider3d* colB, const BroadphaseResult3d& broadResult, ContactPoint3D* out_contacts)
{
	const Sphere3d shapeA = colA->GetWorldShape();
	const Sphere3d shapeB = colB->GetWorldShape();

	// Get the two surface points along the normal
	const Vector3 surfaceA = shapeA.m_center + shapeA.m_radius * broadResult.m_direction;
	const Vector3 surfaceB = shapeB.m_center + shapeB.m_radius * -1.0f * broadResult.m_direction;
	const Vector3 contactPos = 0.5f * (surfaceA + surfaceB);

	out_contacts[0].m_position = contactPos;
	out_contacts[0].m_normal = broadResult.m_direction;
	return 1;
}


//-------------------------------------------------------------------------------------------------
BroadphaseResult3d CollisionUtils3d::Collide(PolytopeCollider3d* colA, CapsuleCollider3d* colB)
{
	UNIMPLEMENTED();
	UNUSED(colA);
	UNUSED(colB);
}


//-------------------------------------------------------------------------------------------------
BroadphaseResult3d CollisionUtils3d::Collide(CapsuleCollider3d* colA, PolytopeCollider3d* colB)
{
	UNIMPLEMENTED();
	UNUSED(colA);
	UNUSED(colB);
}


//-------------------------------------------------------------------------------------------------
BroadphaseResult3d CollisionUtils3d::Collide(PolytopeCollider3d* colA, SphereCollider3d* colB)
{
	UNIMPLEMENTED();
	UNUSED(colA);
	UNUSED(colB);
}


//-------------------------------------------------------------------------------------------------
BroadphaseResult3d CollisionUtils3d::Collide(SphereCollider3d* colA, PolytopeCollider3d* colB)
{
	UNIMPLEMENTED();
	UNUSED(colA);
	UNUSED(colB);
}


//-------------------------------------------------------------------------------------------------
BroadphaseResult3d CollisionUtils3d::Collide(CapsuleCollider3d* colA, SphereCollider3d* colB)
{
	UNIMPLEMENTED();
	UNUSED(colA);
	UNUSED(colB);
}


//-------------------------------------------------------------------------------------------------
BroadphaseResult3d CollisionUtils3d::Collide(SphereCollider3d* colA, CapsuleCollider3d* colB)
{
	UNIMPLEMENTED();
	UNUSED(colA);
	UNUSED(colB);
}


//-------------------------------------------------------------------------------------------------
// Finds the max value b penetrates onto each of a's normals, and returns the max of all projections
// A value greater than zero implies there's a gap -> no overlap
float SolvePartialSAT(const Polygon3d* a, const Polygon3d* b, int& out_faceIndex)
{
	int numFaces = a->GetNumFaces();
	float maxDistance = 0.f;

	for (int faceIndex = 0; faceIndex < numFaces; ++faceIndex)
	{
		Plane3 plane = a->GetFaceSupportPlane(faceIndex);

		Vector3 supportDir = -1.0f * plane.GetNormal();
		Vector3 supportPoint;
		b->GetSupportPoint(supportDir, supportPoint);

		float distance = plane.GetDistanceFromPlane(supportPoint);
		if (faceIndex == 0 || distance > maxDistance)
		{
			maxDistance = distance;
			out_faceIndex = faceIndex;
		}
	}

	return maxDistance;
}

//-------------------------------------------------------------------------------------------------
float SolveEdgeSAT(const Polygon3d* a, const Polygon3d* b, Vector3& out_direction, int& out_indexA, int& out_indexB)
{
	UniqueHalfEdgeIterator edgeAIter(*a);
	const HalfEdge* edgeA = edgeAIter.GetNext();

	float maxDistance = 0.f;
	bool firstIteration = true;

	while (edgeA != nullptr)
	{
		Vector3 directionA = a->GetEdgeDirection(edgeA);
		Vector3 outwardDirA = a->GetVertexPosition(edgeA->m_vertexIndex) - a->GetCenter();

		UniqueHalfEdgeIterator edgeBIter(*b);
		const HalfEdge* edgeB = edgeBIter.GetNext();

		while (edgeB != nullptr)
		{
			Vector3 aStart = a->GetVertexPosition(edgeA->m_vertexIndex);
			Vector3 aEnd = a->GetVertexPosition(a->GetEdge(edgeA->m_nextEdgeIndex)->m_vertexIndex);
			Vector3 bStart = b->GetVertexPosition(edgeB->m_vertexIndex);
			Vector3 bEnd = b->GetVertexPosition(b->GetEdge(edgeB->m_nextEdgeIndex)->m_vertexIndex);

			Vector3 aDir = aEnd - aStart;
			Vector3 bDir = bEnd - bStart;

			Vector3 directionB = b->GetEdgeDirection(edgeB);

			Vector3 normal = CrossProduct(directionA, directionB);

			if (AreMostlyEqual(normal.GetLengthSquared(), 0.f))
			{
				edgeB = edgeBIter.GetNext();
				continue;
			}
			else
			{
				normal.Normalize();
			}

			// Ensure the normal points away from A
			if (AreMostlyEqual(DotProduct(normal, outwardDirA), 0.f))
			{
				edgeB = edgeBIter.GetNext();
				continue;
			}
			else if (DotProduct(normal, outwardDirA) < 0.f)
			{
				normal *= -1.0f;
			}

			// Make a plane on the edge of A facing outward from A
			Plane3 plane(normal, a->GetVertexPosition(edgeA->m_vertexIndex));

			Vector3 supportA;
			a->GetSupportPoint(normal, supportA);
			float distanceA = plane.GetDistanceFromPlane(supportA);

			// If the plane is in the middle of A, just skip it
			if (distanceA > 0.f)
			{
				edgeB = edgeBIter.GetNext();
				continue;
			}	

			// Get the vertex in B that would be furthest against the normal (if anything will be behind this plane, it would be that point)
			Vector3 supportB;
			b->GetSupportPoint(-1.0f * normal, supportB);

			// If the plane is in the middle of B, just skip it
			Plane3 planeInB(normal, b->GetVertexPosition(edgeB->m_vertexIndex));
			float distanceWithinB = planeInB.GetDistanceFromPlane(supportB);

			if (!AreMostlyEqual(distanceWithinB, 0.f))
			{
				edgeB = edgeBIter.GetNext();
				continue;
			}

			float distance = plane.GetDistanceFromPlane(supportB);

			if (firstIteration || distance > maxDistance)
			{
				maxDistance = distance;
				out_direction = normal;
				out_indexA = edgeA->m_edgeIndex;
				out_indexB = edgeB->m_edgeIndex;
				firstIteration = false;
			}

			edgeB = edgeBIter.GetNext();
		}

		edgeA = edgeAIter.GetNext();
	}

	return maxDistance;
}


//-------------------------------------------------------------------------------------------------
BroadphaseResult3d CollisionUtils3d::Collide(PolytopeCollider3d* colA, PolytopeCollider3d* colB)
{
	const Polygon3d* worldShapeA = colA->GetWorldShape();
	const Polygon3d* worldShapeB = colB->GetWorldShape();

	int bestFaceIndexA = -1;
	int bestFaceIndexB = -1;

	float aOntoBDistance = SolvePartialSAT(worldShapeA, worldShapeB, bestFaceIndexA);

	if (aOntoBDistance > 0.f)
		return BroadphaseResult3d(false);

	float bOntoADistance = SolvePartialSAT(worldShapeB, worldShapeA, bestFaceIndexB);

	if (bOntoADistance > 0.f)
		return BroadphaseResult3d(false);

	Vector3 edgeDir;
	int edgeIndexA;
	int edgeIndexB;
	float edgeDistance = SolveEdgeSAT(worldShapeA, worldShapeB, edgeDir, edgeIndexA, edgeIndexB);

	if (edgeDistance > 0.f)
		return BroadphaseResult3d(false);

	// No gaps exist on face normals or edges, so the min distance will be our min pen
	BroadphaseResult3d result(true);
	float penOnFaceA = Abs(aOntoBDistance);
	float penOnFaceB = Abs(bOntoADistance);
	float penOnEdge = Abs(edgeDistance);
	result.m_penetration = Min(penOnFaceA, penOnFaceB, penOnEdge);

	if (result.m_penetration == penOnFaceA)
	{
		result.m_direction = worldShapeA->GetFaceNormal(bestFaceIndexA);
		result.m_isFaceCollision = true;
		result.m_refFaceIndex = bestFaceIndexA;
		result.m_refCol = colA;
		result.m_incCol = colB;
	}
	else if (result.m_penetration == penOnFaceB)
	{
		result.m_direction = worldShapeB->GetFaceNormal(bestFaceIndexB);
		result.m_isFaceCollision = true;
		result.m_refFaceIndex = bestFaceIndexB;
		result.m_refCol = colB;
		result.m_incCol = colA;
	}
	else
	{
		Vector3 as, ae, bs, be;
		worldShapeA->GetEdgeEndPoints(edgeIndexA, as, ae);
		worldShapeB->GetEdgeEndPoints(edgeIndexB, bs, be);

		result.m_direction = edgeDir;
		result.m_isFaceCollision = false;
		result.m_refCol = colA; // Since we put the normal created from Cross(edgeA, edgeB) on edge A
		result.m_incCol = colB;
		result.m_refEdgeIndex = edgeIndexA;
		result.m_incEdgeIndex = edgeIndexB;
	}
	
	ASSERT_OR_DIE(AreMostlyEqual(result.m_direction.GetLength(), 1.0f), "Direction not unit!");
	return result;
}

//-------------------------------------------------------------------------------------------------
BroadphaseResult3d CollisionUtils3d::Collide(CapsuleCollider3d* colA, CapsuleCollider3d* colB)
{
	UNIMPLEMENTED();
	UNUSED(colA);
	UNUSED(colB);
}


//-------------------------------------------------------------------------------------------------
// Separating axis theorem!
//BroadphaseResult3d CollisionUtils3d::Collide(BoxCollider3d* colA, BoxCollider3d* colB)
//{
//	OBB3 shapeA = colA->GetWorldShape();
//	OBB3 shapeB = colB->GetWorldShape();
//
//	Vector3 pointsA[8];
//	Vector3 pointsB[8];
//	shapeA.GetPoints(pointsA);
//	shapeB.GetPoints(pointsB);
//
//	Vector3 axes[6];
//	axes[0] = shapeA.GetRightVector().GetNormalized();
//	axes[1] = shapeA.GetUpVector().GetNormalized();
//	axes[2] = shapeA.GetForwardVector().GetNormalized();
//	axes[3] = shapeB.GetRightVector().GetNormalized();
//	axes[4] = shapeB.GetUpVector().GetNormalized();
//	axes[5] = shapeB.GetForwardVector().GetNormalized();
//
//	BroadphaseResult3d result;
//
//	for (int axisIndex = 0; axisIndex < 6; ++axisIndex)
//	{
//		const Vector3& axis = axes[axisIndex];
//
//		Range projRangeA;
//		Range projRangeB;
//
//		for (int pointIndex = 0; pointIndex < 8; ++pointIndex)
//		{
//			const Vector3& pointA = pointsA[pointIndex];
//			const Vector3& pointB = pointsB[pointIndex];
//
//			float dotA = DotProduct(pointA, axis);
//			float dotB = DotProduct(pointB, axis);
//
//			if (pointIndex == 0)
//			{
//				projRangeA.min = dotA;
//				projRangeA.max = dotA;
//				projRangeB.min = dotB;
//				projRangeB.max = dotB;
//			}
//			else
//			{
//				projRangeA.min = Min(projRangeA.min, dotA);
//				projRangeA.max = Max(projRangeA.max, dotA);
//				projRangeB.min = Min(projRangeB.min, dotB);
//				projRangeB.max = Max(projRangeB.max, dotB);
//			}
//		}
//
//		if (!DoRangesOverlap(projRangeA, projRangeB))
//		{
//			return BroadphaseResult3d(false);
//		}
//
//		float pen1 = projRangeB.max - projRangeA.min;
//		float pen2 = projRangeA.max - projRangeB.min;
//		float minPen = Min(pen1, pen2);
//
//		if (minPen < result.m_penetration)
//		{
//			result.m_penetration = minPen;
//			result.m_direction = axis;
//
//			// Ensure we point all normals from A to B
//			Vector3 aToB = shapeB.center - shapeA.center;
//			if (DotProduct(aToB, result.m_direction) < 0.f)
//			{
//				result.m_direction *= -1.0f;
//			}
//		}
//	}
//
//	result.m_collisionFound = true;
//	return result;
//}

