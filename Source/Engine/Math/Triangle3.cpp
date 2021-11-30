///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: Nov 16th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/Matrix3.h"
#include "Engine/Math/Triangle2.h"
#include "Engine/Math/Triangle3.h"


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
// "Flattens" the point to the plane x/y/z = 0 depending on compToFlatten
// The point must be on the plane before flattening, hence the plane parameter
static Vector2 FlattenPoint(const Vector3& point, int compToFlatten, const Plane3& plane)
{
	// Need to project onto plane first
	Vector3 projPt = plane.GetProjectedPointOntoPlane(point);

	Vector2 flatPt;

	switch (compToFlatten)
	{
	case 0:
		// Flatten to x = 0 plane
		flatPt = projPt.yz;
		break;
	case 1:
		// Flatten to y = 0 plane
		flatPt = projPt.xz;
		break;
	case 2:
		// Flatten to z = 0 plane
		flatPt = projPt.xy;
		break;
	default:
		ERROR_AND_DIE("Bad component index!");
		break;
	}

	return flatPt;
}

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
void Triangle3::operator=(const Triangle3& copy)
{
	m_a = copy.m_a;
	m_b = copy.m_b;
	m_c = copy.m_c;
}


//-------------------------------------------------------------------------------------------------
void Triangle3::TransformSelfInto2DBasis(Triangle2& out_triangle2) const
{
	Plane3 plane;
	int compToFlatten = GetComponentToFlatten(plane);

	out_triangle2.m_a = FlattenPoint(m_a, compToFlatten, plane);
	out_triangle2.m_b = FlattenPoint(m_b, compToFlatten, plane);
	out_triangle2.m_c = FlattenPoint(m_c, compToFlatten, plane);
}


//-------------------------------------------------------------------------------------------------
Vector2 Triangle3::TransformPointInto2DBasis(const Vector3& point) const
{
	Plane3 plane;
	int compToFlatten = GetComponentToFlatten(plane);

	return FlattenPoint(point, compToFlatten, plane);
}


//-------------------------------------------------------------------------------------------------
Vector3 Triangle3::TransformPointOutOf2DBasis(const Vector2& point) const
{
	Triangle2 flatTri;
	TransformSelfInto2DBasis(flatTri);
	Vector3 baryCoords = ComputeBarycentricCoordinates(point, flatTri);

	return baryCoords.u * m_a + baryCoords.v * m_b + baryCoords.w * m_c;
}


//-------------------------------------------------------------------------------------------------
int Triangle3::GetComponentToFlatten(Plane3& out_plane) const
{
	Vector3 ab = m_b - m_a;
	Vector3 ac = m_c - m_a;
	Vector3 normal = CrossProduct(ab, ac);
	normal.SafeNormalize(Vector3::ZERO);
	ASSERT_OR_DIE(!AreMostlyEqual(normal, Vector3::ZERO), "Degenerate triangle!");
	out_plane = Plane3(normal, m_a);

	float max = Max(Abs(normal.x), Abs(normal.y), Abs(normal.z));

	if (max == Abs(normal.y))
	{
		// Give priority to flattening on Y first
		return 1;
	}
	else if (max == Abs(normal.z))
	{
		// Then prioritize z
		return 2;
	}
	else
	{
		return 0;
	}
}
