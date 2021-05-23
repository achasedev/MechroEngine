///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 28th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/Matrix3.h"
#include "Engine/Math/OBB3.h"
#include "Engine/Math/Vector2.h"
#include <math.h>
#include <cstdlib>

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

//-------------------------------------------------------------------------------------------------
int Ceiling(float value)
{
	return static_cast<int>(ceilf(value));
}


//-------------------------------------------------------------------------------------------------
int Floor(float value)
{
	return static_cast<int>(floorf(value));
}


//-------------------------------------------------------------------------------------------------
float Log2(float value)
{
	return log2f(value);
}


//-------------------------------------------------------------------------------------------------
float Log10(float value)
{
	return log10f(value);
}


//-------------------------------------------------------------------------------------------------
// Doesn't behave as expected for negatives!
float ModFloat(float x, float y)
{
	return fmodf(x, y);
}


//-------------------------------------------------------------------------------------------------
float Sqrt(float value)
{
	return sqrtf(value);
}


//-------------------------------------------------------------------------------------------------
float Pow(float base, float exponent)
{
	return powf(base, exponent);
}


//-------------------------------------------------------------------------------------------------
float Normalize(uint8 inValue)
{
	return static_cast<float>(inValue) * (1.f / 255.f);
}


//-------------------------------------------------------------------------------------------------
uint8 NormalizedFloatToByte(float inValue)
{
	float scaledValue = Clamp(inValue * 255.f, 0.f, 255.0f);
	return static_cast<uint8>(RoundToNearestInt(scaledValue));
}


//-------------------------------------------------------------------------------------------------
Vector2 PolarToCartesian(float radius, float angleRadians)
{
	Vector2 result;
	result.x = radius * cosf(angleRadians);
	result.y = radius * sinf(angleRadians);
	return result;
}


//-------------------------------------------------------------------------------------------------
void CartesianToPolar(float x, float y, float& out_radius, float& out_angleDegrees)
{
	out_radius = sqrtf((x * x) + (y * y));
	out_angleDegrees = RadiansToDegrees(atan2f(y, x));
}


//-------------------------------------------------------------------------------------------------
void CartesianToPolar(const Vector2& point, float& out_radius, float& out_angleDegrees)
{
	return CartesianToPolar(point.x, point.y, out_radius, out_angleDegrees);
}


//-------------------------------------------------------------------------------------------------
Vector3 SphericalToCartesian(float radius, float rotationDegrees, float azimuthDegrees)
{
	Vector3 cartesianCoordinate;

	cartesianCoordinate.x = radius * CosDegrees(rotationDegrees) * SinDegrees(azimuthDegrees);
	cartesianCoordinate.z = radius * SinDegrees(rotationDegrees) * SinDegrees(azimuthDegrees);
	cartesianCoordinate.y = radius * CosDegrees(azimuthDegrees);

	return cartesianCoordinate;
}


//-------------------------------------------------------------------------------------------------
float RadiansToDegrees(float radians)
{
	return radians * (180.f / PI);
}


//-------------------------------------------------------------------------------------------------
Vector3 RadiansToDegrees(Vector3 radians)
{
	return Vector3(RadiansToDegrees(radians.x), RadiansToDegrees(radians.y), RadiansToDegrees(radians.z));
}


//-------------------------------------------------------------------------------------------------
float DegreesToRadians(float degrees)
{
	return degrees * (PI / 180.f);
}


//-------------------------------------------------------------------------------------------------
Vector3 DegreesToRadians(Vector3 eulerAngles)
{
	return Vector3(DegreesToRadians(eulerAngles.x), DegreesToRadians(eulerAngles.y), DegreesToRadians(eulerAngles.z));
}


//-------------------------------------------------------------------------------------------------
float CosDegrees(float degrees)
{
	float radians = DegreesToRadians(degrees);

	return cosf(radians);
}


//-------------------------------------------------------------------------------------------------
float ACosDegrees(float ratio)
{
	Clamp(ratio, -1.f, 1.f);
	float radians = acosf(ratio);

	return RadiansToDegrees(radians);
}


//-------------------------------------------------------------------------------------------------
float SinDegrees(float degrees)
{
	float radians = DegreesToRadians(degrees);

	return sinf(radians);
}


//-------------------------------------------------------------------------------------------------
float ASinDegrees(float ratio)
{
	ratio = Clamp(ratio, -1.f, 1.f);
	float radians = asinf(ratio);

	return RadiansToDegrees(radians);
}


//-------------------------------------------------------------------------------------------------
float TanDegrees(float degrees)
{
	float radians = DegreesToRadians(degrees);

	return tanf(radians);
}


//-------------------------------------------------------------------------------------------------
float Atan2Degrees(float y, float x)
{
	float radians = atan2f(y, x);

	return RadiansToDegrees(radians);
}


//-------------------------------------------------------------------------------------------------
float Atan2Degrees(float ratio)
{
	float radians = atan2f(ratio, 1.f);

	return RadiansToDegrees(radians);
}


