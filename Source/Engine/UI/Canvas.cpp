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
#include "Engine/IO/InputSystem.h"
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
#include <algorithm>

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------
RTTI_TYPE_DEFINE(Canvas);

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


//-------------------------------------------------------------------------------------------------
static bool CompareByLayerDescending(UIElement* a, UIElement* b)
{
	return (a->GetLayer() > b->GetLayer());
}


//-------------------------------------------------------------------------------------------------
static bool CheckAndExecuteHandler(UIElement* element, UIMouseInputHandler handler, const UIMouseInfo& input)
{
	// Default to blocking input
	bool consumeInput = true;
	if (element != nullptr && handler != nullptr)
	{
		consumeInput = handler(element, input);
	}

	return consumeInput;
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
	m_outputTextureHeight = m_outputTexture->GetHeight();
	g_eventSystem->SubscribeEventCallbackObjectMethod("window-resize", &Canvas::Event_WindowResize, *this);
}


//-------------------------------------------------------------------------------------------------
Canvas::~Canvas()
{
	g_eventSystem->UnsubscribeEventCallbackObjectMethod("window-resize", &Canvas::Event_WindowResize, *this);
}


//-------------------------------------------------------------------------------------------------
void Canvas::Initialize(Texture2D* outputTexture, const Vector2& resolution, ScreenMatchMode mode, float widthHeightBlend /*= 1.0f*/)
{
	m_outputTexture = outputTexture;
	m_outputTextureHeight = m_outputTexture->GetHeight();
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
	m_transform.SetDimensions(m_resolution.x, m_resolution.y);

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
		UIElement* newElement = CreateUIElementFromXML(*child, this);

		if (newElement != nullptr)
		{
			AddChild(newElement);
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
void Canvas::ProcessInput()
{
	UIMouseInfo mouseInfo;
	SetupUIMouseInfo(mouseInfo);

	// We need to check all elements first before we begin calling their input functions
	// Otherwise they may move or change mid-check
	std::vector<UIElement*> hoverStack;
	FindMouseHoveredElements(mouseInfo.m_position, hoverStack);

	HandleMouseJustHovers(hoverStack, mouseInfo);
	HandleMouseHovers(hoverStack, mouseInfo);	
	HandleMouseClicks(hoverStack, mouseInfo);
	HandleMouseUnhovers(hoverStack, mouseInfo);

	// Save off current frame hovers
	m_elementsHoveredLastFrame = hoverStack;
	
	// Save off mouse state
	m_lastFrameUIMouseInfo = mouseInfo;
}


//-------------------------------------------------------------------------------------------------
void Canvas::Update()
{
	UIElement::Update();
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
void Canvas::AddElementToGlobalMap(UIElement* element)
{
	bool elementAlreadyAdded = m_globalElementMap.find(element->GetID()) != m_globalElementMap.end();
	ASSERT_RECOVERABLE(!elementAlreadyAdded, "Duplicate element being added!");

	// Default to preserving whatever was there
	if (!elementAlreadyAdded)
	{
		m_globalElementMap[element->GetID()] = element;
	}
}


//-------------------------------------------------------------------------------------------------
void Canvas::RemoveElementFromGlobalMap(UIElement* element)
{
	bool elementExists = m_globalElementMap.find(element->GetID()) != m_globalElementMap.end();
	ASSERT_RECOVERABLE(elementExists, "Element doesn't exist, can't remove!");

	if (elementExists)
	{
		m_globalElementMap.erase(element->GetID());

		// Also remove from our per-frame input cache
		std::vector<UIElement*>::iterator itr = std::find(m_elementsHoveredLastFrame.begin(), m_elementsHoveredLastFrame.end(), element);
		if (itr != m_elementsHoveredLastFrame.end())
		{
			m_elementsHoveredLastFrame.erase(itr);
		}

		if (element == m_currentClickedElement)
		{
			m_currentClickedElement = nullptr;
		}
	}
}


//-------------------------------------------------------------------------------------------------
Texture2D* Canvas::GetOutputTexture() const
{
	return m_outputTexture;
}


//-------------------------------------------------------------------------------------------------
UIElement* Canvas::FindElementByID(StringID id)
{
	if (m_globalElementMap.find(id) != m_globalElementMap.end())
	{
		return m_globalElementMap[id];
	}

	return nullptr;
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
StringID Canvas::GetNextUnspecifiedID()
{
	std::string name = Stringf("UNSPECIFIED_%i", m_nextUnspecifiedIDSuffix++);
	return SID(name);
}


//-------------------------------------------------------------------------------------------------
Vector2 Canvas::GetMousePosition() const
{
	const Mouse& mouse = g_inputSystem->GetMouse();
	IntVector2 mouseClientPos = mouse.GetCursorClientPosition();

	// Client (0,0) is top left, but canvas (0,0) is bottom left
	Vector2 mouseCanvasPos = Vector2(ToCanvasWidth((uint32)mouseClientPos.x), m_resolution.y - ToCanvasHeight((uint32)mouseClientPos.y));
	return mouseCanvasPos;
}


//-------------------------------------------------------------------------------------------------
Vector2 Canvas::GetMousePositionLastFrame() const
{
	const Mouse& mouse = g_inputSystem->GetMouse();
	IntVector2 mouseClientLastFramePos = mouse.GetCursorClientLastFramePosition();

	// Client (0,0) is top left, but canvas (0,0) is bottom left
	Vector2 mouseCanvasLastFramePos = Vector2(ToCanvasWidth((uint32)mouseClientLastFramePos.x), m_resolution.y - ToCanvasHeight((uint32)mouseClientLastFramePos.y));
	return mouseCanvasLastFramePos;
}


//-------------------------------------------------------------------------------------------------
bool Canvas::WasHoveredLastFrame(UIElement* element) const
{
	return (std::find(m_elementsHoveredLastFrame.begin(), m_elementsHoveredLastFrame.end(), element) != m_elementsHoveredLastFrame.end());
}


//-------------------------------------------------------------------------------------------------
// If our output texture resizes, we need to dirty all UIText objects so they can be rebuilt using
// the correct font size
bool Canvas::Event_WindowResize(NamedProperties& args)
{
	UNUSED(args);

	if (m_outputTexture->GetHeight() != m_outputTextureHeight)
	{
		std::map<StringID, UIElement*>::iterator itr = m_globalElementMap.begin();

		for (itr; itr != m_globalElementMap.end(); itr++)
		{
			if (itr->second->GetType() == UIText::GetTypeStatic())
			{
				UIText* text = (UIText*)itr->second;
				text->MarkDirty();
			}
		}

		m_outputTextureHeight = m_outputTexture->GetHeight();
	}

	return false;
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


//-------------------------------------------------------------------------------------------------
void Canvas::SetupUIMouseInfo(UIMouseInfo& out_input)
{
	const Mouse& mouse = g_inputSystem->GetMouse();

	out_input.m_leftClicked = mouse.WasButtonJustPressed(MOUSEBUTTON_LEFT);
	out_input.m_leftReleased = mouse.WasButtonJustReleased(MOUSEBUTTON_LEFT);
	out_input.m_leftHeld = mouse.IsButtonPressed(MOUSEBUTTON_LEFT);

	out_input.m_rightClicked = mouse.WasButtonJustPressed(MOUSEBUTTON_RIGHT);
	out_input.m_rightReleased = mouse.WasButtonJustReleased(MOUSEBUTTON_RIGHT);
	out_input.m_rightHeld = mouse.IsButtonPressed(MOUSEBUTTON_RIGHT);

	out_input.m_position = GetMousePosition();
	out_input.m_lastFramePosition = GetMousePositionLastFrame();
	out_input.m_cursorCanvasDelta = (out_input.m_position - out_input.m_lastFramePosition);

	if (out_input.m_leftClicked)
	{
		// Start a new hold
		out_input.m_leftHoldStartPosition = out_input.m_position;
	}
	else if (out_input.m_leftHeld)
	{
		// Preserve the start on this hold
		out_input.m_leftHoldStartPosition = m_lastFrameUIMouseInfo.m_leftHoldStartPosition;
	}

	if (out_input.m_rightClicked)
	{
		// Start a new hold
		out_input.m_rightHoldStartPosition = out_input.m_position;
	}
	else if (out_input.m_rightHeld)
	{
		// Preserve the start on this hold
		out_input.m_rightHoldStartPosition = m_lastFrameUIMouseInfo.m_rightHoldStartPosition;
	}

	out_input.m_leftHoldDelta = out_input.m_position - out_input.m_leftHoldStartPosition;
	out_input.m_rightHoldDelta = out_input.m_position - out_input.m_rightHoldStartPosition;

	out_input.m_mouseWheelDelta = mouse.GetMouseWheelDelta();
}


//-------------------------------------------------------------------------------------------------
void Canvas::FindMouseHoveredElements(const Vector2& cursorCanvasPos, std::vector<UIElement*>& out_hoverStack) const
{
	std::map<StringID, UIElement*>::const_iterator itr = m_globalElementMap.begin();
	for (itr; itr != m_globalElementMap.end(); itr++)
	{
		UIElement* currElement = itr->second;
		OBB2 elementCanvasBounds = currElement->GetCanvasBounds();

		if (elementCanvasBounds.IsPointInside(cursorCanvasPos))
		{
			out_hoverStack.push_back(currElement);
		}
	}

	// Top layer elements get priority
	std::sort(out_hoverStack.begin(), out_hoverStack.end(), CompareByLayerDescending);
}


//-------------------------------------------------------------------------------------------------
void Canvas::HandleMouseJustHovers(const std::vector<UIElement*>& hoverStack, const UIMouseInfo& mouseInfo)
{
	for (size_t hoverIndex = 0; hoverIndex < hoverStack.size(); ++hoverIndex)
	{
		UIElement* currElement = hoverStack[hoverIndex];

		if (WasHoveredLastFrame(currElement))
		{
			break;
		}
		
		if (CheckAndExecuteHandler(currElement, currElement->m_onJustHovered, mouseInfo))
		{
			break;
		}
	}
}


//-------------------------------------------------------------------------------------------------
void Canvas::HandleMouseHovers(const std::vector<UIElement*>& hoverStack, const UIMouseInfo& mouseInfo)
{
	for (size_t hoverIndex = 0; hoverIndex < hoverStack.size(); ++hoverIndex)
	{
		UIElement* currElement = hoverStack[hoverIndex];

		if (CheckAndExecuteHandler(currElement, currElement->m_onHover, mouseInfo))
		{
			break;
		}
	}
}


//-------------------------------------------------------------------------------------------------
void Canvas::HandleMouseClicks(const std::vector<UIElement*>& hoverStack, const UIMouseInfo& mouseInfo)
{
	bool justClicked = mouseInfo.m_leftClicked || mouseInfo.m_rightClicked;
	bool justReleased = mouseInfo.m_leftReleased || mouseInfo.m_rightReleased;
	bool isPressed = mouseInfo.m_leftHeld || mouseInfo.m_rightHeld;

	// Just clicks
	if (justClicked)
	{
		bool inputConsumed = false;
		for (size_t hoverIndex = 0; hoverIndex < hoverStack.size(); ++hoverIndex)
		{
			UIElement* currElement = hoverStack[hoverIndex];

			inputConsumed = CheckAndExecuteHandler(currElement, currElement->m_onMouseClick, mouseInfo);
			if (inputConsumed)
			{
				// Consumed/Blocked input, cache it off for next frame
				m_currentClickedElement = currElement;

				break;
			}	
		}
	}

	if (justReleased && m_currentClickedElement != nullptr)
	{
		CheckAndExecuteHandler(m_currentClickedElement, m_currentClickedElement->m_onMouseRelease, mouseInfo);
		m_currentClickedElement = nullptr;
	}

	// Call this even if justClicked was done above - let both get called
	if (isPressed && m_currentClickedElement != nullptr)
	{
		CheckAndExecuteHandler(m_currentClickedElement, m_currentClickedElement->m_onMouseHold, mouseInfo);
	}
}


//-------------------------------------------------------------------------------------------------
void Canvas::HandleMouseUnhovers(const std::vector<UIElement*>& hoverStack, const UIMouseInfo& mouseInfo)
{
	// Check all elements hovered last frame, and if they're no longer hovered then call the unhover handlers
	for (size_t elementIndex = 0; elementIndex < m_elementsHoveredLastFrame.size(); ++elementIndex)
	{
		UIElement* elementHoveredLastFrame = m_elementsHoveredLastFrame[elementIndex];
		bool notHoveredThisFrame = (std::find(hoverStack.begin(), hoverStack.end(), elementHoveredLastFrame) == hoverStack.end());
		
		if (notHoveredThisFrame)
		{
			// Unhovers don't consume any input
			CheckAndExecuteHandler(elementHoveredLastFrame, elementHoveredLastFrame->m_onUnhovered, mouseInfo);
		}
	}
}
