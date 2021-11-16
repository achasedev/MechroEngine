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
BoundingVolumeSphere::BoundingVolumeSphere(const Sphere& sphere)
{
	m_center = sphere.m_center;
	m_radius = sphere.m_radius;
}


//-------------------------------------------------------------------------------------------------
BoundingVolumeSphere::BoundingVolumeSphere(const BoundingVolumeSphere& a, const BoundingVolumeSphere& b)
{
	// We need to create a bounding volume that contains both of these bounding volumes
	// First check if one is completely contained in the other - if so, this volume can just become the containing volume

	Vector3 aToB = b.m_center - a.m_center;
	float distanceSquared = aToB.GetLengthSquared();

	float radiusDiff = a.m_radius - b.m_radius;

	if (radiusDiff * radiusDiff >= distanceSquared)
	{
		if (a.m_radius > b.m_radius)
		{
			m_center = a.m_center;
			m_radius = a.m_radius;
		}
		else
		{
			m_center = b.m_center;
			m_radius = b.m_radius;
		}
	}
	else
	{
		// Need to create a sphere that encapsulates both spheres as tightly as possible
		float distance = Sqrt(distanceSquared);
		m_radius = 0.5f * (a.m_radius + b.m_radius + distance); // From the far edge of a to the far edge of b, then take half
		
		m_center = a.m_center;
		if (distance > 0.f)
		{
			m_center += aToB * ((m_radius - a.m_radius) / distance);
		}
	}
}


//-------------------------------------------------------------------------------------------------
BoundingVolumeSphere::BoundingVolumeSphere()
{
	m_radius = 1.f;
	m_center = Vector3::ZERO;
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
	m_center = colBoxWs.center;
	m_radius = colBoxWs.extents.GetLength();
}


//-------------------------------------------------------------------------------------------------
BoundingVolumeSphere::BoundingVolumeSphere(const CapsuleCollider& capsuleCol)
{
	Capsule3 capsuleWs = capsuleCol.GetDataInWorldSpace();

	m_center = 0.5f * (capsuleWs.start + capsuleWs.end);
	m_radius = 0.5f * (capsuleWs.start - capsuleWs.end).GetLength() + capsuleWs.radius;
}


//-------------------------------------------------------------------------------------------------
BoundingVolumeSphere::BoundingVolumeSphere(const CylinderCollider& cylinderCol)
{
	Cylinder cylinderWs = cylinderCol.GetDataInWorldSpace();

	m_center = 0.5f * (cylinderWs.m_bottom + cylinderWs.m_top);

	// Holy cows, an application of Pythagoras' Theorem :D
	float aSquared = (cylinderWs.m_bottom - m_center).GetLengthSquared();
	float bSquared = (cylinderWs.m_radius * cylinderWs.m_radius);

	m_radius = Sqrt(aSquared + bSquared);
}


//-------------------------------------------------------------------------------------------------
BoundingVolumeSphere::BoundingVolumeSphere(const ConvexHullCollider& polyCol)
{
	Polyhedron polyWs = polyCol.GetDataInWorldSpace();
	int numVerts = polyWs.GetNumVertices();

	Vector3 avgPos = Vector3::ZERO;
	for (int iVert = 0; iVert < numVerts; ++iVert)
	{
		avgPos += polyWs.GetVertexPosition(iVert);
	}

	avgPos /= (float)numVerts;

	float maxDistSqr = -1.0f;
	for (int iVert = 0; iVert < numVerts; ++iVert)
	{
		float distSqr = (avgPos - polyWs.GetVertexPosition(iVert)).GetLengthSquared();
		maxDistSqr = Max(maxDistSqr, distSqr);
	}

	m_center = avgPos;
	m_radius = Sqrt(maxDistSqr);
}


//-------------------------------------------------------------------------------------------------
BoundingVolumeSphere BoundingVolumeSphere::GetTransformApplied(const Transform& transform)
{
	BoundingVolumeSphere result;
	result.m_center = transform.TransformPosition(m_center);
	result.m_radius = m_radius;
	return result;
}


//-------------------------------------------------------------------------------------------------
void BoundingVolumeSphere::DebugRender() const
{
	DebugRenderOptions options;
	options.m_startColor = Rgba::CYAN;
	options.m_endColor = Rgba::CYAN;
	options.m_lifetime = 0.f;
	options.m_fillMode = FILL_MODE_WIREFRAME;
	options.m_cullMode = CULL_MODE_NONE; // To see the bounding volume from the inside

	DebugDrawSphere(m_center, m_radius, options);
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
	float distance = plane.GetDistanceFromPlane(m_center) - m_radius;

	return (distance < 0.f);
}


//-------------------------------------------------------------------------------------------------
bool BoundingVolumeSphere::Overlaps(const PlaneCollider* planeCol) const
{
	Plane3 plane = planeCol->GetDataInWorldSpace();
	float distance = Abs(plane.GetDistanceFromPlane(m_center));

	return (distance < m_radius);
}


//-------------------------------------------------------------------------------------------------
float BoundingVolumeSphere::GetGrowth(const BoundingVolumeSphere& other) const
{
	// Gauge growth by change in volume
	// We *cannot* move this sphere to encapsulate other, as then we may uncover something we're already encapsulating
	// We can only grow our radius
	float distance = (m_center - other.m_center).GetLength();
	float radiusNeeded = distance + other.m_radius;

	float currVolume = (4.f / 3.f) * PI * m_radius * m_radius * m_radius;
	float growthVolume = (4.f / 3.f) * PI * radiusNeeded * radiusNeeded * radiusNeeded;
	
	return Max(0.f, growthVolume - currVolume); // Clamp above zero to indicate when we don't need to grow
}
