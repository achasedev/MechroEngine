///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 3rd, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/OBB2.h"
#include "Engine/UI/Canvas.h"
#include "Engine/UI/RectTransform.h"

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
RectTransform::RectTransform(Canvas* canvas)
	: m_canvas(canvas)
{
}


//-------------------------------------------------------------------------------------------------
void RectTransform::SetLeftPadding(float left)
{
	bool usingPaddingForX = IsPaddingHorizontal();
	ASSERT_RECOVERABLE(usingPaddingForX, "Attempting to set UIElement left padding when anchors aren't set to pad horizontal!");

	if (usingPaddingForX)
	{
		m_leftPadding = left;
	}
}


//-------------------------------------------------------------------------------------------------
void RectTransform::SetRightPadding(float right)
{
	bool isPaddingHorizontal = IsPaddingHorizontal();
	ASSERT_RECOVERABLE(isPaddingHorizontal, "Attempting to set UIElement right padding when anchors aren't set to pad horizontal!");

	if (isPaddingHorizontal)
	{
		m_rightPadding = right;
	}
}


//-------------------------------------------------------------------------------------------------
void RectTransform::SetHorizontalPadding(float left, float right)
{
	bool isPaddingHorizontal = IsPaddingHorizontal();
	ASSERT_RECOVERABLE(isPaddingHorizontal, "Attempting to set UIElement horizontal padding when anchors aren't set to pad horizontal!");

	if (isPaddingHorizontal)
	{
		m_leftPadding = left;
		m_rightPadding = right;
	}
}


//-------------------------------------------------------------------------------------------------
void RectTransform::SetTopPadding(float top)
{
	bool isPaddingVertical = IsPaddingVertical();
	ASSERT_RECOVERABLE(isPaddingVertical, "Attempting to set UIElement top padding when anchors aren't set to pad vertical!");

	if (isPaddingVertical)
	{
		m_topPadding = top;
	}
}


//-------------------------------------------------------------------------------------------------
void RectTransform::SetBottomPadding(float bottom)
{
	bool isPaddingVertical = IsPaddingVertical();
	ASSERT_RECOVERABLE(isPaddingVertical, "Attempting to set UIElement bottom padding when anchors aren't set to pad vertical!");

	if (isPaddingVertical)
	{
		m_bottomPadding = bottom;
	}
}


//-------------------------------------------------------------------------------------------------
void RectTransform::SetVerticalPadding(float top, float bottom)
{
	bool isPaddingVertical = IsPaddingVertical();
	ASSERT_RECOVERABLE(isPaddingVertical, "Attempting to set UIElement vertical padding when anchors aren't set to pad vertical!");

	if (isPaddingVertical)
	{
		m_topPadding = top;
		m_bottomPadding = bottom;
	}
}


//-------------------------------------------------------------------------------------------------
void RectTransform::SetPadding(float left, float right, float top, float bottom)
{
	SetHorizontalPadding(left, right);
	SetVerticalPadding(top, bottom);
}


//-------------------------------------------------------------------------------------------------
void RectTransform::SetXPosition(float x)
{
	bool isPositioningHorizontal = !IsPaddingHorizontal();
	ASSERT_RECOVERABLE(isPositioningHorizontal, "Attempting to set UIElement X position when anchors set to pad horizontal!");

	if (isPositioningHorizontal)
	{
		m_xPosition = x;
	}
}


//-------------------------------------------------------------------------------------------------
void RectTransform::SetYPosition(float y)
{
	bool isPositioningVertical = !IsPaddingVertical();
	ASSERT_RECOVERABLE(isPositioningVertical, "Attempting to set UIElement Y position when anchors set to pad vertical!");

	if (isPositioningVertical)
	{
		m_yPosition = y;
	}
}


//-------------------------------------------------------------------------------------------------
void RectTransform::SetPosition(const Vector2& position)
{
	SetXPosition(position.x);
	SetYPosition(position.y);
}


//-------------------------------------------------------------------------------------------------
void RectTransform::SetWidth(float width)
{
	bool isPositioningHorizontal = !IsPaddingHorizontal();
	ASSERT_RECOVERABLE(isPositioningHorizontal, "Attempting to set UIElement width when anchors set to pad horizontal!");

	if (isPositioningHorizontal)
	{
		m_width = width;
	}
}


