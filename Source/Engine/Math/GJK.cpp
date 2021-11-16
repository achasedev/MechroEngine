///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: Nov 16th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/Math/GJK.h"
#include "Engine/Math/LineSegment2.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/Triangle2.h"
#include "Engine/Math/Vector2.h"

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
float GJK::PointToLineSegment(const Vector2& point, const LineSegment2& lineSegment, Vector2& out_closestPt)
{
	Vector2 uvs = ComputeLineSegmentBarycentricCoords(point, lineSegment);
	Vector2 closestPt;

	if (uvs.u <= 0.f)
	{
		closestPt = lineSegment.m_b;
	}
	else if (uvs.v <= 0.f)
	{
		closestPt = lineSegment.m_a;
	}
	else
	{
		closestPt = uvs.u * lineSegment.m_a + uvs.v * lineSegment.m_b;
	}

	out_closestPt = closestPt;
	return (closestPt - point).GetLength();
}


//-------------------------------------------------------------------------------------------------
float GJK::PointToTriangle(const Vector2& point, const Triangle2& triangle, Vector2& out_closestPt)
{
	LineSegment2 ab(triangle.m_a, triangle.m_b);
	LineSegment2 bc(triangle.m_b, triangle.m_c);
	LineSegment2 ca(triangle.m_c, triangle.m_a);

	Vector2 abUVs = ComputeLineSegmentBarycentricCoords(point, ab);
	Vector2 bcUVs = ComputeLineSegmentBarycentricCoords(point, bc);
	Vector2 caUVs = ComputeLineSegmentBarycentricCoords(point, ca);

	Maybe<Vector2> closestPt;

	// Check endpoints
	if (caUVs.u <= 0.f && abUVs.v <= 0.f)
	{
		closestPt.Set(triangle.m_a);
	}
	else if (abUVs.u <= 0.f && bcUVs.v <= 0.f)
	{
		closestPt.Set(triangle.m_b);
	}
	else if (bcUVs.u <= 0.f && caUVs.v <= 0.f)
	{
		closestPt.Set(triangle.m_c);
	}

	// Check triangle edges
	if (!closestPt.IsValid())
	{
		Vector3 triUVW = ComputeTriangleBarycentricCoords(point, triangle);

		if (abUVs.u > 0.f && abUVs.v > 0.f && triUVW.w <= 0.f)
		{
			closestPt.Set(abUVs.u * triangle.m_a + abUVs.v * triangle.m_b);
		}
		else if (bcUVs.u > 0.f && bcUVs.v > 0.f && triUVW.u <= 0.f)
		{
			closestPt.Set(bcUVs.u * triangle.m_b + bcUVs.v * triangle.m_c);
		}
		else if (caUVs.u > 0.f && caUVs.v > 0.f && triUVW.v <= 0.f)
		{
			closestPt.Set(caUVs.u * triangle.m_c + caUVs.v * triangle.m_a);
		}
	}

	if (!closestPt.IsValid())
	{
		// Point should be in the triangle
		closestPt.Set(point);
	}

	out_closestPt = closestPt.Get();
	return (out_closestPt - point).GetLength();
}


//-------------------------------------------------------------------------------------------------
Vector2 GJK::ComputeLineSegmentBarycentricCoords(const Vector2& point, const LineSegment2& lineSegment)
{
	Vector2 dir = (lineSegment.m_b - lineSegment.m_a);
	float length = dir.Normalize();
	float invLength = (length > 0.f ? 1.f / length : 0.f);

	float u = DotProduct(lineSegment.m_b - point, dir) * invLength;
	float v = DotProduct(point - lineSegment.m_a, dir) * invLength;

	return Vector2(u, v);
}


//-------------------------------------------------------------------------------------------------
Vector3 GJK::ComputeTriangleBarycentricCoords(const Vector2& point, const Triangle2& triangle)
{
	Vector2 ab = triangle.m_b - triangle.m_a;
	Vector2 bc = triangle.m_c - triangle.m_b;
	Vector2 ca = triangle.m_a - triangle.m_c;

	float totalArea = 0.5f * CrossProduct(ab, bc);

	Vector2 bp = (point - triangle.m_b);
	float areaBCP = 0.5f * CrossProduct(bc, bp);
	float u = areaBCP / totalArea;

	Vector2 cp = (point - triangle.m_c);
	float areaCAP = 0.5f * CrossProduct(ca, cp);
	float v = areaCAP / totalArea;

	Vector2 ap = (point - triangle.m_a);
	float areaABP = 0.5f * CrossProduct(ab, ap);
	float w = areaABP / totalArea;

	return Vector3(u, v, w);
}
