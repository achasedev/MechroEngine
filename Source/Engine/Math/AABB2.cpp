///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 29, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** INCLUDES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Math/AABB2.h"
#include "Engine/Math/MathUtils.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** DEFINES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                              *** TYPES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** STRUCTS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                        *** GLOBALS AND STATICS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------
const AABB2 AABB2::NEGATIVE_ONE_TO_ONE = AABB2(Vector2(-1.f, -1.f), Vector2(1.f, 1.f));
const AABB2 AABB2::NEGATIVE_HALF_TO_HALF = AABB2(Vector2(-0.5f, -0.5f), Vector2(0.5f, 0.5f));
const AABB2 AABB2::ZERO_TO_ONE = AABB2(Vector2(0.f, 0.f), Vector2(1.f, 1.f));

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                           *** C FUNCTIONS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** CLASSES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
AABB2::AABB2(float minX, float minY, float maxX, float maxY)
	: mins(Vector2(minX, minY))
	, maxs(Vector2(maxX, maxY))
{
}


//-------------------------------------------------------------------------------------------------
AABB2::AABB2(const Vector2& referenceMins, const Vector2& referenceMaxs)
	: mins(referenceMins)
	, maxs(referenceMaxs)
{
}


//-------------------------------------------------------------------------------------------------
AABB2::AABB2(const Vector2& center, float radiusX, float radiusY)
	: mins(Vector2((center.x - radiusX), (center.y - radiusY)))
	, maxs(Vector2((center.x + radiusX), (center.y + radiusY)))
{
}


//-------------------------------------------------------------------------------------------------
void AABB2::StretchToIncludePoint(float x, float y)
{
	if (x < mins.x)
	{
		mins.x = x;
	}
	else if (x > maxs.x)
	{
		maxs.x = x;
	}

	if (y < mins.y)
	{
		mins.y = y;
	}
	else if (y > maxs.y)
	{
		maxs.y = y;
	}
}


//-------------------------------------------------------------------------------------------------
void AABB2::StretchToIncludePoint(const Vector2& point)
{
	StretchToIncludePoint(point.x, point.y);
}


//-------------------------------------------------------------------------------------------------
void AABB2::AddPaddingToSides(float xPadding, float yPadding)
{
	mins.x -= xPadding;
	maxs.x += xPadding;
	mins.y -= yPadding;
	maxs.y += yPadding;
}


//-------------------------------------------------------------------------------------------------
void AABB2::Translate(const Vector2& translation)
{
	Translate(translation.x, translation.y);
}


//-------------------------------------------------------------------------------------------------
void AABB2::Translate(float translationX, float translationY)
{
	mins.x += translationX;
	maxs.x += translationX;
	mins.y += translationY;
	maxs.y += translationY;
}


//-------------------------------------------------------------------------------------------------
bool AABB2::IsPointInside(float x, float y) const
{
	bool isInside = true;

	if (x < mins.x || x > maxs.x)
	{
		isInside = false;
	}

	if (y < mins.y || y > maxs.y)
	{
		isInside = false;
	}

	return isInside;
}


//-------------------------------------------------------------------------------------------------
bool AABB2::IsPointInside(const Vector2& point) const
{
	return IsPointInside(point.x, point.y);
}


//-------------------------------------------------------------------------------------------------
Vector2 AABB2::GetDimensions() const
{
	float width = (maxs.x - mins.x);
	float height = (maxs.y - mins.y);

	return Vector2(width, height);
}


//-------------------------------------------------------------------------------------------------
Vector2 AABB2::GetCenter() const
{
	float xCenter = 0.5f * (maxs.x + mins.x);
	float yCenter = 0.5f * (maxs.y + mins.y);

	return Vector2(xCenter, yCenter);
}


//-------------------------------------------------------------------------------------------------
Vector2 AABB2::GetRandomPointInside() const
{
	float randomX = GetRandomFloatInRange(mins.x, maxs.x);
	float randomY = GetRandomFloatInRange(mins.y, maxs.y);

	return Vector2(randomX, randomY);
}


//-------------------------------------------------------------------------------------------------
Vector2 AABB2::GetBottomLeft() const
{
	return mins;
}


//-------------------------------------------------------------------------------------------------
Vector2 AABB2::GetBottomRight() const
{
	return Vector2(maxs.x, mins.y);
}


//-------------------------------------------------------------------------------------------------
Vector2 AABB2::GetTopRight() const
{
	return maxs;
}


//-------------------------------------------------------------------------------------------------
Vector2 AABB2::GetTopLeft() const
{
	return Vector2(mins.x, maxs.y);
}


//-------------------------------------------------------------------------------------------------
void AABB2::operator+=(const Vector2& translation)
{
	Translate(translation);
}


//-------------------------------------------------------------------------------------------------
void AABB2::operator-=(const Vector2& translation)
{
	Translate(-1.0f * translation);
}


//-------------------------------------------------------------------------------------------------
AABB2 AABB2::operator*(float scalar) const
{
	AABB2 scaledBox;

	scaledBox.mins = mins * scalar;
	scaledBox.maxs = maxs * scalar;

	return scaledBox;
}


//-------------------------------------------------------------------------------------------------
AABB2 AABB2::operator+(const Vector2& translation) const
{
	AABB2 translatedBox = *this;
	translatedBox.Translate(translation);

	return translatedBox;
}


//-------------------------------------------------------------------------------------------------
AABB2 AABB2::operator-(const Vector2& translation) const
{
	AABB2 antiTranslatedBox = *this;
	antiTranslatedBox.Translate(-1.0f * translation);

	return antiTranslatedBox;
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
