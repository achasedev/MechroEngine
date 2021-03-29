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
	UNIMPLEMENTED();
	UNUSED(colA);
	UNUSED(colB);
	UNUSED(broadResult);
	UNUSED(out_contacts);
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

	out_contacts[0] = ContactPoint3D(contactPos, broadResult.m_direction);
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
float SolveEdgeSAT(const Polygon3d* a, const Polygon3d* b, Plane3& out_bestPlane)
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

			if (distanceA > 0.f)
			{
				edgeB = edgeBIter.GetNext();
				continue;
			}

			// Get the vertex in B that would be furthest against the normal (if anything will be behind this plane, it would be that point)
			Vector3 supportB;
			b->GetSupportPoint(-1.0f * normal, supportB);
			float distance = plane.GetDistanceFromPlane(supportB);

			if (firstIteration || distance > maxDistance)
			{
				maxDistance = distance;
				out_bestPlane = plane;
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

	int bestAFaceIndex = -1;
	int bestBFaceIndex = -1;

	float aOntoBDistance = SolvePartialSAT(worldShapeA, worldShapeB, bestAFaceIndex);

	if (aOntoBDistance > 0.f)
		return BroadphaseResult3d(false);

	float bOntoADistance = SolvePartialSAT(worldShapeB, worldShapeA, bestBFaceIndex);

	if (bOntoADistance > 0.f)
		return BroadphaseResult3d(false);

	Plane3 bestPlane;
	float edgeDistance = SolveEdgeSAT(worldShapeA, worldShapeB, bestPlane);

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
		result.m_direction = worldShapeA->GetFaceNormal(bestAFaceIndex);
		result.m_position = worldShapeA->GetFaceSupportPlane(bestAFaceIndex).GetDistance() * worldShapeA->GetFaceSupportPlane(bestAFaceIndex).GetNormal();
	}
	else if (result.m_penetration == penOnFaceB)
	{
		result.m_direction = -1.0f * worldShapeB->GetFaceNormal(bestBFaceIndex); // Flip the axis so it points from A to B
		result.m_position = worldShapeB->GetFaceSupportPlane(bestBFaceIndex).GetDistance() * worldShapeB->GetFaceSupportPlane(bestBFaceIndex).GetNormal();
	}
	else
	{
		result.m_direction = bestPlane.GetNormal();
		result.m_position = bestPlane.GetDistance() * bestPlane.GetNormal();
	}
	
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

