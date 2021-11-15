///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: May 6th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Math/Sphere3D.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class BoxCollider;
class CapsuleCollider;
class CylinderCollider;
class HalfSpaceCollider;
class PlaneCollider;
class PolygonCollider;
class SphereCollider;
class Transform;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class BoundingVolumeSphere : public Sphere3D
{
public:
	//-----Public Methods-----

	BoundingVolumeSphere();
	BoundingVolumeSphere(const Sphere3D& sphere);
	BoundingVolumeSphere(const BoundingVolumeSphere& a, const BoundingVolumeSphere& b); // For combining bounding volumes
	BoundingVolumeSphere(const SphereCollider& colSphere);
	BoundingVolumeSphere(const BoxCollider& colBox);
	BoundingVolumeSphere(const CapsuleCollider& capsuleCol);
	BoundingVolumeSphere(const CylinderCollider& cylinderCol);
	BoundingVolumeSphere(const PolygonCollider& polyCol);

	BoundingVolumeSphere	GetTransformApplied(const Transform& transform);
	void					DebugRender() const;

	bool					Overlaps(const BoundingVolumeSphere& sphere) const;
	bool					Overlaps(const HalfSpaceCollider* halfspace) const;
	bool					Overlaps(const PlaneCollider* planeCol) const;
	float					GetSize() const { return radius; }
	float					GetGrowth(const BoundingVolumeSphere& other) const;


private:
	//-----Private Data-----

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
