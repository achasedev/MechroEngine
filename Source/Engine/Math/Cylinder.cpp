///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: Nov 5th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/Math/Cylinder.h"
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
// Constructor
Cylinder::Cylinder(const Vector3& bottom, const Vector3& top, float radius)
	: m_bottom(bottom)
	, m_top(top)
	, m_radius(radius)
{
}


//-------------------------------------------------------------------------------------------------
// Returns the point on the cylinder surface furthest in the given direction
// This is always a point on one of the disc edges of the cylinder, except for the cases where direction is perpendicular to the cylinder spine
// In this case, I still choose a disc edge point for consistency
Vector3 Cylinder::GetFurthestEdgePointInDirection(const Vector3& direction, bool* onTop /*= nullptr*/) const
{
	// Get the spine direction and end point based on params
	Vector3 endPoint;
	Vector3 spineDir;
	if (onTop == nullptr)
	{
		float bottomDot = DotProduct(direction, m_bottom);
		float topDot = DotProduct(direction, m_top);
		endPoint = (bottomDot >= topDot ? m_bottom : m_top); // In tie cases, default to bottom
		spineDir = (bottomDot >= topDot ? (m_bottom - m_top) : (m_top - m_bottom));
	}
	else if (*onTop)
	{
		endPoint = m_top;
		spineDir = (m_top - m_bottom);
	}
	else
	{
		endPoint = m_bottom;
		spineDir = (m_bottom - m_top);
	}

	spineDir.Normalize();

	// Project onto the spine vector
	float dot = DotProduct(spineDir, direction);

	// Get the projection of the endPointToPlane onto the disc of the cylinder
	Vector3 discVector = direction - spineDir * dot;
	discVector.SafeNormalize(discVector);

	return endPoint + discVector * m_radius;
}


//-------------------------------------------------------------------------------------------------
Vector3 Cylinder::GetCenter() const
{
	return 0.5f * (m_bottom + m_top);
}


//-------------------------------------------------------------------------------------------------
void Cylinder::GetSupportPoint(const Vector3& direction, Vector3& out_point) const
{
	float bottomDot = DotProduct(direction, m_bottom);
	float topDot = DotProduct(direction, m_top);
	Vector3 endPoint = (bottomDot >= topDot ? m_bottom : m_top); // In tie cases, default to bottom
	Vector3 spineDir = (bottomDot >= topDot ? (m_bottom - m_top) : (m_top - m_bottom));

	spineDir.Normalize();

	// Project onto the spine vector
	float dot = DotProduct(spineDir, direction);

	// Get the projection of the endPointToPlane onto the disc of the cylinder
	Vector3 discVector = direction - spineDir * dot;

	if (AreMostlyEqual(discVector, Vector3::ZERO))
	{
		discVector = Vector3::ZERO;
	}
	else
	{
		discVector.SafeNormalize(Vector3::ZERO);
	}

	out_point = endPoint + discVector * m_radius;
}