//-------------------------------------------------------------------------------------------------
float GetNearestCardinalAngleDegrees(float angleDegrees)
{
	float eastDistance = abs(GetAngularDisplacement(angleDegrees, 0.f));
	float northDistance	= abs(GetAngularDisplacement(angleDegrees, 90.f));
	float westDistance	= abs(GetAngularDisplacement(angleDegrees, 180.f));
	float southDistance	= abs(GetAngularDisplacement(angleDegrees, 270.f));

	float minDistance = Min(eastDistance, northDistance, westDistance, southDistance);

	// Return the direction corresponding to the min distance
	if		(minDistance == eastDistance)	{ return 0.f; }
	else if (minDistance == westDistance)	{ return 180.f; }
	else if (minDistance == northDistance)	{ return 90.f; }
	else									{ return 270.f; }
}


//-------------------------------------------------------------------------------------------------
float GetNearestInterCardinalAngleDegrees(float angleDegrees)
{
	float neDistance = abs(GetAngularDisplacement(angleDegrees, 45.f));
	float nwDistance = abs(GetAngularDisplacement(angleDegrees, 135.f));
	float swDistance = abs(GetAngularDisplacement(angleDegrees, 225.f));
	float seDistance = abs(GetAngularDisplacement(angleDegrees, 315.f));

	float minDistance = Min(neDistance, nwDistance, swDistance, seDistance);

	// Return the direction corresponding to the min distance
	if		(minDistance == neDistance) { return 45.f; }
	else if (minDistance == nwDistance) { return 135.f; }
	else if (minDistance == swDistance) { return 225.f; }
	else								{ return 315.f; }
}


//-------------------------------------------------------------------------------------------------
Vector2 GetNearestCardinalDirection(const Vector2& direction)
{
	float eastDot	= DotProduct(direction, Vector2::X_AXIS);
	float northDot	= DotProduct(direction, Vector2::Y_AXIS);
	float westDot	= DotProduct(direction, Vector2::MINUS_X_AXIS);
	float southDot	= DotProduct(direction, Vector2::MINUS_Y_AXIS);

	float maxDot = Max(northDot, southDot, eastDot, westDot);

	if		(maxDot == northDot)	{ return Vector2::Y_AXIS; }
	else if (maxDot == southDot)	{ return Vector2::MINUS_Y_AXIS; }
	else if (maxDot == eastDot)		{ return Vector2::X_AXIS; }
	else							{ return Vector2::MINUS_X_AXIS; }
}


//-------------------------------------------------------------------------------------------------
Vector2 GetNearestCardinalDirection(float angleDegrees)
{
	Vector2 direction = Vector2::MakeDirectionAtDegrees(angleDegrees);
	return GetNearestCardinalDirection(direction);
}


//-------------------------------------------------------------------------------------------------
float GetAngleBetweenMinusOneEightyAndOneEighty(float angleDegrees)
{
	while (angleDegrees > 180.f)
	{
		angleDegrees -= 360.f;
	}

	while (angleDegrees < -180.f)
	{
		angleDegrees += 360.f;
	}

	return angleDegrees;
}


//-------------------------------------------------------------------------------------------------
// Returns angle between 0.f (inclusive) and 360.f (exclusive)
float GetAngleBetweenZeroThreeSixty(float angleDegrees)
{
	while (angleDegrees >= 360.f)
	{
		angleDegrees -= 360.f;
	}

	while (angleDegrees < 0.f)
	{
		angleDegrees += 360.f;
	}

	return angleDegrees;
}


//-------------------------------------------------------------------------------------------------
float GetRandomFloatZeroToOne()
{
	return static_cast<float>(rand()) * (1.f / static_cast<float>(RAND_MAX));
}


//-------------------------------------------------------------------------------------------------
float GetRandomFloatInRange(float minInclusive, float maxInclusive)
{
	float ratio = GetRandomFloatZeroToOne();
	return Interpolate(minInclusive, maxInclusive, ratio);
}


//-------------------------------------------------------------------------------------------------
int GetRandomIntLessThan(int maxExclusive)
{
	return rand() % maxExclusive;
}


//-------------------------------------------------------------------------------------------------
int GetRandomIntInRange(int minInclusive, int maxInclusive)
{
	return (rand() % (maxInclusive - minInclusive + 1)) + minInclusive;
}


//-------------------------------------------------------------------------------------------------
bool GetRandomBool()
{
	return ((rand() % 2) == 0);
}


//-------------------------------------------------------------------------------------------------
bool CheckRandomChance(float chanceForSuccess)
{
	if (chanceForSuccess >= 1.f)
	{
		return true;
	}
	else if (chanceForSuccess <= 0.f)
	{
		return false;
	}
	else
	{
		float outcome = GetRandomFloatZeroToOne();
		return (outcome <= chanceForSuccess);
	}
}


