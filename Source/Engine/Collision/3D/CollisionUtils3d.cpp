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
		const PolygonFace3d* face = a->GetFace(faceIndex);
		Plane3 plane = face->GetSupportPlane();

		Vector3 supportDir = -1.0f * plane.GetNormal();
		Vector3 supportPoint;
		b->GetFarthestWorldVertexInDirection(supportDir, supportPoint);

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
BroadphaseResult3d CollisionUtils3d::Collide(PolytopeCollider3d* colA, PolytopeCollider3d* colB)
{
	const Polygon3d* shapeA = colA->GetShape();
	const Polygon3d* shapeB = colB->GetShape();

	int bestAFaceIndex = -1;
	int bestBFaceIndex = -1;

	float aOntoBDistance = SolvePartialSAT(shapeA, shapeB, bestAFaceIndex);
	float bOntoADistance = SolvePartialSAT(shapeB, shapeA, bestBFaceIndex);

	BroadphaseResult3d result;
	if (aOntoBDistance < 0.f && bOntoADistance < 0.f)
	{
		result.m_collisionFound = true;

		float penOnAAxis = Abs(aOntoBDistance);
		float penOnBAxis = Abs(bOntoADistance);

		if (penOnAAxis < penOnBAxis)
		{
			result.m_penetration = penOnAAxis;
			result.m_direction = shapeA->GetFace(bestAFaceIndex)->GetNormal();
		}
		else
		{
			result.m_penetration = penOnBAxis;
			result.m_direction = -1.0f * shapeB->GetFace(bestBFaceIndex)->GetNormal(); // Flip the axis so it points from A to B
		}
	
		return result;
	}

	// TODO: Edges
	UniqueHalfEdgeIterator edgeAIter(*shapeA);

	const HalfEdge* currEdgeA = edgeAIter.GetNext();

	while (currEdgeA != nullptr)
	{
		UniqueHalfEdgeIterator edgeBIter(*shapeB);
		const HalfEdge* currEdgeB = edgeBIter.GetNext();



		currEdgeA = edgeAIter.GetNext();
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

