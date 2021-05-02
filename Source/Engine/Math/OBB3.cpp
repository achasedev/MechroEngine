///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 20th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/Math/MathUtils.h"
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
	: center(center), extents(extents), rotation(Quaternion::CreateFromEulerAnglesDegrees(rotationDegrees))
{
}


//-------------------------------------------------------------------------------------------------
OBB3::OBB3(const Vector3& center, const Vector3& extents, const Quaternion& rotation)
	: center(center), extents(extents), rotation(rotation)
{
}


//-------------------------------------------------------------------------------------------------
void OBB3::GetPoints(Vector3 out_points[8]) const
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
Vector3 OBB3::GetMinsWs() const
{
	return rotation.RotatePoint(center - extents);
}


//-------------------------------------------------------------------------------------------------
Vector3 OBB3::GetMaxsWs() const
{
	return rotation.RotatePoint(center + extents);
}


//-------------------------------------------------------------------------------------------------
Vector3 OBB3::GetRightVector() const
{
	Matrix4 rotAsMatrix = Matrix4::MakeRotation(rotation);
	return rotAsMatrix.GetIVector().xyz();
}


//-------------------------------------------------------------------------------------------------
Vector3 OBB3::GetUpVector() const
{
	Matrix4 rotAsMatrix = Matrix4::MakeRotation(rotation);
	return rotAsMatrix.GetJVector().xyz();
}


//-------------------------------------------------------------------------------------------------
Vector3 OBB3::GetForwardVector() const
{
	Matrix4 rotAsMatrix = Matrix4::MakeRotation(rotation);
	return rotAsMatrix.GetKVector().xyz();
}


//-------------------------------------------------------------------------------------------------
Matrix4 OBB3::GetModelMatrix() const
{
	return Matrix4::MakeModelMatrix(center, rotation.GetAsEulerAnglesDegrees(), extents);
}


//-------------------------------------------------------------------------------------------------
Face3 OBB3::GetFaceInDirection(const Vector3& direction) const
{
	Vector3 right	= GetRightVector();
	Vector3 left	= -1.0f * right;
	Vector3 up		= GetUpVector();
	Vector3 down	= -1.0f * up;
	Vector3 forward = GetForwardVector();
	Vector3 back	= -1.0f * forward;
	
	float rightDot		= DotProduct(right, direction);
	float leftDot		= DotProduct(left, direction);
	float upDot			= DotProduct(up, direction);
	float downDot		= DotProduct(down, direction);
	float forwardDot	= DotProduct(forward, direction);
	float backDot		= DotProduct(back, direction);

	float maxDot = Max(rightDot, leftDot, upDot, downDot, forwardDot, backDot);
	Vector3 p0, p1, p2, p3;

	if (maxDot == rightDot)
	{
		p0 = Vector3(center.x + extents.x, center.y - extents.y, center.z - extents.z);
		p1 = Vector3(center.x + extents.x, center.y + extents.y, center.z - extents.z);
		p2 = Vector3(center.x + extents.x, center.y + extents.y, center.z + extents.z);
		p3 = Vector3(center.x + extents.x, center.y - extents.y, center.z + extents.z);
	}
	else if (maxDot == leftDot)
	{
		p0 = Vector3(center.x - extents.x, center.y - extents.y, center.z + extents.z);
		p1 = Vector3(center.x - extents.x, center.y + extents.y, center.z + extents.z);
		p2 = Vector3(center.x - extents.x, center.y + extents.y, center.z - extents.z);
		p3 = Vector3(center.x - extents.x, center.y - extents.y, center.z - extents.z);
	}
	else if (maxDot == forwardDot)
	{
		p0 = Vector3(center.x + extents.x, center.y - extents.y, center.z + extents.z);
		p1 = Vector3(center.x + extents.x, center.y + extents.y, center.z + extents.z);
		p2 = Vector3(center.x - extents.x, center.y + extents.y, center.z + extents.z);
		p3 = Vector3(center.x - extents.x, center.y - extents.y, center.z + extents.z);
	}
	else if (maxDot == backDot)
	{
		p0 = Vector3(center.x - extents.x, center.y - extents.y, center.z - extents.z);
		p1 = Vector3(center.x - extents.x, center.y + extents.y, center.z - extents.z);
		p2 = Vector3(center.x + extents.x, center.y + extents.y, center.z - extents.z);
		p3 = Vector3(center.x + extents.x, center.y - extents.y, center.z - extents.z);
	}
	else if (maxDot == upDot)
	{
		p0 = Vector3(center.x - extents.x, center.y + extents.y, center.z - extents.z);
		p1 = Vector3(center.x - extents.x, center.y + extents.y, center.z + extents.z);
		p2 = Vector3(center.x + extents.x, center.y + extents.y, center.z + extents.z);
		p3 = Vector3(center.x + extents.x, center.y + extents.y, center.z - extents.z);
	}
	else
	{
		p0 = Vector3(center.x - extents.x, center.y - extents.y, center.z + extents.z);
		p1 = Vector3(center.x - extents.x, center.y - extents.y, center.z - extents.z);
		p2 = Vector3(center.x + extents.x, center.y - extents.y, center.z - extents.z);
		p3 = Vector3(center.x + extents.x, center.y - extents.y, center.z + extents.z);
	}

	Face3 face;
	face.AddVertex(p0);
	face.AddVertex(p1);
	face.AddVertex(p2);
	face.AddVertex(p3);

	return face;
}


//-------------------------------------------------------------------------------------------------
void OBB3::GetFaceSupportPlanes(std::vector<Plane3>& out_planes) const
{
	// Find the local space extremes in world space
	Vector3 minsWs = GetMinsWs();
	Vector3 maxsWs = GetMaxsWs();

	// Normals to the faces will be along these 3 directions
	Vector3 right = GetRightVector().GetNormalized();
	Vector3 up = GetUpVector().GetNormalized();
	Vector3 forward = GetForwardVector().GetNormalized();

	out_planes.clear();
	out_planes.push_back(Plane3(-1.f * right, minsWs));
	out_planes.push_back(Plane3(right, maxsWs));
	out_planes.push_back(Plane3(-1.f * up, minsWs));
	out_planes.push_back(Plane3(up, maxsWs));
	out_planes.push_back(Plane3(-1.f * forward, minsWs));
	out_planes.push_back(Plane3(forward, maxsWs));
}
