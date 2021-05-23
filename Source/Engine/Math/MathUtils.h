///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 28th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/Math/AABB2.h"
#include "Engine/Math/IntVector2.h"
#include "Engine/Math/Line3.h"
#include "Engine/Math/Matrix4.h"
#include "Engine/Math/Plane3.h"
#include "Engine/Math/Polygon2D.h"
#include "Engine/Math/Polygon3d.h"
#include "Engine/Math/Quaternion.h"
#include "Engine/Math/Range.h"
#include "Engine/Math/Sphere3D.h"
#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector4.h"
#include <stdint.h>

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------
const float PI = 3.1415926535897932384626433832795f;
const float PI_OVER_TWO = 0.5f * PI;
const float DEFAULT_EPSILON = 0.0001f;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Math wrappers
int		Ceiling(float value);
int		Floor(float value);
float 	Log2(float value);
float 	Log10(float value);
float 	ModFloat(float x, float y);
float	Sqrt(float value);
float	Pow(float base, float exponent);
int		Abs(int inValue);
float	Abs(float inValue);
Vector2	Abs(const Vector2& inValue);
Vector3	Abs(const Vector3& inValue);


//-------------------------------------------------------------------------------------------------
// Scalar Functions
float Normalize(uint8 inValue);
uint8 NormalizedFloatToByte(float inValue);


//-------------------------------------------------------------------------------------------------
// Coordinate system
Vector2 PolarToCartesian(float radius, float angleRadians);
void	CartesianToPolar(float x, float y, float& out_radius, float& out_angleRadians);
void	CartesianToPolar(const Vector2& point, float& out_radius, float& out_angleRadians);
Vector3 SphericalToCartesian(float radius, float rotationDegrees, float azimuthDegrees);


//-------------------------------------------------------------------------------------------------
// Angles and Trigonometry
float	RadiansToDegrees(float radians);
Vector3	RadiansToDegrees(Vector3 radians);
float	DegreesToRadians(float degrees);
Vector3	DegreesToRadians(Vector3 eulerAngles);
float	CosDegrees(float degrees);
float   ACosDegrees(float ratio);
float	SinDegrees(float degrees);
float   ASinDegrees(float ratio);
float	TanDegrees(float degrees);
float	Atan2Degrees(float y, float x);
float	Atan2Degrees(float ratio);
float	GetNearestCardinalAngleDegrees(float angle);
float	GetNearestInterCardinalAngleDegrees(float angle);
Vector2	GetNearestCardinalDirection(const Vector2& direction);
Vector2 GetNearestCardinalDirection(float angle);
float	GetAngleBetweenMinusOneEightyAndOneEighty(float angleDegrees);
float	GetAngleBetweenZeroThreeSixty(float angleDegrees);
float	GetAngularDisplacement(float startDegrees, float endDegrees);
float	RotateToward(float currentDegrees, float goalDegrees, float maxTurnDegrees);


//-------------------------------------------------------------------------------------------------
// Random functions
float	GetRandomFloatInRange(float minInclusive, float maxInclusive);
int		GetRandomIntInRange(int minInclusive, int maxInclusive);
float	GetRandomFloatZeroToOne();
int		GetRandomIntLessThan(int maxExclusive);
bool	GetRandomBool();
bool	CheckRandomChance(float chanceForSuccess);
Vector2 GetRandomPointOnUnitCircle();
Vector2 GetRandomPointWithinUnitCircle();
Vector3 GetRandomPointOnUnitSphere();
Vector3 GetRandomPointWithinUnitSphere();


//-------------------------------------------------------------------------------------------------
// Rounding and clamping
int		RoundToNearestInt(float inValue);
Vector2	Clamp(const Vector2& inValue, const Vector2& minInclusive, const Vector2& maxInclusive);
Vector3 Clamp(const Vector3& inValue, float minInclusive, float maxInclusive);
Vector3 Clamp(const Vector3& inValue, const Vector3& minInclusive, const Vector3& maxInclusive);


//-------------------------------------------------------------------------------------------------
// Vector utilities
float	DotProduct(const Vector2& a, const Vector2& b);
float	DotProduct(const Vector3& a, const Vector3& b);
float	DotProduct(const Vector4& a, const Vector4& b);
float	DotProduct(const Quaternion& a, const Quaternion& b);
float	CrossProduct(const Vector2& a, const Vector2& b); // Performs a 3D cross product by using z = 0, and returns the z component of the result
Vector3 CrossProduct(const Vector3& a, const Vector3& b);
Vector3 Reflect(const Vector3& incidentVector, const Vector3& normal);
bool	Refract(const Vector3& incidentVector, const Vector3& normal, float niOverNt, Vector3& out_refractedVector); // Returns true if the given vector will refract across the surface, false otherwise
Vector2	RotateDirectionByEulerAngleDegrees(const Vector2& direction, float angleDegrees);
Vector3	RotateDirectionByEulerAnglesDegrees(const Vector3& direction, const Vector3& eulerAnglesDegrees);
Vector3 CalculateNormalForTriangle(const Vector3& a, const Vector3& b, const Vector3& c);


//-------------------------------------------------------------------------------------------------
// Bitflag utilities
bool		AreBitsSet(unsigned char bitFlags8, unsigned char flagsToCheck);
bool		AreBitsSet(unsigned int bitFlags32, unsigned int flagsToCheck);
void		SetBits(unsigned char& bitFlags8, unsigned char flagsToSet);
void		SetBits(unsigned int& bitFlags32, unsigned int flagsToSet);
void		ClearBits(unsigned char& bitFlags8, unsigned char flagsToClear);
void		ClearBits(unsigned int& bitFlags32, unsigned int flagsToClear);
uint32_t	GetBitsReversed(const uint32_t& bits);