//-------------------------------------------------------------------------------------------------
void RectTransform::SetHeight(float height)
{
	bool isPositioningVertical = !IsPaddingVertical();
	ASSERT_RECOVERABLE(isPositioningVertical, "Attempting to set UIElement height when anchors set to pad vertical!");

	if (isPositioningVertical)
	{
		m_height = height;
	}
}


//-------------------------------------------------------------------------------------------------
void RectTransform::SetDimensions(const Vector2& dimensions)
{
	ASSERT_RECOVERABLE(dimensions.x >= 0.f, "Negative dimension!");
	ASSERT_RECOVERABLE(dimensions.y >= 0.f, "Negative dimension!");

	SetWidth(Max(dimensions.x, 0.f));
	SetHeight(Max(dimensions.y, 0.f));
}


//-------------------------------------------------------------------------------------------------
void RectTransform::SetDimensions(float width, float height)
{
	SetWidth(width);
	SetHeight(height);
}


//-------------------------------------------------------------------------------------------------
void RectTransform::SetPivot(const Vector2& pivot)
{
	m_pivot = pivot;
}


//-------------------------------------------------------------------------------------------------
void RectTransform::SetOrientation(float orientationDegrees)
{
	m_orientation = orientationDegrees;
}


//-------------------------------------------------------------------------------------------------
void RectTransform::SetAnchors(const AABB2& anchors)
{
	ASSERT_OR_DIE(anchors.mins.x <= anchors.maxs.x, "Horizontal anchors invalid!");
	ASSERT_OR_DIE(anchors.mins.y <= anchors.maxs.y, "Vertical anchors invalid!");

	m_anchors = anchors;

	// Update convenience mode
	UpdateAnchorMode();
}


//-------------------------------------------------------------------------------------------------
void RectTransform::SetAnchors(float minX, float minY, float maxX, float maxY)
{
	SetAnchors(AABB2(minX, minY, maxX, maxY));
}


//-------------------------------------------------------------------------------------------------
void RectTransform::SetAnchors(AnchorPreset anchorPreset)
{
	switch (anchorPreset)
	{
	case AnchorPreset::TOP_LEFT:		m_anchors = AABB2(0.0f, 1.0f, 0.0f, 1.0f); break;
	case AnchorPreset::TOP_CENTER:		m_anchors = AABB2(0.5f, 1.0f, 0.5f, 1.0f); break;
	case AnchorPreset::TOP_RIGHT:		m_anchors = AABB2(1.0f, 1.0f, 1.0f, 1.0f); break;
	case AnchorPreset::MIDDLE_LEFT:		m_anchors = AABB2(0.0f, 0.5f, 0.0f, 0.5f); break;
	case AnchorPreset::MIDDLE_CENTER:	m_anchors = AABB2(0.5f, 0.5f, 0.5f, 0.5f); break;
	case AnchorPreset::MIDDLE_RIGHT:	m_anchors = AABB2(1.0f, 0.5f, 1.0f, 0.5f); break;
	case AnchorPreset::BOTTOM_LEFT:		m_anchors = AABB2(0.0f, 0.0f, 0.0f, 0.0f); break;
	case AnchorPreset::BOTTOM_CENTER:	m_anchors = AABB2(0.5f, 0.0f, 0.5f, 0.0f); break;
	case AnchorPreset::BOTTOM_RIGHT:	m_anchors = AABB2(1.0f, 0.0f, 1.0f, 0.0f); break;
	case AnchorPreset::LEFT_STRETCH:	m_anchors = AABB2(0.0f, 0.0f, 0.0f, 1.0f); break;
	case AnchorPreset::CENTER_STRETCH:	m_anchors = AABB2(0.5f, 0.0f, 0.5f, 1.0f); break;
	case AnchorPreset::RIGHT_STRETCH:	m_anchors = AABB2(1.0f, 0.0f, 1.0f, 1.0f); break;
	case AnchorPreset::TOP_STRETCH:		m_anchors = AABB2(0.0f, 1.0f, 1.0f, 1.0f); break;
	case AnchorPreset::MIDDLE_STRETCH:	m_anchors = AABB2(0.0f, 0.5f, 1.0f, 0.5f); break;
	case AnchorPreset::BOTTOM_STRETCH:	m_anchors = AABB2(0.0f, 0.0f, 1.0f, 0.0f); break;
	case AnchorPreset::STRETCH_ALL:		m_anchors = AABB2(0.0f, 0.0f, 1.0f, 1.0f); break;
	default:
		ERROR_RECOVERABLE("Invalid AnchorPreset!");
		break;
	}

	UpdateAnchorMode();
}


