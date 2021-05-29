///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 20th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/Math/Matrix3.h"
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

	Transform transform(center, rotation, extents);
	out_points[0] = transform.TransformPosition(Vector3(-1.f, -1.f, -1.f)); // Left, Bottom, Back
	out_points[1] = transform.TransformPosition(Vector3(-1.f, 1.f, -1.f));	// Left, Top, Back
	out_points[2] = transform.TransformPosition(Vector3(1.f, 1.f, -1.f));	// Right, Top, Back 
	out_points[3] = transform.TransformPosition(Vector3(1.f, -1.f, -1.f));	// Right, Bottom, Back
	out_points[4] = transform.TransformPosition(Vector3(1.f, -1.f, 1.f));	// Right, Bottom, Front
	out_points[5] = transform.TransformPosition(Vector3(1.f, 1.f, 1.f));	// Right, Top, Front
	out_points[6] = transform.TransformPosition(Vector3(-1.f, 1.f, 1.f));	// Left, Top, Front
	out_points[7] = transform.TransformPosition(Vector3(-1.f, -1.f, 1.f));	// Left, Bottom, Front

	//out_points[0] = center + rotation.RotatePosition(Vector3(-extents.x, -extents.y, -extents.z)); // Left, Bottom, Back
	//out_points[1] = center + rotation.RotatePosition(Vector3(-extents.x, extents.y, -extents.z));	// Left, Top, Back
	//out_points[2] = center + rotation.RotatePosition(Vector3(extents.x, extents.y, -extents.z));	// Right, Top, Back 
	//out_points[3] = center + rotation.RotatePosition(Vector3(extents.x, -extents.y, -extents.z));	// Right, Bottom, Back
	//out_points[4] = center + rotation.RotatePosition(Vector3(extents.x, -extents.y, extents.z));	// Right, Bottom, Front
	//out_points[5] = center + rotation.RotatePosition(Vector3(extents.x, extents.y, extents.z));	// Right, Top, Front
	//out_points[6] = center + rotation.RotatePosition(Vector3(-extents.x, extents.y, extents.z));	// Left, Top, Front
	//out_points[7] = center + rotation.RotatePosition(Vector3(-extents.x, -extents.y, extents.z));	// Left, Bottom, Front
}


//-------------------------------------------------------------------------------------------------
Vector3 OBB3::GetMinsWs() const
{
	Transform transform(center, rotation, extents);
	return transform.TransformPosition(Vector3(-1.f, -1.f, -1.f)); // Left, Bottom, Back

}


//-------------------------------------------------------------------------------------------------
Vector3 OBB3::GetMaxsWs() const
{
	Transform transform(center, rotation, extents);
	return transform.TransformPosition(Vector3(1.f, 1.f, 1.f));	// Right, Top, Front
}


//-------------------------------------------------------------------------------------------------
Vector3 OBB3::GetRightVector() const
{
	Matrix3 rotAsMatrix = Matrix3(rotation);
	return rotAsMatrix.iBasis;
}


//-------------------------------------------------------------------------------------------------
Vector3 OBB3::GetUpVector() const
{
	Matrix3 rotAsMatrix = Matrix3(rotation);
	return rotAsMatrix.jBasis;
}


//-------------------------------------------------------------------------------------------------
Vector3 OBB3::GetForwardVector() const
{
	Matrix3 rotAsMatrix = Matrix3(rotation);
	return rotAsMatrix.kBasis;
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
void OBB3::GetFaceSupportPlanes(Plane3* out_planes) const
{
	// Find the local space extremes in world space
	Vector3 minsWs = GetMinsWs();
	Vector3 maxsWs = GetMaxsWs();

	// Normals to the faces will be along these 3 directions
	Vector3 right = GetRightVector().GetNormalized();
	Vector3 up = GetUpVector().GetNormalized();
	Vector3 forward = GetForwardVector().GetNormalized();

	out_planes[0] = Plane3(right, maxsWs);
	out_planes[1] = Plane3(-1.f * right, minsWs);
	out_planes[2] = Plane3(up, maxsWs);
	out_planes[3] = Plane3(-1.f * up, minsWs);
	out_planes[4] = Plane3(forward, maxsWs);
	out_planes[5] = Plane3(-1.f * forward, minsWs);
}


//-------------------------------------------------------------------------------------------------
void OBB3::GetEdges(Edge3* out_edges) const
{
	Vector3 points[8];
	GetPoints(points);

	// Back face
	out_edges[0] = Edge3(points[0], points[1]);
	out_edges[1] = Edge3(points[1], points[2]);
	out_edges[2] = Edge3(points[2], points[3]);
	out_edges[3] = Edge3(points[3], points[0]);

	// Front face
	out_edges[4] = Edge3(points[4], points[5]);
	out_edges[5] = Edge3(points[5], points[6]);
	out_edges[6] = Edge3(points[6], points[7]);
	out_edges[7] = Edge3(points[7], points[4]);

	// Side edges
	out_edges[8] = Edge3(points[0], points[7]);
	out_edges[9] = Edge3(points[1], points[6]);
	out_edges[10] = Edge3(points[2], points[5]);
	out_edges[11] = Edge3(points[3], points[4]);
}


//-------------------------------------------------------------------------------------------------
Vector3 OBB3::TransformPositionIntoSpace(const Vector3& position) const
{
	Matrix3 basis = Matrix3(rotation);
	return basis.GetInverse() * (position - center);
}


//-------------------------------------------------------------------------------------------------
Vector3 OBB3::TransformPositionOutOfSpace(const Vector3& position) const
{
	Matrix3 basis = Matrix3(rotation);
	return (basis * position) + center;
}


//-------------------------------------------------------------------------------------------------
bool OBB3::ContainsWorldSpacePoint(const Vector3& pointWs) const
{
	Vector3 pointLs = TransformPositionIntoSpace(pointWs);

	return (pointLs.x > -extents.x) && (pointLs.x < extents.x) && (pointLs.y > -extents.y) && (pointLs.y < extents.y) && (pointLs.z > -extents.z) && (pointLs.z < extents.z);
}
