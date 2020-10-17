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
	: m_alignedBounds(AABB2(minX, minY, maxX, maxY))
	, m_orientationDegrees(initialDegrees)
{
}


//-------------------------------------------------------------------------------------------------
OBB2::OBB2(const Vector2& mins, const Vector2& maxs, float initialDegrees)
	: m_alignedBounds(AABB2(mins, maxs))
	, m_orientationDegrees(initialDegrees)
{
}


//-------------------------------------------------------------------------------------------------
OBB2::OBB2(const AABB2& initialBounds, float initialDegrees)
	: m_alignedBounds(AABB2(initialBounds))
	, m_orientationDegrees(initialDegrees)
{
}


//-------------------------------------------------------------------------------------------------
OBB2::OBB2(const AABB2& initialBounds)
	: m_alignedBounds(AABB2(initialBounds))
	, m_orientationDegrees(0.f)
{
}


//-------------------------------------------------------------------------------------------------
void OBB2::RotateAboutPoint(const Vector2& point, float angleDegrees)
{
	float c = CosDegrees(angleDegrees);
	float s = SinDegrees(angleDegrees);

	// First rotate about the point
	Vector2 bottomLeft	= m_alignedBounds.GetBottomLeft();
	Vector2 topLeft		= m_alignedBounds.GetTopLeft();
	Vector2 topRight	= m_alignedBounds.GetTopRight();
	Vector2 bottomRight = m_alignedBounds.GetBottomRight();

	bottomLeft	= RotatePointAboutPoint2D(bottomLeft, point, c, s);
	topLeft		= RotatePointAboutPoint2D(topLeft, point, c, s);
	topRight	= RotatePointAboutPoint2D(topRight, point, c, s);
	bottomRight	= RotatePointAboutPoint2D(bottomRight, point, c, s);

	// Then rotate backwards about the center so that the resulting positions form an aligned bounds again
	Vector2 center = 0.5f * (topRight - bottomLeft) + bottomLeft;
	c = CosDegrees(-angleDegrees);
	s = SinDegrees(-angleDegrees);

	m_alignedBounds.SetBottomLeft(RotatePointAboutPoint2D(bottomLeft, center, c, s));
	m_alignedBounds.SetTopLeft(RotatePointAboutPoint2D(topLeft, center, c, s));
	m_alignedBounds.SetTopRight(RotatePointAboutPoint2D(topRight, center, c, s));
	m_alignedBounds.SetBottomRight(RotatePointAboutPoint2D(bottomRight, center, c, s));

	if (m_alignedBounds.left > m_alignedBounds.right)
	{
		float temp = m_alignedBounds.left;
		m_alignedBounds.left = m_alignedBounds.right;
		m_alignedBounds.right = temp;
	}

	if (m_alignedBounds.bottom > m_alignedBounds.top)
	{
		float temp = m_alignedBounds.bottom;
		m_alignedBounds.bottom = m_alignedBounds.top;
		m_alignedBounds.top = temp;
	}

	// Resulting orientation is just the sum
	m_orientationDegrees += angleDegrees;
}


//-------------------------------------------------------------------------------------------------
void OBB2::GetPositions(Vector2* out_vertices) const
{
	// Assumes rotated about center
	float c = CosDegrees(m_orientationDegrees);
	float s = SinDegrees(m_orientationDegrees);

	Vector2 center = m_alignedBounds.GetCenter();
	Vector2 bottomLeft = m_alignedBounds.GetBottomLeft();
	Vector2 topLeft = m_alignedBounds.GetTopLeft();
	Vector2 topRight = m_alignedBounds.GetTopRight();
	Vector2 bottomRight = m_alignedBounds.GetBottomRight();

	out_vertices[0] = RotatePointAboutPoint2D(bottomLeft, center, c, s);
	out_vertices[1] = RotatePointAboutPoint2D(topLeft, center, c, s);
	out_vertices[2] = RotatePointAboutPoint2D(topRight, center, c, s);
	out_vertices[3] = RotatePointAboutPoint2D(bottomRight, center, c, s);
}


//-------------------------------------------------------------------------------------------------
bool OBB2::IsPointInside(const Vector2& point) const
{
	// Assumes rotation is about the box's center
	Vector2 center = m_alignedBounds.GetCenter();
	Vector2 toPoint = (point - center);
	float c = CosDegrees(m_orientationDegrees);
	float s = SinDegrees(m_orientationDegrees);

	Vector2 rotatedPoint;
	rotatedPoint.x = toPoint.x * c - toPoint.y * s;
	rotatedPoint.y = toPoint.x * s + toPoint.y * c;

	rotatedPoint += center;

	return (m_alignedBounds.IsPointInside(rotatedPoint));
}
