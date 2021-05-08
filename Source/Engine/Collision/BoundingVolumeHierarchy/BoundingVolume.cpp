///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: May 6th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Collision/BoundingVolumeHierarchy/BoundingVolume.h"
#include "Engine/Core/EngineCommon.h"
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
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
BoundingVolumeSphere::BoundingVolumeSphere(const Sphere3D* sphere)
{
	center = sphere->center;
	radius = sphere->radius;
}


//-------------------------------------------------------------------------------------------------
BoundingVolumeSphere::BoundingVolumeSphere(const BoundingVolumeSphere& a, const BoundingVolumeSphere& b)
{
	// We need to create a bounding volume that contains both of these bounding volumes
	// First check if one is completely contained in the other - if so, this volume can just become the containing volume

	Vector3 aToB = b.center - a.center;
	float distanceSquared = aToB.GetLengthSquared();

	float radiusDiff = a.radius - b.radius;

	if (radiusDiff * radiusDiff >= distanceSquared)
	{
		if (a.radius > b.radius)
		{
			center = a.center;
			radius = a.radius;
		}
		else
		{
			center = b.center;
			radius = b.radius;
		}
	}
	else
	{
		// Need to create a sphere that encapsulates both spheres as tightly as possible
		float distance = Sqrt(distanceSquared);
		radius = 0.5f * (a.radius + b.radius + distance); // From the far edge of a to the far edge of b, then take half
		
		center = a.center;
		if (distance > 0.f)
		{
			center += aToB * ((radius - a.radius) / distance);
		}
	}
}


//-------------------------------------------------------------------------------------------------
bool BoundingVolumeSphere::Overlaps(const BoundingVolumeSphere& sphere)
{
	return DoSpheresOverlap(*this, sphere);
}
