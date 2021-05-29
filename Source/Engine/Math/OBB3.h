///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 20th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Math/AABB3.h"
#include "Engine/Math/Face3.h"
#include "Engine/Math/Plane3.h"
#include "Engine/Math/Transform.h"
#include <vector>

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

//-------------------------------------------------------------------------------------------------
class OBB3
{
public:
	//-----Public Methods-----

	OBB3() {}
	OBB3(const Vector3& center, const Vector3& extents, const Vector3& rotation);
	OBB3(const Vector3& center, const Vector3& extents, const Quaternion& rotation);

	Vector3		GetCenter() const { return center; }
	Vector3		GetExtents() const { return extents; }
	Vector3		GetRotationDegrees() const { return rotation.GetAsEulerAnglesDegrees(); }

	Vector3		GetMinsWs() const;
	Vector3		GetMaxsWs() const;
	void		GetPoints(Vector3 out_points[8]) const;

	Vector3		GetRightVector() const;
	Vector3		GetUpVector() const;
	Vector3		GetForwardVector() const;
	Matrix4		GetModelMatrix() const;
	Face3		GetFaceInDirection(const Vector3& direction) const;

	void		GetFaceSupportPlanes(Plane3* out_planes) const;
	void		GetEdges(Edge3* out_edges) const;
	Vector3		TransformPositionIntoSpace(const Vector3& position) const;
	Vector3		TransformPositionOutOfSpace(const Vector3& position) const;
	bool		ContainsWorldSpacePoint(const Vector3& pointWs) const;


public:
	//-----Public Data-----

	Vector3		center;
	Vector3		extents;
	Quaternion	rotation;

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
