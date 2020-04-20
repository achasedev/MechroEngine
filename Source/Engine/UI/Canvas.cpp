///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 4th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Event/EventSystem.h"
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Render/Camera/Camera.h"
#include "Engine/Render/Core/RenderContext.h"
#include "Engine/Render/Texture/Texture2D.h"
#include "Engine/UI/Canvas.h"
#include "Engine/UI/UIElement.h"
#include "Engine/Utility/NamedProperties.h"

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
Canvas::Canvas()
	: UIElement(nullptr)
{
}

//-------------------------------------------------------------------------------------------------
Canvas::~Canvas()
{
}


//-------------------------------------------------------------------------------------------------
void Canvas::Initialize(Texture2D* outputTexture, const Vector2& resolution, ScreenMatchMode mode, float widthHeightBlend /*= 1.0f*/)
{
	m_outputTexture = outputTexture;
	m_matchMode = mode;
	m_widthOrHeightBlend = widthHeightBlend;
	m_resolution = Vector2(resolution.x, resolution.y);
	m_transform.SetDimensions(resolution.x, resolution.y);
}


//-------------------------------------------------------------------------------------------------
void Canvas::Render() const
{
	UIElement::Render();
}


//-------------------------------------------------------------------------------------------------
void Canvas::SetScreenMatchMode(ScreenMatchMode mode, float widthHeightBlend /*= 1.0f*/)
{
	m_matchMode = mode;
	m_widthOrHeightBlend = widthHeightBlend;
}


//-------------------------------------------------------------------------------------------------
void Canvas::SetResolution(float width, float height)
{
	m_resolution = Vector2(width, height);
	m_transform.SetDimensions(width, height);
}


//-------------------------------------------------------------------------------------------------
Texture2D* Canvas::GetOutputTexture() const
{
	return m_outputTexture;
}


//-------------------------------------------------------------------------------------------------
Matrix44 Canvas::GenerateOrtho()
{
	AABB2 orthoBounds;
	orthoBounds.left = 0.f;
	orthoBounds.top = m_resolution.y;

	float targetAspect = m_outputTexture->GetAspect();

	float heightToFillVertical = m_resolution.y;
	float heightToFillHorizontal = m_resolution.x / targetAspect;

	float finalHeight = 0.f;

	switch (m_matchMode)
	{
	case SCREEN_MATCH_WIDTH_OR_HEIGHT:
		finalHeight = (heightToFillVertical * m_widthOrHeightBlend) + (heightToFillHorizontal * (1.0f - m_widthOrHeightBlend));
		break;
	case SCREEN_MATCH_EXPAND_TO_FILL:
		finalHeight = Min(heightToFillVertical, heightToFillHorizontal);
		break;
	case SCREEN_MATCH_SHRINK_TO_FIT:
		finalHeight = Max(heightToFillVertical, heightToFillHorizontal);
		break;
	default:
		break;
	}

	orthoBounds.bottom = orthoBounds.top - finalHeight;
	orthoBounds.right = finalHeight * targetAspect;

	return Matrix44::MakeOrtho(orthoBounds.GetBottomLeft(), orthoBounds.GetTopRight(), -1.0f, 1.0f);
}
