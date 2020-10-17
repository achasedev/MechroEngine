///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 3rd, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Math/AABB2.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

// See Unity's anchor presets
enum class AnchorPreset
{
	TOP_LEFT,
	TOP_CENTER,
	TOP_RIGHT,
	MIDDLE_LEFT,
	MIDDLE_CENTER,
	MIDDLE_RIGHT,
	BOTTOM_LEFT,
	BOTTOM_CENTER,
	BOTTOM_RIGHT,
	LEFT_STRETCH,
	CENTER_STRETCH,
	RIGHT_STRETCH,
	TOP_STRETCH,
	MIDDLE_STRETCH,
	BOTTOM_STRETCH,
	STRETCH_ALL
};

// Convenience, for tracking anchor state
enum class AnchorMode
{
	X_POSITIONAL_Y_POSITIONAL,
	X_POSITIONAL_Y_PADDING,
	X_PADDING_Y_POSITIONAL,
	X_PADDING_Y_PADDING
};

class Canvas;
class OBB2;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class RectTransform
{
public:
	//-----Public Methods-----

	RectTransform(Canvas* canvas);

	void		SetLeftPadding(float left);
	void		SetRightPadding(float right);
	void		SetHorizontalPadding(float left, float right);

	void		SetTopPadding(float top);
	void		SetBottomPadding(float bottom);
	void		SetVerticalPadding(float top, float bottom);
	void		SetPadding(float left, float right, float top, float bottom);

	void		SetXPosition(float x);
	void		SetYPosition(float y);
	void		SetPosition(const Vector2& position);

	void		SetWidth(float width);
	void		SetHeight(float height);
	void		SetDimensions(float width, float height);
	void		SetDimensions(const Vector2& dimensions);

	void		SetPivot(const Vector2& pivot);
	void		SetOrientation(float orientationDegrees);
	void		SetAnchors(const AABB2& anchors);
	void		SetAnchors(float minX, float minY, float maxX, float maxY);
	void		SetAnchors(AnchorPreset anchorPreset);
	void		SetParentTransform(const RectTransform* parent);
	void		SetScale(float xScale, float yScale);
	void		SetScale(float uniformScale);
	void		SetScale(const Vector2& scale);

	bool		IsPaddingHorizontal() const;
	bool		IsPaddingVertical() const;
	OBB2		GetBounds() const;
	float		GetXPosition() const;
	float		GetYPosition() const;
	float		GetWidth() const;
	float		GetHeight() const;
	Vector2		GetDimensions() const;
	float		GetLeftPadding() const;
	float		GetRightPadding() const;
	float		GetBottomPadding() const;
	float		GetTopPadding() const;
	Vector2		GetScale() const;
	AnchorMode	GetAnchorMode() const { return m_anchorMode; }
	Vector2		GetPivot() const { return m_pivot; }
	Vector2		GetPivotCanvasSpace() const;


private:
	//-----Private Methods------

	void UpdateAnchorMode();


private:
	//-----Private Data------

	// Anchor-relative data - ALL are pre rotation/scale
	// Union to alias what the values mean based on anchor
	// (i.e when anchors aligned, left padding is now treated as a position, etc)
	union
	{
		float m_xPosition;		// Canvas coordinates; Offset from the anchors x to the pivot x
		float m_leftPadding;	// Canvas coordinates; Offset from anchor min x to this element's left side
	};

	union
	{
		float m_width;			// Canvas coordinates; Width of element
		float m_rightPadding;	// Canvas coordinates; Offset from this element's right side to anchor max x
	};

	union
	{
		float m_yPosition;		// Canvas coordinates; Offset from the anchors y to the pivot y
		float m_topPadding;		// Canvas coordinates; Offset from this element's top side to anchor max y
	};

	union
	{
		float m_height;			// Canvas coordinates; Height of element
		float m_bottomPadding;	// Canvas coordinates; Offset from anchor min y to this element's bottom side
	};

	AABB2					m_anchors		= AABB2(Vector2(0.5f));						// Normalized coordinates; (0,0) == parent's bottom left, (1,1) == parent's top right
	AnchorMode				m_anchorMode	= AnchorMode::X_POSITIONAL_Y_POSITIONAL;	// Updated whenever anchors change, just for convenience to know current anchor behavior
	Vector2					m_pivot			= Vector2(0.5f);							// Normalized coordinates; (0,0) == this element's bottom left, (1,1) == this element's top right
	const RectTransform*	m_parent		= nullptr;
	float					m_orientation	= 0.f;
	Vector2					m_scale			= Vector2::ONES;
	Canvas*					m_canvas		= nullptr;

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

AnchorPreset StringToAnchorPreset(const std::string& text);