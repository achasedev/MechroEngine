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
		result.m_magnitude = aToB.Normalize();
		result.m_direction = aToB;
	}

	return result;
}


//-------------------------------------------------------------------------------------------------
BroadphaseResult3d CollisionUtils3d::Collide(BoxCollider3d* colA, BoxCollider3d* colB)
{
	OBB3 boundsA = colA->GetWorldBounds();
	OBB3 boundsB = colB->GetWorldBounds();


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
	OBB3 boxB = colB->GetWorldBounds();

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
			if (penetration < result.m_magnitude)
			{
				result.m_magnitude = penetration;
				result.m_direction = -1.0f * plane.GetNormal(); // Direction is always from A
			}
		}
	}

	return result;
}
