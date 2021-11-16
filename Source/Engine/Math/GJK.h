///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: Nov 12th, 2021
/// Description: File for all GJK computations
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class LineSegment2;
class LineSegment3;
class Polygon2;
class Polygon3;
class Polyhedron;
class Triangle2;
class Triangle3;
class Vector2;
class Vector3;

enum SimplexResult
{
	SIMPLEX_RESULT_NO_INTERSECTION,
	SIMPLEX_RESULT_INTERSECTION_FOUND,
	SIMPLEX_RESULT_STILL_EVOLVING
};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class GJK
{
public:
	//-----Public Methods-----

	static float PointToLineSegment(const Vector2& point, const LineSegment2& lineSegment, Vector2& out_closestPt);
	static float PointToLineSegment(const Vector3& point, const LineSegment3& lineSegment, Vector2& out_closestPt);
	static float PointToTriangle(const Vector2& point, const Triangle2& triangle, Vector2& out_closestPt);
	static float PointToTriangle(const Vector3& point, const Triangle3& triangle, Vector2& out_closestPt);
	static float PointToPolygon(const Vector2& point, const Polygon2& polygon, Vector2& out_closestPt);
	static float PointToPolygon(const Vector2& point, const Polygon3& polygon, Vector2& out_closestPt);
	static float PointToPolyhedron(const Vector3& point, const Polyhedron& polygon, Vector2& out_closestPt);

	static Vector2 ComputeLineSegmentBarycentricCoords(const Vector2& point, const LineSegment2& lineSegment);
	static Vector3 ComputeTriangleBarycentricCoords(const Vector2& point, const Triangle2& triangle);

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------