//-------------------------------------------------------------------------------------------------
Vector2 GetRandomPointOnUnitCircle()
{
	float theta = GetRandomFloatInRange(0.f, 360.f);
	float x = CosDegrees(theta);
	float y = SinDegrees(theta);

	return Vector2(x, y);
}


//-------------------------------------------------------------------------------------------------
Vector2 GetRandomPointWithinUnitCircle()
{
	Vector2 boundaryVector = GetRandomPointOnUnitCircle();
	float randomRadius = GetRandomFloatInRange(0.01f, 0.99f);

	return boundaryVector * randomRadius;
}


//-------------------------------------------------------------------------------------------------
Vector3 GetRandomPointOnUnitSphere()
{
	float theta = GetRandomFloatInRange(0.f, 360.f);
	float phi = GetRandomFloatInRange(0.f, 360.f);

	return SphericalToCartesian(1.0f, theta, phi);
}


//-------------------------------------------------------------------------------------------------
Vector3 GetRandomPointWithinUnitSphere()
{
	Vector3 boundaryVector = GetRandomPointOnUnitSphere();
	float randomMagnitude = GetRandomFloatInRange(0.01f, 0.99f);

	return randomMagnitude * boundaryVector;
}


//-------------------------------------------------------------------------------------------------
int RoundToNearestInt(float inValue)
{
	int castedInt = static_cast<int>(inValue);
	float fraction = (inValue - static_cast<float>(castedInt));

	int result = castedInt;

	if (fraction >= 0.5)
	{
		result = castedInt + 1;
	}

	if (fraction < -0.5f)
	{
		result = castedInt - 1;
	}

	return result;
}


//-------------------------------------------------------------------------------------------------
uint32 RoundToNearestUInt(float inValue)
{
	uint32 result = 0;

	// If the float is less than zero.....just return zero
	if (inValue > 0.f)
	{
		uint32 castedUInt = static_cast<uint32>(inValue);
		float fraction = (inValue - static_cast<float>(castedUInt));
		result = (fraction >= 0.5f ? castedUInt + 1 : castedUInt);
	}

	return result;
}


//-------------------------------------------------------------------------------------------------
Vector2 Clamp(const Vector2& inValue, const Vector2& minInclusive, const Vector2& maxInclusive)
{
	Vector2 result;
	result.x = Clamp(inValue.x, minInclusive.x, maxInclusive.x);
	result.y = Clamp(inValue.y, minInclusive.y, maxInclusive.y);

	return result;
}


//-------------------------------------------------------------------------------------------------
Vector3 Clamp(const Vector3& inValue, float minInclusive, float maxInclusive)
{
	Vector3 result;
	result.x = Clamp(inValue.x, minInclusive, maxInclusive);
	result.y = Clamp(inValue.y, minInclusive, maxInclusive);
	result.z = Clamp(inValue.z, minInclusive, maxInclusive);

	return result;
}


//-------------------------------------------------------------------------------------------------
Vector3 Clamp(const Vector3& inValue, const Vector3& minInclusive, const Vector3& maxInclusive)
{
	Vector3 result;

	result.x = Clamp(inValue.x, minInclusive.x, maxInclusive.x);
	result.y = Clamp(inValue.y, minInclusive.y, maxInclusive.y);
	result.z = Clamp(inValue.z, minInclusive.z, maxInclusive.z);

	return result;
}


//-------------------------------------------------------------------------------------------------
float GetFractionInRange(float inValue, float rangeStart, float rangeEnd)
{
	float offsetIntoRange = (inValue - rangeStart);
	float rangeSize = (rangeEnd - rangeStart);

	return (offsetIntoRange / rangeSize);
}


//-------------------------------------------------------------------------------------------------
float RangeMapFloat(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	// If in range has size 0, just return the average of the out range
	if (inStart == inEnd)
	{
		return (outStart + outEnd) * 0.5f;
	}

	float fractionIntoInRange = GetFractionInRange(inValue, inStart, inEnd);
	float outRangeSize = outEnd - outStart;
	float amountIntoOutRange = fractionIntoInRange * outRangeSize;

	return amountIntoOutRange + outStart;
}


//-------------------------------------------------------------------------------------------------
Vector2 RangeMap(Vector2 inValue, Vector2 inStart, Vector2 inEnd, Vector2 outStart, Vector2 outEnd)
{
	float x = RangeMapFloat(inValue.x, inStart.x, inEnd.x, outStart.x, outEnd.x);
	float y = RangeMapFloat(inValue.y, inStart.y, inEnd.y, outStart.y, outEnd.y);

	return Vector2(x, y);
}


//-------------------------------------------------------------------------------------------------
float GetAngularDisplacement(float startDegrees, float endDegrees)
{
	float angularDisp = (endDegrees - startDegrees);

	// Increment/decrement the displacement to represent the shorter turn direction
	while (angularDisp > 180.f)
	{
		angularDisp -= 360.f;
	}

	while (angularDisp < -180.f)
	{
		angularDisp += 360.f;
	}

	return angularDisp;
}


