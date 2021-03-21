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
OBB3::OBB3(const Vector3& center, const Vector3& extents, const Vector3& rotationDegrees)
	: center(center), extents(extents), rotation(Quaternion::FromEuler(rotationDegrees))
{
}


//-------------------------------------------------------------------------------------------------
OBB3::OBB3(const Vector3& center, const Vector3& extents, const Quaternion& rotation)
	: center(center), extents(extents), rotation(rotation)
{
}


//-------------------------------------------------------------------------------------------------
void OBB3::GetPoints(Vector3 out_points[8])
{
	//out_points[0] = rotation.RotatePoint(center) + Vector3(-extents.x, -extents.y, -extents.z); // Left, Bottom, Back
	//out_points[1] = rotation.RotatePoint(center) + Vector3(-extents.x, extents.y, -extents.z);	// Left, Top, Back
	//out_points[2] = rotation.RotatePoint(center) + Vector3(extents.x, extents.y, -extents.z);	// Right, Top, Back 
	//out_points[3] = rotation.RotatePoint(center) + Vector3(extents.x, -extents.y, -extents.z);	// Right, Bottom, Back
	//out_points[4] = rotation.RotatePoint(center) + Vector3(extents.x, -extents.y, extents.z);	// Right, Bottom, Front
	//out_points[5] = rotation.RotatePoint(center) + Vector3(extents.x, extents.y, extents.z);	// Right, Top, Front
	//out_points[6] = rotation.RotatePoint(center) + Vector3(-extents.x, extents.y, extents.z);	// Left, Top, Front
	//out_points[7] = rotation.RotatePoint(center) + Vector3(-extents.x, -extents.y, extents.z);	// Left, Bottom, Front

	out_points[0] = center + rotation.RotatePoint(Vector3(-extents.x, -extents.y, -extents.z)); // Left, Bottom, Back
	out_points[1] = center + rotation.RotatePoint(Vector3(-extents.x, extents.y, -extents.z));	// Left, Top, Back
	out_points[2] = center + rotation.RotatePoint(Vector3(extents.x, extents.y, -extents.z));	// Right, Top, Back 
	out_points[3] = center + rotation.RotatePoint(Vector3(extents.x, -extents.y, -extents.z));	// Right, Bottom, Back
	out_points[4] = center + rotation.RotatePoint(Vector3(extents.x, -extents.y, extents.z));	// Right, Bottom, Front
	out_points[5] = center + rotation.RotatePoint(Vector3(extents.x, extents.y, extents.z));	// Right, Top, Front
	out_points[6] = center + rotation.RotatePoint(Vector3(-extents.x, extents.y, extents.z));	// Left, Top, Front
	out_points[7] = center + rotation.RotatePoint(Vector3(-extents.x, -extents.y, extents.z));	// Left, Bottom, Front
}


//-------------------------------------------------------------------------------------------------
Vector3 OBB3::GetMinsWs()
{
	return rotation.RotatePoint(center - extents);
}


//-------------------------------------------------------------------------------------------------
Vector3 OBB3::GetMaxsWs()
{
	return rotation.RotatePoint(center + extents);
}


//-------------------------------------------------------------------------------------------------
Vector3 OBB3::GetRightVector()
{
	Matrix44 rotAsMatrix = Matrix44::MakeRotation(rotation);
	return rotAsMatrix.GetIVector().xyz();
}


//-------------------------------------------------------------------------------------------------
Vector3 OBB3::GetUpVector()
{
	Matrix44 rotAsMatrix = Matrix44::MakeRotation(rotation);
	return rotAsMatrix.GetJVector().xyz();
}


//-------------------------------------------------------------------------------------------------
Vector3 OBB3::GetForwardVector()
{
	Matrix44 rotAsMatrix = Matrix44::MakeRotation(rotation);
	return rotAsMatrix.GetKVector().xyz();
}


//-------------------------------------------------------------------------------------------------
Matrix44 OBB3::GetModelMatrix()
{
	return Matrix44::MakeModelMatrix(center, rotation.GetAsEulerAngles(), extents);
}


//-------------------------------------------------------------------------------------------------
void OBB3::GetFaceSupportPlanes(std::vector<Plane>& out_planes)
{
	// Find the local space extremes in world space
	Vector3 minsWs = GetMinsWs();
	Vector3 maxsWs = GetMaxsWs();

	// Normals to the faces will be along these 3 directions
	Vector3 right = GetRightVector().GetNormalized();
	Vector3 up = GetUpVector().GetNormalized();
	Vector3 forward = GetForwardVector().GetNormalized();

	out_planes.clear();
	out_planes.push_back(Plane(-1.f * right, minsWs));
	out_planes.push_back(Plane(right, maxsWs));
	out_planes.push_back(Plane(-1.f * up, minsWs));
	out_planes.push_back(Plane(up, maxsWs));
	out_planes.push_back(Plane(-1.f * forward, minsWs));
	out_planes.push_back(Plane(forward, maxsWs));
}