//-------------------------------------------------------------------------------------------------
void RectTransform::SetParentTransform(const RectTransform* parent)
{
	m_parent = parent;
}


//-------------------------------------------------------------------------------------------------
void RectTransform::SetScale(float xScale, float yScale)
{
	m_scale.x = xScale;
	m_scale.y = yScale;
}


//-------------------------------------------------------------------------------------------------
void RectTransform::SetScale(float uniformScale)
{
	SetScale(uniformScale, uniformScale);
}


//-------------------------------------------------------------------------------------------------
void RectTransform::SetScale(const Vector2& scale)
{
	SetScale(scale.x, scale.y);
}


//-------------------------------------------------------------------------------------------------
bool RectTransform::IsPaddingHorizontal() const
{
	return m_anchorMode == AnchorMode::X_PADDING_Y_PADDING || m_anchorMode == AnchorMode::X_PADDING_Y_POSITIONAL;
}


//-------------------------------------------------------------------------------------------------
bool RectTransform::IsPaddingVertical() const
{
	return m_anchorMode == AnchorMode::X_PADDING_Y_PADDING || m_anchorMode == AnchorMode::X_POSITIONAL_Y_PADDING;
}


//-------------------------------------------------------------------------------------------------
OBB2 RectTransform::GetBounds() const
{
	if (m_parent != nullptr)
	{
		OBB2 orientedRefBounds = m_parent->GetBounds();
		AABB2& alignedRefBounds = orientedRefBounds.alignedBounds;

		Vector2 scale = GetScale();
		Vector2 parentDimensions = orientedRefBounds.GetDimensions();

		// Get anchor positions
		AABB2 anchorPositions = AABB2::ZERO_TO_ONE;
		anchorPositions.mins.x = m_anchors.mins.x * parentDimensions.x + alignedRefBounds.mins.x;
		anchorPositions.maxs.x = m_anchors.maxs.x * parentDimensions.x + alignedRefBounds.mins.x;
		anchorPositions.mins.y = m_anchors.mins.y * parentDimensions.y + alignedRefBounds.mins.y;
		anchorPositions.maxs.y = m_anchors.maxs.y * parentDimensions.y + alignedRefBounds.mins.y;

		AABB2 bounds;

		// Horizontal
		if (IsPaddingHorizontal())
		{
			bounds.mins.x = anchorPositions.mins.x + m_leftPadding;
			bounds.maxs.x = anchorPositions.maxs.x - m_rightPadding;

			// Scale
			float deltaWidth = (scale.x - 1.0f) * m_width;
			bounds.mins.x -= deltaWidth * m_pivot.x;
			bounds.maxs.x += deltaWidth * (1.0f - m_pivot.x);
		}
		else
		{
			// Account for the pivot + scale
			float scaledWidth = m_width * scale.x;
			bounds.mins.x = anchorPositions.mins.x + m_xPosition + -1.0f * (m_pivot.x * scaledWidth);
			bounds.maxs.x = bounds.mins.x + scaledWidth;
		}

		// Vertical
		if (IsPaddingVertical())
		{
			bounds.mins.y = anchorPositions.mins.y + m_bottomPadding;
			bounds.maxs.y = anchorPositions.maxs.y - m_topPadding;

			// Scale
			float deltaHeight = (scale.y - 1.0f) * m_height;
			bounds.mins.y -= deltaHeight * m_pivot.y;
			bounds.maxs.y += deltaHeight * (1.0f - m_pivot.y);
		}
		else
		{
			// Account for the pivot + scale
			float scaledHeight = m_height * scale.y;
			bounds.mins.y = anchorPositions.mins.y + m_yPosition + -1.0f * (m_pivot.y * scaledHeight);
			bounds.maxs.y = bounds.mins.y + scaledHeight;
		}

		// Rotation
		float orientation = orientedRefBounds.orientationDegrees + m_orientation;

		return OBB2(bounds, orientation);
	}
	else
	{
		// Just return my absolute bounds
		AABB2 bounds = AABB2(m_width, m_height);
		return OBB2(bounds, 0.f);
	}
}


//-------------------------------------------------------------------------------------------------
float RectTransform::GetXPosition() const
{
	ASSERT_OR_DIE(!IsPaddingHorizontal(), "Trying to read position when anchors are using padding!");
	return m_xPosition;
}


