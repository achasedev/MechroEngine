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
#include "Engine/Math/MathUtils.h"

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
	Sphere3d sphereA = colA->GetWorldBounds();
	Sphere3d sphereB = colB->GetWorldBounds();

	Vector3 aToB = sphereB.center - sphereA.center;
	float distanceSquared = (sphereB.center - sphereA.center).GetLengthSquared();
	float radiusSquared = (sphereB.radius + sphereA.radius) * (sphereB.radius + sphereA.radius);

	BroadphaseResult3d result;

	if (distanceSquared < radiusSquared)
	{
		result.m_collisionFound = true;
		result.m_penetration = aToB.Normalize();
		result.m_direction = aToB;
	}

	return result;
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
BroadphaseResult3d CollisionUtils3d::Collide(PolytopeCollider3d* colA, BoxCollider3d* colB)
{
	UNIMPLEMENTED();
	UNUSED(colA);
	UNUSED(colB);
}


//-------------------------------------------------------------------------------------------------
BroadphaseResult3d CollisionUtils3d::Collide(BoxCollider3d* colA, PolytopeCollider3d* colB)
{
	UNIMPLEMENTED();
	UNUSED(colA);
	UNUSED(colB);
}


//-------------------------------------------------------------------------------------------------
BroadphaseResult3d CollisionUtils3d::Collide(CapsuleCollider3d* colA, BoxCollider3d* colB)
{
	UNIMPLEMENTED();
	UNUSED(colA);
	UNUSED(colB);
}