//-------------------------------------------------------------------------------------------------
// Easing Functions
float	SmoothStart2(float t);		// 2nd-degree smooth start (a.k.a "quadratic ease in")
float	SmoothStart3(float t);		// 3rd-degree smooth start (a.k.a "cubic ease in")
float	SmoothStart4(float t);		// 4th-degree smooth start (a.k.a "quartic ease in")
float	SmoothStop2(float t);		// 2nd-degree smooth start (a.k.a "quadratic ease out")
float	SmoothStop3(float t);		// 3rd-degree smooth start (a.k.a "cubic ease out")
float	SmoothStop4(float t);		// 4th-degree smooth start (a.k.a "quartic ease out")
float	SmoothStep3(float t);		// 3nd-degree smooth start/stop (a.k.a "smoothstep")


//-------------------------------------------------------------------------------------------------
// Interpolation
float				Interpolate(float start, float end, float fractionTowardEnd);
int					Interpolate(int start, int end, float fractionTowardEnd);
unsigned char		Interpolate(unsigned char start, unsigned char end, float fractionTowardEnd);
const Vector2		Interpolate(const Vector2& start, const Vector2& end, float fractionTowardEnd);
const Vector3		Interpolate(const Vector3& start, const Vector3& end, float fractionTowardEnd);
const Vector4		Interpolate(const Vector4& start, const Vector4& end, float fractionTowardEnd);
const IntVector2	Interpolate(const IntVector2& start, const IntVector2& end, float fractionTowardEnd);
const Vector3		Interpolate(const Vector3& start, const Vector3& end, float fractionTowardEnd);
const AABB2			Interpolate(const AABB2& start, const AABB2& end, float fractionTowardEnd);		// Interpolates the mins/maxes of the boxes
const Matrix4		Interpolate(const Matrix4& start, const Matrix4& end, float fractionTowardEnd);
float				GetFractionInRange(float inValue, float rangeStart, float rangeEnd);
float				RangeMapFloat(float inValue, float inStart, float inEnd, float outStart, float outEnd);
Vector2				RangeMap(Vector2 inValue, Vector2 inStart, Vector2 inEnd, Vector2 outStart, Vector2 outEnd);


//-------------------------------------------------------------------------------------------------
// Polynomials
bool SolveQuadratic(Vector2& out_solutions, float a, float b, float c);


//-------------------------------------------------------------------------------------------------
// Float comparison function
bool AreMostlyEqual(float a, float b, float epsilon = DEFAULT_EPSILON);
bool AreMostlyEqual(const Vector2& a, const Vector2& b, float epsilon = DEFAULT_EPSILON);
bool AreMostlyEqual(const Vector3& a, const Vector3& b, float epsilon = DEFAULT_EPSILON);
bool AreMostlyEqual(const Matrix3& a, const Matrix3& b, float epsilon = DEFAULT_EPSILON);
bool AreMostlyEqual(const Quaternion& a, const Quaternion& b, float epsilon = DEFAULT_EPSILON);
bool AreMostlyEqual(const Sphere3D& a, const Sphere3D& b, float epsilon = DEFAULT_EPSILON);

bool IsReasonable(float value);
bool IsReasonable(const Vector2& value);
bool IsReasonable(const Vector3& value);
bool IsReasonable(const Matrix3& value);
bool IsReasonable(const Quaternion& value);
bool IsReasonable(const OBB3& value);


//-------------------------------------------------------------------------------------------------
// Geometry
bool	DoRangesOverlap(const Range& a, const Range& b);
bool	DoAABB2sOverlap(const AABB2& boxOne, const AABB2& boxTwo);
bool	DoAABB3sOverlap(const AABB3& a, const AABB3& b);
bool	DoSpheresOverlap(const Sphere3D& a, const Sphere3D& b);
Vector2 RotatePointAboutPoint2D(const Vector2& pointToRotate, const Vector2& pointOfRotation, float angleDegrees);
Vector2 RotatePointAboutPoint2D(const Vector2& pointToRotate, const Vector2& pointOfRotation, float cosAngle, float sinAngle);

float	CalculateVolumeOfTetrahedron(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d);
Vector3 SolveLinePlaneIntersection(const Line3& line, const Plane3& plane);
float	GetClosestPointOnLineSegment(const Vector3& start, const Vector3& end, const Vector3& point, Vector3& out_closestPoint);
float	FindClosestPointsOnLineSegments(const Vector3& startA, const Vector3& endA, const Vector3& startB, const Vector3& endB, Vector3& out_pointOnA, Vector3& out_pointOnB);

//-------------------------------------------------------------------------------------------------
// Templates
template <typename T>
T Max(const T& a, const T& b)
{
	return (a > b ? a : b);
}

template <typename T, typename ...ARGS>
T Max(const T&a, ARGS ...args)
{
	T max = Max<T>(args...);
	return Max<T>(a, max);
}

template <typename T>
T Min(const T& a, const T& b)
{
	return (a < b ? a : b);
}

template <typename T, typename ...ARGS>
T Min(const T&a, ARGS ...args)
{
	T min = Min<T>(args...);
	return Min<T>(a, min);
}

template <typename T>
T Clamp(const T& inValue, const T& minInclusive, const T& maxInclusive)
{
	if (inValue > maxInclusive)
	{
		return maxInclusive;
	}
	else if (inValue < minInclusive)
	{
		return minInclusive;
	}
	else
	{
		return inValue;
	}
}