//-------------------------------------------------------------------------------------------------
float RectTransform::GetYPosition() const
{
	ASSERT_OR_DIE(!IsPaddingVertical(), "Trying to read position when anchors are using padding!");
	return m_yPosition;
}


//-------------------------------------------------------------------------------------------------
float RectTransform::GetWidth() const
{
	ASSERT_OR_DIE(!IsPaddingHorizontal(), "Trying to read position when anchors are using padding!");
	return m_width;
}


//-------------------------------------------------------------------------------------------------
float RectTransform::GetHeight() const
{
	ASSERT_OR_DIE(!IsPaddingVertical(), "Trying to read position when anchors are using padding!");
	return m_height;
}


//-------------------------------------------------------------------------------------------------
float RectTransform::GetLeftPadding() const
{
	ASSERT_OR_DIE(IsPaddingHorizontal(), "Trying to read padding when anchors are using positions!");
	return m_leftPadding;
}


//-------------------------------------------------------------------------------------------------
float RectTransform::GetRightPadding() const
{
	ASSERT_OR_DIE(IsPaddingHorizontal(), "Trying to read padding when anchors are using positions!");
	return m_rightPadding;
}


//-------------------------------------------------------------------------------------------------
float RectTransform::GetBottomPadding() const
{
	ASSERT_OR_DIE(IsPaddingVertical(), "Trying to read padding when anchors are using positions!");
	return m_bottomPadding;
}


//-------------------------------------------------------------------------------------------------
float RectTransform::GetTopPadding() const
{
	ASSERT_OR_DIE(IsPaddingVertical(), "Trying to read padding when anchors are using positions!");
	return m_topPadding;
}


//-------------------------------------------------------------------------------------------------
Vector2 RectTransform::GetScale() const
{
	Vector2 parentScale = (m_parent != nullptr ? m_parent->GetScale() : Vector2::ONES);
	
	return Vector2(m_scale.x * parentScale.x, m_scale.y * parentScale.y);
}


//-------------------------------------------------------------------------------------------------
void RectTransform::UpdateAnchorMode()
{
	if (m_anchors.mins.x == m_anchors.maxs.x)
	{
		if (m_anchors.mins.y == m_anchors.maxs.y)
		{
			m_anchorMode = AnchorMode::X_POSITIONAL_Y_POSITIONAL;
		}
		else
		{
			m_anchorMode = AnchorMode::X_POSITIONAL_Y_PADDING;
		}
	}
	else
	{
		if (m_anchors.mins.y == m_anchors.maxs.y)
		{
			m_anchorMode = AnchorMode::X_PADDING_Y_POSITIONAL;
		}
		else
		{
			m_anchorMode = AnchorMode::X_PADDING_Y_PADDING;
		}
	}
}


//-------------------------------------------------------------------------------------------------
AnchorPreset StringToAnchorPreset(const std::string& text)
{
	if (text == "top_left")			{ return AnchorPreset::TOP_LEFT; }
	if (text == "top_center")		{ return AnchorPreset::TOP_CENTER; }
	if (text == "top_right")		{ return AnchorPreset::TOP_RIGHT; }
	if (text == "middle_left")		{ return AnchorPreset::MIDDLE_LEFT; }
	if (text == "middle_center")	{ return AnchorPreset::MIDDLE_CENTER; }
	if (text == "middle_right")		{ return AnchorPreset::MIDDLE_RIGHT; }
	if (text == "bottom_left")		{ return AnchorPreset::BOTTOM_LEFT; }
	if (text == "bottom_center")	{ return AnchorPreset::BOTTOM_CENTER; }
	if (text == "bottom_right")		{ return AnchorPreset::BOTTOM_RIGHT; }
	if (text == "left_stretch")		{ return AnchorPreset::LEFT_STRETCH; }
	if (text == "center_stretch")	{ return AnchorPreset::CENTER_STRETCH; }
	if (text == "right_stretch")	{ return AnchorPreset::RIGHT_STRETCH; }
	if (text == "top_stretch")		{ return AnchorPreset::TOP_STRETCH; }
	if (text == "middle_stretch")	{ return AnchorPreset::MIDDLE_STRETCH; }
	if (text == "bottom_stretch")	{ return AnchorPreset::BOTTOM_STRETCH; }
	if (text == "stretch_all")		{ return AnchorPreset::STRETCH_ALL; }
	
	ERROR_RECOVERABLE("Unable to convert text to AnchorPreset! Defaulting to bottom_right...");
	return AnchorPreset::BOTTOM_RIGHT;
}
