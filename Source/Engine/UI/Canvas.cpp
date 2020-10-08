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
#include "Engine/UI/UIPanel.h"
#include "Engine/UI/UIElement.h"
#include "Engine/UI/UIText.h"
#include "Engine/Utility/NamedProperties.h"
#include "Engine/Utility/StringID.h"
#include "Engine/Utility/XMLUtils.h"

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
static ScreenMatchMode StringToScreenMatchMode(const std::string& text)
{
	if (text == "blend") { return SCREEN_MATCH_WIDTH_OR_HEIGHT; }
	if (text == "shrink") { return SCREEN_MATCH_SHRINK_TO_FIT; }
	if (text == "expand") { return SCREEN_MATCH_EXPAND_TO_FILL; }

	ERROR_RECOVERABLE("Invalid match mode %s", text.c_str());
	return SCREEN_MATCH_WIDTH_OR_HEIGHT;
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
Canvas::Canvas()
	: UIElement(nullptr)
{
	m_id = SID("canvas");
	m_outputTexture = g_renderContext->GetDefaultRenderTarget();
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
// Doesn't call UIElement::InitializeFromXML() to avoid setting anchors, pivot, etc.
void Canvas::InitializeFromXML(const XMLElem& element)
{
	ASSERT_OR_DIE(strcmp(element.Name(), "canvas") == 0, "XMLElement isn't for a canvas!");

	// Resolution
	m_resolution = XML::ParseAttribute(element, "resolution", Vector2(1000.f));

	// Match mode
	std::string matchModeText = XML::ParseAttribute(element, "match_mode", "blend");
	m_matchMode = StringToScreenMatchMode(matchModeText);

	// If blending, get blend
	if (m_matchMode == SCREEN_MATCH_WIDTH_OR_HEIGHT)
	{
		m_widthOrHeightBlend = XML::ParseAttribute(element, "blend", 1.0f);
	}

	// Create the child elements
	const XMLElem* child = element.FirstChildElement();
	while (child != nullptr)
	{
		UIElement* uiElement = CreateUIElementFromXML(*child, this);

		if (uiElement != nullptr)
		{
			m_children.push_back(uiElement);
		}

		child = child->NextSiblingElement();
	}
}


//-------------------------------------------------------------------------------------------------
void Canvas::InitializeFromXML(const char* xmlFilePath)
{
	XMLDoc document;
	XMLErr error = document.LoadFile(xmlFilePath);
	GUARANTEE_OR_DIE(error == tinyxml2::XML_SUCCESS, "Couldn't load %s!", xmlFilePath);
	
	InitializeFromXML(*document.RootElement());
}


//-------------------------------------------------------------------------------------------------
void Canvas::Render()
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
float Canvas::GetAspect() const
{
	return m_resolution.x / m_resolution.y;
}


//-------------------------------------------------------------------------------------------------
// The current DPI of the canvas in both dimensions
Vector2 Canvas::GetPixelsPerUnit() const
{
	AABB2 visibleBounds = GenerateOrthoBounds();
	float horizontalPPU = static_cast<float>(m_outputTexture->GetWidth()) / visibleBounds.GetWidth();
	float verticalPPU = static_cast<float>(m_outputTexture->GetHeight()) / visibleBounds.GetHeight();

	return Vector2(horizontalPPU, verticalPPU);
}


//-------------------------------------------------------------------------------------------------
// Returns the number of canvas units per pixel in both dimensions
Vector2 Canvas::GetCanvasUnitsPerPixel() const
{
	AABB2 visibleBounds = GenerateOrthoBounds();
	float horizontalUnitsPerPixel = visibleBounds.GetWidth() / static_cast<float>(m_outputTexture->GetWidth());
	float verticalUnitsPerPixel = visibleBounds.GetHeight() / static_cast<float>(m_outputTexture->GetHeight());

	return Vector2(horizontalUnitsPerPixel, verticalUnitsPerPixel);
}


//-------------------------------------------------------------------------------------------------
uint32 Canvas::ToPixelWidth(float canvasWidth) const
{
	float horizontalPPU = GetPixelsPerUnit().x;
	return (uint32)RoundToNearestInt(canvasWidth * horizontalPPU);
}


//-------------------------------------------------------------------------------------------------
uint32 Canvas::ToPixelHeight(float canvasHeight) const
{
	float verticalPPU = GetPixelsPerUnit().y;
	return (uint32)RoundToNearestInt(canvasHeight * verticalPPU);
}


//-------------------------------------------------------------------------------------------------
float Canvas::ToCanvasWidth(uint32 pixelWidth) const
{
	float horizontalUnitsPerPixel = GetCanvasUnitsPerPixel().x;
	return pixelWidth * horizontalUnitsPerPixel;
}


//-------------------------------------------------------------------------------------------------
float Canvas::ToCanvasHeight(uint32 pixelHeight) const
{
	float verticalUnitsPerPixel = GetCanvasUnitsPerPixel().y;
	return pixelHeight * verticalUnitsPerPixel;
}


//-------------------------------------------------------------------------------------------------
// Determine what the bounding box in Canvas coordinates that will be visible based on Canvas matching
// The top left of the bounding box will *always* be the top left of the output texture!
// This is so the canvas will always be fixed to the top left
AABB2 Canvas::GenerateOrthoBounds() const
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

	return orthoBounds;
}


//-------------------------------------------------------------------------------------------------
Matrix44 Canvas::GenerateOrthoMatrix() const
{
	AABB2 orthoBounds = GenerateOrthoBounds();
	return Matrix44::MakeOrtho(orthoBounds.GetBottomLeft(), orthoBounds.GetTopRight(), -1.0f, 1.0f);
}