//-------------------------------------------------------------------------------------------------
BroadphaseResult3d CollisionUtils3d::Collide(BoxCollider3d* colA, CapsuleCollider3d* colB)
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
BroadphaseResult3d CollisionUtils3d::Collide(PolytopeCollider3d* colA, PolytopeCollider3d* colB)
{
	UNIMPLEMENTED();
	UNUSED(colA);
	UNUSED(colB);
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
BroadphaseResult3d CollisionUtils3d::Collide(BoxCollider3d* colA, BoxCollider3d* colB)
{
	OBB3 shapeA = colA->GetShapeWs();
	OBB3 shapeB = colB->GetShapeWs();

	Vector3 pointsA[8];
	Vector3 pointsB[8];
	shapeA.GetPoints(pointsA);
	shapeB.GetPoints(pointsB);

	Vector3 axes[6];
	axes[0] = shapeA.GetRightVector().GetNormalized();
	axes[1] = shapeA.GetUpVector().GetNormalized();
	axes[2] = shapeA.GetForwardVector().GetNormalized();
	axes[3] = shapeB.GetRightVector().GetNormalized();
	axes[4] = shapeB.GetUpVector().GetNormalized();
	axes[5] = shapeB.GetForwardVector().GetNormalized();

	BroadphaseResult3d result;

	for (int axisIndex = 0; axisIndex < 6; ++axisIndex)
	{
		const Vector3& axis = axes[axisIndex];
		bool isAAxis = (axisIndex < 3);

		Range projRangeA;
		Range projRangeB;

		for (int pointIndex = 0; pointIndex < 8; ++pointIndex)
		{
			const Vector3& pointA = pointsA[pointIndex];
			const Vector3& pointB = pointsB[pointIndex];

			float dotA = DotProduct(pointA, axis);
			float dotB = DotProduct(pointB, axis);

			if (pointIndex == 0)
			{
				projRangeA.min = dotA;
				projRangeA.max = dotA;
				projRangeB.min = dotB;
				projRangeB.max = dotB;
			}
			else
			{
				projRangeA.min = Min(projRangeA.min, dotA);
				projRangeA.max = Max(projRangeA.max, dotA);
				projRangeB.min = Min(projRangeB.min, dotB);
				projRangeB.max = Max(projRangeB.max, dotB);
			}
		}

		if (!DoRangesOverlap(projRangeA, projRangeB))
		{
			return BroadphaseResult3d(false);
		}

		float pen1 = projRangeB.max - projRangeA.min;
		float pen2 = projRangeA.max - projRangeB.min;
		float minPen = Min(pen1, pen2);

		if (minPen < result.m_penetration)
		{
			result.m_penetration = minPen;
			result.m_direction = axis;

			// Ensure we point all normals from A to B
			Vector3 aToB = shapeB.center - shapeA.center;
			if (DotProduct(aToB, result.m_direction) < 0.f)
			{
				result.m_direction *= -1.0f;
			}
		}
	}

	result.m_collisionFound = true;
	return result;



	//// Extents are symmetrical, as they range from center
	//Range xRangeA = Range(-xExtentA, xExtentA);
	//Range yRangeA = Range(-yExtentA, yExtentA);
	//Range zRangeA = Range(-zExtentA, zExtentA);

	//Range xRangeB(FLT_MAX, FLT_MIN);
	//Range yRangeB(FLT_MAX, FLT_MIN);
	//Range zRangeB(FLT_MAX, FLT_MIN);

	//// Project all points of B into A's axes
	//Vector3 pointsB[8];
	//boundsB.GetPoints(pointsB);

	//for (int i = 0; i < 8; ++i)
	//{
	//	// B's projected values onto A's axes
	//	float x = DotProduct(pointsB[i], rightA);
	//	float y = DotProduct(pointsB[i], upA);
	//	float z = DotProduct(pointsB[i], forwardA);

	//	xRangeB.min = Min(xRangeB.min, x);
	//	xRangeB.max = Max(xRangeB.max, x);

	//	yRangeB.min = Min(yRangeB.min, y);
	//	yRangeB.max = Max(yRangeB.max, y);
	//	
	//	zRangeB.min = Min(zRangeB.min, z);
	//	zRangeB.max = Max(zRangeB.max, z);
	//}

	//ASSERT_RECOVERABLE(xRangeA.IsValid(), "");
	//ASSERT_RECOVERABLE(yRangeA.IsValid(), "");
	//ASSERT_RECOVERABLE(zRangeA.IsValid(), "");
	//ASSERT_RECOVERABLE(xRangeB.IsValid(), "");
	//ASSERT_RECOVERABLE(yRangeB.IsValid(), "");
	//ASSERT_RECOVERABLE(zRangeB.IsValid(), "");

	//float xOverlap = xRangeA.GetOverlap(xRangeB);
	//float yOverlap = yRangeA.GetOverlap(yRangeB);
	//float zOverlap = zRangeA.GetOverlap(zRangeB);

	//BroadphaseResult3d result;

	//// Intersecting only if we overlap on all axes
	//if (xOverlap > 0.f && yOverlap > 0.f && zOverlap > 0.f)
	//{
	//	result.m_collisionFound = true;

	//	// Overlap =/= correction
	//	// Find which axis has the least correction

	//	float minOverlap = Min(xOverlap, yOverlap, zOverlap);

	//	float leftCorrection = xRangeB.max - xRangeA.min;
	//	float rightCorrection = xRangeA.max - xRangeB.min;
	//	float downCorrection = yRangeB.max - yRangeA.min;
	//	float upCorrection = yRangeA.max - yRangeB.min;
	//	float backCorrection = zRangeB.max - zRangeA.min;
	//	float forwardCorrection = zRangeA.max - zRangeB.min;




	//	if (minOverlap == xOverlap)
	//	{
	//		float leftCorrection = xRangeB.max - xRangeA.min;
	//		float rightCorrection = xRangeA.max - xRangeB.min;

	//		if (leftCorrection < rightCorrection)
	//		{
	//			result.m_direction = -1.0f * rightA;
	//			result.m_penetration = leftCorrection;
	//		}
	//		else
	//		{
	//			result.m_direction = rightA;
	//			result.m_penetration = rightCorrection;
	//		}
	//	}
	//	else if (minOverlap == yOverlap)
	//	{
	//		float downCorrection = yRangeB.max - yRangeA.min;
	//		float upCorrection = yRangeA.max - yRangeB.min;

	//		if (downCorrection < upCorrection)
	//		{
	//			result.m_direction = -1.0f * upA;
	//			result.m_penetration = downCorrection;
	//		}
	//		else
	//		{
	//			result.m_direction = upA;
	//			result.m_penetration = upCorrection;
	//		}
	//	}
	//	else
	//	{
	//		float backCorrection = zRangeB.max - zRangeA.min;
	//		float forwardCorrection = zRangeA.max - zRangeB.min;

	//		if (backCorrection < forwardCorrection)
	//		{
	//			result.m_direction = -1.0f * forwardA;
	//			result.m_penetration = backCorrection;
	//		}
	//		else
	//		{
	//			result.m_direction = forwardA;
	//			result.m_penetration = forwardCorrection;
	//		}
	//	}
	//}

	//return result;
}


//-------------------------------------------------------------------------------------------------
BroadphaseResult3d CollisionUtils3d::Collide(BoxCollider3d* colA, SphereCollider3d* colB)
{
	BroadphaseResult3d result = CollisionUtils3d::Collide(colB, colA);
	result.m_direction *= -1.0f;

	return result;
}


//-------------------------------------------------------------------------------------------------
BroadphaseResult3d CollisionUtils3d::Collide(SphereCollider3d* colA, BoxCollider3d* colB)
{
	Sphere3d sphereA = colA->GetWorldBounds();
	OBB3 boxB = colB->GetShapeWs();

	std::vector<Plane> planesB;
	boxB.GetFaceSupportPlanes(planesB);
	int numPlanes = (int)planesB.size();

	BroadphaseResult3d result;

	for (int i = 0; i < numPlanes; ++i)
	{
		const Plane& plane = planesB[i];

		float distanceFromPlane = plane.GetDistanceFromPlane(sphereA.center) - sphereA.radius;
		if (distanceFromPlane < 0.f)
		{
			result.m_collisionFound = true;

			float penetration = Abs(distanceFromPlane);
			if (penetration < result.m_penetration)
			{
				result.m_penetration = penetration;
				result.m_direction = -1.0f * plane.GetNormal(); // Direction is always from A
			}
		}
	}

	return result;
}