//-------------------------------------------------------------------------------------------------
// Returns an angle that is at most maxTurnDegrees from currentDegrees towards goalDegrees, in
// the direction of the shortest path.
//
float RotateToward(float currentDegrees, float goalDegrees, float maxTurnDegrees)
{
	float angularDisplacement = GetAngularDisplacement(currentDegrees, goalDegrees);

	if (abs(angularDisplacement) <= maxTurnDegrees)
	{
		return goalDegrees;
	}

	float directionToTurn = angularDisplacement > 0.f ? 1.0f : -1.0f;
	float result = ((directionToTurn * maxTurnDegrees) + currentDegrees);

	return result;
}


//-------------------------------------------------------------------------------------------------
float DotProduct(const Vector2& a, const Vector2& b)
{
	return ((a.x * b.x) + (a.y * b.y));
}


//-------------------------------------------------------------------------------------------------
float DotProduct(const Vector3& a, const Vector3& b)
{
	return ((a.x * b.x) + (a.y * b.y) + (a.z * b.z));
}


//-------------------------------------------------------------------------------------------------
float DotProduct(const Vector4& a, const Vector4& b)
{
	return ((a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w));
}


//-------------------------------------------------------------------------------------------------
float DotProduct(const Quaternion& a, const Quaternion& b)
{
	return (a.real * b.real) + (a.v.x * b.v.x) + (a.v.y * b.v.y) + (a.v.z * b.v.z);
}


//-------------------------------------------------------------------------------------------------
Vector3 CrossProduct(const Vector3& a, const Vector3& b)
{
	Vector3 result;

	result.x = (a.y * b.z) - (a.z * b.y);
	result.y = (a.z * b.x) - (a.x * b.z);
	result.z = (a.x * b.y) - (a.y * b.x);

	return result;
}


//-------------------------------------------------------------------------------------------------
float CrossProduct(const Vector2& a, const Vector2& b)
{
	// Equivalent to CrossProduct(Vector3(a, 0.f), Vector3(b, 0.f));
	return a.x * b.y - b.x * a.y;
}


//-------------------------------------------------------------------------------------------------
Vector3 Reflect(const Vector3& incidentVector, const Vector3& normal)
{
	Vector3 alongNormal = DotProduct(incidentVector, normal) * normal;
	return incidentVector - 2.f * alongNormal;
}


