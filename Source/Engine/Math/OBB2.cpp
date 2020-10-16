///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 4th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/OBB2.h"

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
OBB2::OBB2(float minX, float minY, float maxX, float maxY, float initialDegrees)
	: alignedBounds(AABB2(minX, minY, maxX, maxY))
	, orientationDegrees(initialDegrees)
{
}


//-------------------------------------------------------------------------------------------------
OBB2::OBB2(const Vector2& mins, const Vector2& maxs, float initialDegrees)
	: alignedBounds(AABB2(mins, maxs))
	, orientationDegrees(initialDegrees)
{
}


//-------------------------------------------------------------------------------------------------
OBB2::OBB2(const AABB2& initialBounds, float initialDegrees)
	: alignedBounds(AABB2(initialBounds))
	, orientationDegrees(initialDegrees)
{
}


//-------------------------------------------------------------------------------------------------
OBB2::OBB2(const AABB2& initialBounds)
	: alignedBounds(AABB2(initialBounds))
	, orientationDegrees(0.f)
{
}


//-------------------------------------------------------------------------------------------------
void OBB2::GetCorners(Vector2* out_vertices) const
{
	// Assumes rotated about center
	float c = CosDegrees(orientationDegrees);
	float s = SinDegrees(orientationDegrees);

	Vector2 center = alignedBounds.GetCenter();
	Vector2 bottomLeft = alignedBounds.GetBottomLeft();
	Vector2 topLeft = alignedBounds.GetTopLeft();
	Vector2 topRight = alignedBounds.GetTopRight();
	Vector2 bottomRight = alignedBounds.GetBottomRight();

	Vector2 toBottomLeft = (bottomLeft - center);
	Vector2 toTopLeft = (topLeft - center);
	Vector2 toTopRight = (topRight - center);
	Vector2 toBottomRight = (bottomRight - center);

	out_vertices[0] = Vector2(toBottomLeft.x * c - toBottomLeft.y * s, toBottomLeft.x * s + toBottomLeft.y * c) + center;
	out_vertices[1] = Vector2(toTopLeft.x * c - toTopLeft.y * s, toTopLeft.x * s + toTopLeft.y * c) + center;
	out_vertices[2] = Vector2(toTopRight.x * c - toTopRight.y * s, toTopRight.x * s + toTopRight.y * c) + center;
	out_vertices[3] = Vector2(toBottomRight.x * c - toBottomRight.y * s, toBottomRight.x * s + toBottomRight.y * c) + center;
}


//-------------------------------------------------------------------------------------------------
bool OBB2::IsPointInside(const Vector2& point) const
{
	// Assumes rotation is about the box's center
	Vector2 center = alignedBounds.GetCenter();
	Vector2 toPoint = (point - center);
	float c = CosDegrees(orientationDegrees);
	float s = SinDegrees(orientationDegrees);

	Vector2 rotatedPoint;
	rotatedPoint.x = toPoint.x * c - toPoint.y * s;
	rotatedPoint.y = toPoint.x * s + toPoint.y * c;

	rotatedPoint += center;

	return (alignedBounds.IsPointInside(rotatedPoint));
}
