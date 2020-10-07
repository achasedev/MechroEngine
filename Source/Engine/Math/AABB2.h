///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 29th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector4.h"
#pragma warning(disable : 4201) // Keep the structs anonymous

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
class AABB2
{
public:

	AABB2() {}
	explicit AABB2(float width, float height);
	explicit AABB2(float minX, float minY, float maxX, float maxY);
	explicit AABB2(const Vector2& mins, const Vector2& maxs);
	explicit AABB2(const Vector2& center, float radiusX, float radiusY);
	explicit AABB2(const Vector2& minsAndMaxs);
	AABB2(const AABB2& copy);
	~AABB2() {}

	void	StretchToIncludePoint(float x, float y);
	void	StretchToIncludePoint(const Vector2& point);
	void	AddPaddingToSides(float xPaddingRadius, float yPaddingRadius);
	void	Translate(const Vector2& translation);
	void	Translate(float translationX, float TranslationY);
	bool	IsPointInside(float x, float y) const;
	bool	IsPointInside(const Vector2& point) const;

	Vector2 GetDimensions() const;
	float	GetWidth() const;
	float	GetHeight() const;
	float	GetAspect() const;
	Vector2 GetCenter() const;
	Vector2 GetRandomPointInside() const;
	Vector2 GetBottomLeft() const;
	Vector2 GetBottomRight() const;
	Vector2 GetTopRight() const;
	Vector2 GetTopLeft() const;

	void operator=(const AABB2& copy);
	void operator+=(const Vector2& translation);
	void operator-=(const Vector2& antiTranslation);
	AABB2 operator+(const Vector2& translation) const;
	AABB2 operator-(const Vector2& antiTranslation) const;
	AABB2 operator*(float scalar) const;


public:

	static const AABB2 NEGATIVE_ONE_TO_ONE;
	static const AABB2 NEGATIVE_HALF_TO_HALF;
	static const AABB2 ZERO_TO_ONE;
	static const AABB2 ZEROS;


public:
	//-----Public Member Data-----

	union
	{
		float	data[4];
		Vector4 vectorData;

		struct 
		{
			Vector2 mins;
			Vector2 maxs;
		};

		struct
		{
			float left;
			float bottom;
			float right;
			float top;
		};
	};
};

#pragma warning(default : 4201) // Keep the structs anonymous

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