//-------------------------------------------------------------------------------------------------
// Attempts to refract the incident vector through the surface defined by normal, returning it in out_refracedVector
// Returns true if the vector was refracted, false otherwise (See Snell's Law)
//
bool Refract(const Vector3& incidentVector, const Vector3& normal, float niOverNt, Vector3& out_refractedVector)
{
	Vector3 normalizedIncident = incidentVector.GetNormalized();

	float dt = DotProduct(normalizedIncident, normal);
	float discriminant = 1.0f - niOverNt * niOverNt * (1.0f - dt * dt);

	if (discriminant > 0) // Can be refracted
	{
		out_refractedVector = niOverNt * (normalizedIncident - normal * dt) - normal * sqrtf(discriminant);
		return true;
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
Vector2 RotateDirectionByEulerAngleDegrees(const Vector2& direction, float deltaAngleDegrees)
{
	float simplifiedAngle = GetAngleBetweenZeroThreeSixty(deltaAngleDegrees);

	// Optimizations
	if (simplifiedAngle == 0.f)
	{
		return direction;
	}

	if (simplifiedAngle == 90.f)
	{
		return Vector2(-direction.y, direction.x);
	}

	if (simplifiedAngle == 180.f)
	{
		return -1.0f * direction;
	}

	if (simplifiedAngle == 270.f)
	{
		return Vector2(direction.y, -direction.x);
	}

	// Do the actual math
	float cs = CosDegrees(simplifiedAngle);
	float sn = SinDegrees(simplifiedAngle);

	Vector2 result;
	result.x = direction.x * cs - direction.y * sn;
	result.y = direction.x * sn + direction.y * cs;

	return result;
}


//-------------------------------------------------------------------------------------------------
Vector3 RotateDirectionByEulerAnglesDegrees(const Vector3& direction, const Vector3& eulerAnglesDegrees)
{
	Matrix4 rotationMat = Matrix4::MakeRotationFromEulerAnglesDegrees(eulerAnglesDegrees);
	return rotationMat.TransformDirection(direction);
}


//-------------------------------------------------------------------------------------------------
Vector3 CalculateNormalForTriangle(const Vector3& a, const Vector3& b, const Vector3& c)
{
	Vector3 ab = b - a;
	Vector3 ac = c - a;

	Vector3 cross = CrossProduct(ab, ac);
	cross.Normalize();

	return cross;
}


//-------------------------------------------------------------------------------------------------
bool AreBitsSet(unsigned char bitFlags8, unsigned char flagsToCheck)
{
	return ((bitFlags8 & flagsToCheck) == flagsToCheck);
}


//-------------------------------------------------------------------------------------------------
bool AreBitsSet(unsigned int bitFlags32, unsigned int flagsToCheck)
{
	return ((bitFlags32 & flagsToCheck) == flagsToCheck);
}


//-------------------------------------------------------------------------------------------------
void SetBits(unsigned char& bitFlags8, unsigned char flagsToSet)
{
	bitFlags8 |= flagsToSet;
}


//-------------------------------------------------------------------------------------------------
void SetBits(unsigned int& bitFlags32, unsigned int flagsToSet)
{
	bitFlags32 |= flagsToSet;
}


//-------------------------------------------------------------------------------------------------
void ClearBits(unsigned char& bitFlags8, unsigned char flagsToClear)
{
	unsigned char bitMask = ~flagsToClear;
	bitFlags8 &= bitMask;
}


//-------------------------------------------------------------------------------------------------
void ClearBits(unsigned int& bitFlags32, unsigned int flagsToClear)
{
	unsigned int bitMask = ~flagsToClear;
	bitFlags32 &= bitMask;
}


//-------------------------------------------------------------------------------------------------
// Returns the int given with the bits reversed (i.e. 0010010 -> 0100100)
//
uint32_t GetBitsReversed(const uint32_t& bits)
{
	uint32_t reversedBits = 0;

	for (int i = 0; i < 32; ++i)
	{
		uint32_t currBit = bits & (1 << i);

		if (currBit != 0)
		{
			int inverseI = 32 - i - 1;
			reversedBits |= (1 << inverseI);
		}
	}

	return reversedBits;
}


//-------------------------------------------------------------------------------------------------
// 2nd-degree smooth start (a.k.a "quadratic ease in")
//
float SmoothStart2(float t)
{
	return (t * t);
}


//-------------------------------------------------------------------------------------------------
// 3rd-degree smooth start (a.k.a "cubic ease in")
//
float SmoothStart3(float t)
{
	return (t * t * t);
}


//-------------------------------------------------------------------------------------------------
// 4th-degree smooth start (a.k.a "quartic ease in")
//
float SmoothStart4(float t)
{
	return (t * t * t * t);
}


//-------------------------------------------------------------------------------------------------
// 2nd-degree smooth start (a.k.a "quadratic ease out")
//
float SmoothStop2(float t)
{
	float flipped = (1 - t);
	float squaredFlipped = (flipped * flipped);
	float flippedSquaredFlipped = (1 - squaredFlipped);

	return flippedSquaredFlipped;
}


//-------------------------------------------------------------------------------------------------
// 3rd-degree smooth start (a.k.a "cubic ease out")
//
float SmoothStop3(float t)
{
	float flipped = (1 - t);
	float cubedFlipped = (flipped * flipped * flipped);
	float flippedCubedFlipped = (1 - cubedFlipped);

	return flippedCubedFlipped;
}


//-------------------------------------------------------------------------------------------------
// 4th-degree smooth start (a.k.a "quartic ease out")
//
float SmoothStop4(float t)
{
	float flipped = (1 - t);
	float quartedFlipped = (flipped * flipped * flipped * flipped);
	float flippedQuartedFlipped = (1 - quartedFlipped);

	return flippedQuartedFlipped;
}


//-------------------------------------------------------------------------------------------------
// 3rd-degree smooth start/stop (a.k.a "smoothstep")
//
float SmoothStep3(float t)
{
	return ((1 - t) * SmoothStart2(t)) + (t * SmoothStop2(t));
}


//-------------------------------------------------------------------------------------------------
float Interpolate(float start, float end, float fractionTowardEnd)
{
	float rangeSize = (end - start);
	float offsetIntoRange = (fractionTowardEnd * rangeSize);
	return (offsetIntoRange + start);
}


//-------------------------------------------------------------------------------------------------
int Interpolate(int start, int end, float fractionTowardEnd)
{
	float range = static_cast<float>(end - start);
	return start + RoundToNearestInt(fractionTowardEnd * range);
}


//-------------------------------------------------------------------------------------------------
unsigned char Interpolate(unsigned char start, unsigned char end, float fractionTowardEnd)
{
	float range = static_cast<float>(end - start);

	int change = RoundToNearestInt(fractionTowardEnd * range);

	// Do the math as an int to allow clamping overflow to 255
	int result = static_cast<int>(start) + change;
	result = Clamp(result, 0, 255);

	return static_cast<unsigned char>(result);
}


//-------------------------------------------------------------------------------------------------
const Vector2 Interpolate(const Vector2& start, const Vector2& end, float fractionTowardEnd)
{
	float interpolatedX = Interpolate(start.x, end.x, fractionTowardEnd);
	float interpolatedY = Interpolate(start.y, end.y, fractionTowardEnd);

	return Vector2(interpolatedX, interpolatedY);
}


//-------------------------------------------------------------------------------------------------
const Vector3 Interpolate(const Vector3& start, const Vector3& end, float fractionTowardEnd)
{
	float interpolatedX = Interpolate(start.x, end.x, fractionTowardEnd);
	float interpolatedY = Interpolate(start.y, end.y, fractionTowardEnd);
	float interpolatedZ = Interpolate(start.z, end.z, fractionTowardEnd);

	return Vector3(interpolatedX, interpolatedY, interpolatedZ);
}


//-------------------------------------------------------------------------------------------------
const Vector4 Interpolate(const Vector4& start, const Vector4& end, float fractionTowardEnd)
{
	float interpolatedX = Interpolate(start.x, end.x, fractionTowardEnd);
	float interpolatedY = Interpolate(start.y, end.y, fractionTowardEnd);
	float interpolatedZ = Interpolate(start.z, end.z, fractionTowardEnd);
	float interpolatedW = Interpolate(start.w, end.w, fractionTowardEnd);

	return Vector4(interpolatedX, interpolatedY, interpolatedZ, interpolatedW);
}


//-------------------------------------------------------------------------------------------------
const IntVector2 Interpolate(const IntVector2& start, const IntVector2& end, float fractionTowardEnd)
{
	int interpolatedX = Interpolate(start.x, end.x, fractionTowardEnd);
	int interpolatedY = Interpolate(start.y, end.y, fractionTowardEnd);

	return IntVector2(interpolatedX, interpolatedY);
}


//-------------------------------------------------------------------------------------------------
const AABB2 Interpolate(const AABB2& start, const AABB2& end, float fractionTowardEnd)
{
	Vector2 interpolatedMins = Interpolate(start.mins, end.mins, fractionTowardEnd);
	Vector2 interpolatedMaxs = Interpolate(start.maxs, end.maxs, fractionTowardEnd);

	return AABB2(interpolatedMins, interpolatedMaxs);
}


//-------------------------------------------------------------------------------------------------
const Matrix4 Interpolate(const Matrix4& start, const Matrix4& end, float fractionTowardEnd)
{
	Vector4 startI = start.GetIVector();
	Vector4 endI = end.GetIVector();

	Vector4 startJ = start.GetJVector();
	Vector4 endJ = end.GetJVector();

	Vector4 startK = start.GetKVector();
	Vector4 endK = end.GetKVector();

	Vector4 startT = start.GetTVector();
	Vector4 endT = end.GetTVector();

	Vector4 resultI = Interpolate(startI, endI, fractionTowardEnd);
	Vector4 resultJ = Interpolate(startJ, endJ, fractionTowardEnd);
	Vector4 resultK = Interpolate(startK, endK, fractionTowardEnd);
	Vector4 resultT = Interpolate(startT, endT, fractionTowardEnd);

	return Matrix4(resultI, resultJ, resultK, resultT);
}


//-------------------------------------------------------------------------------------------------
// Finds the roots of the quadratic function given by the coefficients a, b, and c, and stores them
// in solutions
// Returns true if roots were found, false otherwise
//
bool SolveQuadratic(Vector2& out_solutions, float a, float b, float c)
{
	// (-b +- sqrt(b^2 - 4ac)) / (2a)

	// First determine the inside of the square root - if it is negative, then there are no real solutions
	float discriminant = (b * b) - (4 * a * c);

	if (discriminant < 0)
	{
		return false;
	}

	// There is at least one solution
	float sqrtValue = sqrtf(discriminant);

	float firstSolution = (-b + sqrtValue) / (2 * a);
	float secondSolution = (-b - sqrtValue) / (2 * a);

	// Order the solutions in order of magnitude
	out_solutions.x = Min(firstSolution, secondSolution);
	out_solutions.y = Max(firstSolution, secondSolution);

	return true;
}


//-------------------------------------------------------------------------------------------------
int Abs(int inValue)
{
	return abs(inValue);
}


//-------------------------------------------------------------------------------------------------
float Abs(float inValue)
{
	return abs(inValue);
}


//-------------------------------------------------------------------------------------------------
Vector2 Abs(const Vector2& inValue)
{
	Vector2 result;
	result.x = abs(inValue.x);
	result.y = abs(inValue.y);

	return result;
}


//-------------------------------------------------------------------------------------------------
Vector3 Abs(const Vector3& inValue)
{
	Vector3 result;
	result.x = abs(inValue.x);
	result.y = abs(inValue.y);
	result.z = abs(inValue.z);

	return result;
}


//-------------------------------------------------------------------------------------------------
bool AreMostlyEqual(float a, float b, float epsilon /*= DEFAULT_EPSILON*/)
{
	float diff = b - a;
	return (abs(diff) <= epsilon);
}


//-------------------------------------------------------------------------------------------------
bool AreMostlyEqual(const Vector2& a, const Vector2& b, float epsilon /*= DEFAULT_EPSILON*/)
{
	Vector2 difference = (b - a);
	bool mostlyEqual = (abs(difference.x) <= epsilon) && (abs(difference.y) <= epsilon);

	return mostlyEqual;
}


//-------------------------------------------------------------------------------------------------
bool AreMostlyEqual(const Vector3& a, const Vector3& b, float epsilon /*= DEFAULT_EPSILON*/)
{
	Vector3 difference = (b - a);
	bool mostlyEqual = (abs(difference.x) <= epsilon) && (abs(difference.y) <= epsilon) && (abs(difference.z) <= epsilon);

	return mostlyEqual;
}


//-------------------------------------------------------------------------------------------------
bool AreMostlyEqual(const Quaternion& a, const Quaternion& b, float epsilon /*= DEFAULT_EPSILON*/)
{
	float angleBetween = Quaternion::GetAngleBetweenDegrees(a, b);
	return (angleBetween <= epsilon);
}


//-------------------------------------------------------------------------------------------------
bool AreMostlyEqual(const Matrix3& a, const Matrix3& b, float epsilon /*= DEFAULT_EPSILON*/)
{
	return
		AreMostlyEqual(a.Ix, b.Ix, epsilon) &&
		AreMostlyEqual(a.Iy, b.Iy, epsilon) &&
		AreMostlyEqual(a.Iz, b.Iz, epsilon) &&
		AreMostlyEqual(a.Jx, b.Jx, epsilon) &&
		AreMostlyEqual(a.Jy, b.Jy, epsilon) &&
		AreMostlyEqual(a.Jz, b.Jz, epsilon) &&
		AreMostlyEqual(a.Kx, b.Kx, epsilon) &&
		AreMostlyEqual(a.Ky, b.Ky, epsilon) &&
		AreMostlyEqual(a.Kz, b.Kz, epsilon);
}


//-------------------------------------------------------------------------------------------------
bool AreMostlyEqual(const Sphere3D& a, const Sphere3D& b, float epsilon /*= DEFAULT_EPSILON*/)
{
	return AreMostlyEqual(a.center, b.center, epsilon) && AreMostlyEqual(a.radius, b.radius, epsilon);
}


//-------------------------------------------------------------------------------------------------
bool IsReasonable(float value)
{
	return !isinf(value) && !isnan(value);
}


//-------------------------------------------------------------------------------------------------
bool IsReasonable(const Vector3& value)
{
	return IsReasonable(value.x) && IsReasonable(value.y) && IsReasonable(value.z);
}


//-------------------------------------------------------------------------------------------------
bool IsReasonable(const Matrix3& value)
{
	for (int i = 0; i < 9; ++i)
	{
		if (!IsReasonable(value.data[i]))
		{
			return false;
		}
	}

	return true;
}


//-------------------------------------------------------------------------------------------------
bool IsReasonable(const OBB3& value)
{
	return IsReasonable(value.center) && IsReasonable(value.extents) && IsReasonable(value.rotation);
}


//-------------------------------------------------------------------------------------------------
bool IsReasonable(const Quaternion& value)
{
	return IsReasonable(value.w) && IsReasonable(value.x) && IsReasonable(value.y) && IsReasonable(value.z) && AreMostlyEqual(value.GetMagnitude(), 1.0f); // Also make sure it's normalized
}


//-------------------------------------------------------------------------------------------------
bool DoRangesOverlap(const Range& a, const Range& b)
{
	return a.GetOverlap(b) > 0.f;
}


//-------------------------------------------------------------------------------------------------
bool DoAABB2sOverlap(const AABB2& a, const AABB2& b)
{
	bool doOverlap = true;

	if (a.maxs.x <= b.mins.x) // a is completely to the left of b
	{
		doOverlap = false;
	}
	else if (a.mins.x >= b.maxs.x) // a is completely to the right of b
	{
		doOverlap = false;
	}
	else if (a.mins.y >= b.maxs.y) // a is completely above b
	{
		doOverlap = false;
	}
	else if (a.maxs.y <= b.mins.y) // a is completely below b
	{
		doOverlap = false;
	}

	return doOverlap;
}


//-------------------------------------------------------------------------------------------------
bool DoAABB3sOverlap(const AABB3& a, const AABB3& b)
{
	bool doOverlap = true;

	if (a.maxs.x <= b.mins.x) // a is completely to the left of b
	{
		doOverlap = false;
	}
	else if (a.mins.x >= b.maxs.x) // a is completely to the right of b
	{
		doOverlap = false;
	}
	else if (a.mins.y >= b.maxs.y) // a is completely above b
	{
		doOverlap = false;
	}
	else if (a.maxs.y <= b.mins.y) // a is completely below b
	{
		doOverlap = false;
	}
	else if (a.mins.z >= b.maxs.z) // a is completely in front b
	{
		doOverlap = false;
	}
	else if (a.maxs.z <= b.mins.z) // a is completely behind b
	{
		doOverlap = false;
	}

	return doOverlap;
}


//-------------------------------------------------------------------------------------------------
bool DoSpheresOverlap(const Sphere3D& a, const Sphere3D& b)
{
	float radiusSquared = (a.radius + b.radius) * (a.radius + b.radius);
	float distanceSquared = (a.center - b.center).GetLengthSquared();

	return distanceSquared < radiusSquared;
}


//-------------------------------------------------------------------------------------------------
Vector2 RotatePointAboutPoint2D(const Vector2& pointToRotate, const Vector2& pointOfRotation, float angleDegrees)
{
	float c = CosDegrees(angleDegrees);
	float s = SinDegrees(angleDegrees);

	return RotatePointAboutPoint2D(pointToRotate, pointOfRotation, c, s);
}


//-------------------------------------------------------------------------------------------------
Vector2 RotatePointAboutPoint2D(const Vector2& pointToRotate, const Vector2& pointOfRotation, float cosAngle, float sinAngle)
{
	Vector2 toPoint = (pointToRotate - pointOfRotation);
	Vector2 rotatedPoint = Vector2(toPoint.x * cosAngle - toPoint.y * sinAngle, toPoint.x * sinAngle + toPoint.y * cosAngle);
	
	return rotatedPoint + pointOfRotation;
}


//-------------------------------------------------------------------------------------------------
float CalculateVolumeOfTetrahedron(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d)
{
	return (1.f / 6.f) * Abs(DotProduct(CrossProduct(b - a, c - a), d - a));
}


//-------------------------------------------------------------------------------------------------
Vector3 SolveLinePlaneIntersection(const Line3& line, const Plane3& plane)
{
	const Vector3 p = line.GetPoint();
	const Vector3 dir = line.GetDirection();
	const Vector3 n = plane.GetNormal();
	const float d = plane.GetDistance();

	// Check for no solution
	float dot = DotProduct(dir, n);

	if (AreMostlyEqual(dot, 0.f))
	{
		if (plane.ContainsPoint(p))
		{
			ERROR_RETURN(p, "Line falls in the plane, infinite solutions!");
		}

		ERROR_RETURN(Vector3::ZERO, "No solution!");
	}

	// We need to find a p0 such that:
	//  dot(p0, n) = d
	//  p0 = p + dir * t for some t
	// Substitute p0 into the first,
	//  dot(p + dir * t, n) = d
	// => dot(p, n) + dot(dir * t, n) = d, since dot(v1 + v2, v3) = dot(v1, v3) + dot(v2, v3)
	// => dot(p, n) + t * dot(dir, n) = d, since dot(a * v1, v2) = a * dot(v1, v2)
	// => t = (d - dot(p, n) / dot(dir, n)), and we already know dot(dir, n) is nonzero from the check above

	float t = (d - DotProduct(p, n)) / dot;
	return line.FindPointAtT(t);
}


//-------------------------------------------------------------------------------------------------
float GetClosestPointOnLineSegment(const Vector3& start, const Vector3& end, const Vector3& point, Vector3& out_closestPoint)
{
	Vector3 direction = end - start;
	float d = direction.Normalize();

	float dot = DotProduct(point - start, direction);
	float t = dot / d;

	if (t < 0.f)
	{
		out_closestPoint = start;
	}
	else if (t > 1.f)
	{
		out_closestPoint = end;
	}
	else
	{
		out_closestPoint = start + t * direction;
	}

	return (out_closestPoint - point).GetLength();
}


//-------------------------------------------------------------------------------------------------
float FindClosestPointsOnLineSegments(const Vector3& startA, const Vector3& endA, const Vector3& startB, const Vector3& endB, Vector3& out_pointOnA, Vector3& out_pointOnB)
{
	// Make a plane on startB with normal of B
	Vector3 normal = (endB - startB).GetNormalized();
	Plane3 plane(normal, startB);

	// Project A's endpoints onto the plane
	Vector3 inPlaneStartA = plane.GetProjectedPointOntoPlane(startA);
	Vector3 inPlaneEndA = plane.GetProjectedPointOntoPlane(endA);

	// Find the T value for A's closest point - since we're in startB's plane, startB will be the closest point
	Vector3 inPlaneA = inPlaneEndA - inPlaneStartA;
	float tA = DotProduct(startB - inPlaneStartA, inPlaneA) / DotProduct(inPlaneA, inPlaneA);	
	
	// If A and B are parallel, just use t == 0
	if (inPlaneA == Vector3::ZERO)
	{
		tA = 0;
	}

	// Clamp tA to the segment
	tA = Clamp(tA, 0.f, 1.0f);

	// Find the closest point on A
	out_pointOnA = Interpolate(startA, endA, tA);

	// Find the closest point on B to this point on A
	return GetClosestPointOnLineSegment(startB, endB, out_pointOnA, out_pointOnB);
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
