///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: May 6th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Collision/BoundingVolumeHierarchy/BoundingVolume.h"
#include "Engine/Collision/Collider.h"
#include "Engine/Core/EngineCommon.h"
#include "Engine/Core/Rgba.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/Transform.h"
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
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
BoundingVolumeSphere::BoundingVolumeSphere(const Sphere3D& sphere)
{
	center = sphere.center;
	radius = sphere.radius;
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
BoundingVolumeSphere::BoundingVolumeSphere()
{
	radius = 1.f;
	center = Vector3::ZERO;
}


//-------------------------------------------------------------------------------------------------
BoundingVolumeSphere::BoundingVolumeSphere(const SphereCollider& colSphere)
{
	// Bounding volume sphere.....for a sphere.....is the sphere
	(*this) = colSphere.GetDataInWorldSpace();
}


//-------------------------------------------------------------------------------------------------
BoundingVolumeSphere::BoundingVolumeSphere(const BoxCollider& colBox)
{
	OBB3 colBoxWs = colBox.GetDataInWorldSpace();

	// Since all points of the box are equidistant from the center, the length of the extents
	// is the max radius we'd need to include all points
	center = colBoxWs.center;
	radius = colBoxWs.extents.GetLength();
}


//-------------------------------------------------------------------------------------------------
BoundingVolumeSphere::BoundingVolumeSphere(const CapsuleCollider& capsuleCol)
{
	Capsule3D capsuleWs = capsuleCol.GetDataInWorldSpace();

	center = 0.5f * (capsuleWs.start + capsuleWs.end);
	radius = 0.5f * (capsuleWs.start - capsuleWs.end).GetLength() + capsuleWs.radius;
}


//-------------------------------------------------------------------------------------------------
BoundingVolumeSphere BoundingVolumeSphere::GetTransformApplied(const Transform& transform)
{
	BoundingVolumeSphere result;
	result.center = transform.TransformPosition(center);
	result.radius = radius;
	return result;
}


//-------------------------------------------------------------------------------------------------
void BoundingVolumeSphere::DebugRender() const
{
	UNIMPLEMENTED();
	//DebugRenderOptions options;
	//options.m_color = Rgba::RED;
	//options.m_lifetime = 0.f;
	//options.m_fillMode = FILL_MODE_WIREFRAME;

	//DebugRenderSphere* drawSphere = new DebugRenderSphere(center, radius, options);
	//g_debugRenderSystem->AddObject(drawSphere);
}


//-------------------------------------------------------------------------------------------------
bool BoundingVolumeSphere::Overlaps(const BoundingVolumeSphere& sphere) const
{
	return DoSpheresOverlap(*this, sphere);
}


//-------------------------------------------------------------------------------------------------
bool BoundingVolumeSphere::Overlaps(const HalfSpaceCollider* halfspace) const
{
	Plane3 plane = halfspace->GetDataInWorldSpace();
	float distance = plane.GetDistanceFromPlane(center) - radius;

	return (distance < 0.f);
}


//-------------------------------------------------------------------------------------------------
bool BoundingVolumeSphere::Overlaps(const PlaneCollider* planeCol) const
{
	Plane3 plane = planeCol->GetDataInWorldSpace();
	float distance = Abs(plane.GetDistanceFromPlane(center));

	return (distance < radius);
}


//-------------------------------------------------------------------------------------------------
float BoundingVolumeSphere::GetGrowth(const BoundingVolumeSphere& other) const
{
	// Gauge growth by change in volume
	// We *cannot* move this sphere to encapsulate other, as then we may uncover something we're already encapsulating
	// We can only grow our radius
	float distance = (center - other.center).GetLength();
	float radiusNeeded = distance + other.radius;

	float currVolume = (4.f / 3.f) * PI * radius * radius * radius;
	float growthVolume = (4.f / 3.f) * PI * radiusNeeded * radiusNeeded * radiusNeeded;
	
	return Max(0.f, growthVolume - currVolume); // Clamp above zero to indicate when we don't need to grow
}
