///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 20th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Math/OBB3.h"

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
OBB3::OBB3(const Vector3& center, const Vector3& extents, const Vector3& rotation)
{
	m_transform = Transform(center, rotation, extents);
}


//-------------------------------------------------------------------------------------------------
void OBB3::GetPoints(Vector3 out_points[8])
{
	out_points[0] = m_transform.TransformPositionLocalToWorld(Vector3(-1.f, -1.f, -1.f)); // Left, Bottom, Back
	out_points[1] = m_transform.TransformPositionLocalToWorld(Vector3(-1.f, 1.f, -1.f));  // Left, Top, Back
	out_points[2] = m_transform.TransformPositionLocalToWorld(Vector3(1.f, 1.f, -1.f));	  // Right, Top, Back 
	out_points[3] = m_transform.TransformPositionLocalToWorld(Vector3(1.f, -1.f, -1.f));  // Right, Bottom, Back
	out_points[4] = m_transform.TransformPositionLocalToWorld(Vector3(1.f, -1.f, 1.f));	  // Right, Bottom, Front
	out_points[5] = m_transform.TransformPositionLocalToWorld(Vector3(1.f, 1.f, 1.f));	  // Right, Top, Front
	out_points[6] = m_transform.TransformPositionLocalToWorld(Vector3(-1.f, 1.f, 1.f));	  // Left, Top, Front
	out_points[7] = m_transform.TransformPositionLocalToWorld(Vector3(-1.f, -1.f, 1.f));  // Left, Bottom, Front
}


//-------------------------------------------------------------------------------------------------
Vector3 OBB3::GetMinsWs()
{
	return m_transform.TransformPositionLocalToWorld(Vector3(-1.f));
}


//-------------------------------------------------------------------------------------------------
Vector3 OBB3::GetMaxsWs()
{
	return m_transform.TransformPositionLocalToWorld(Vector3(1.f));
}


//-------------------------------------------------------------------------------------------------
Vector3 OBB3::GetRightVector()
{
	return m_transform.GetIVector();
}


//-------------------------------------------------------------------------------------------------
Vector3 OBB3::GetUpVector()
{
	return m_transform.GetJVector();
}


//-------------------------------------------------------------------------------------------------
Vector3 OBB3::GetForwardVector()
{
	return m_transform.GetKVector();
}


//-------------------------------------------------------------------------------------------------
void OBB3::GetFaceSupportPlanes(std::vector<Plane>& out_planes)
{
	// Find the local space extremes in world space
	Vector3 minsWs = GetMinsWs();
	Vector3 maxsWs = GetMaxsWs();

	// Normals to the faces will be along these 3 directions
	Vector3 right = m_transform.GetIVector().GetNormalized();
	Vector3 up = m_transform.GetJVector().GetNormalized();
	Vector3 forward = m_transform.GetKVector().GetNormalized();

	out_planes.clear();
	out_planes.push_back(Plane(-1.f * right, minsWs));
	out_planes.push_back(Plane(right, maxsWs));
	out_planes.push_back(Plane(-1.f * up, minsWs));
	out_planes.push_back(Plane(up, maxsWs));
	out_planes.push_back(Plane(-1.f * forward, minsWs));
	out_planes.push_back(Plane(forward, maxsWs));
}
