///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: October 11th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/IO/Image.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Render/Core/RenderContext.h"
#include "Engine/Render/Material.h"
#include "Engine/Render/Shader.h"
#include "Engine/UI/Canvas.h"
#include "Engine/UI/UIButton.h"
#include "Engine/UI/UIImage.h"
#include "Engine/UI/UIPanel.h"
#include "Engine/UI/UIScrollView.h"
#include "Engine/UI/UIText.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------
RTTI_TYPE_DEFINE(UIScrollView);

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
static bool OnHover_MouseWheelScroll(UIElement* element, const UIMouseInfo& info)
{
	if (info.m_mouseWheelDelta != 0.f)
	{
		UIScrollView* scrollView = element->GetAsType<UIScrollView>();
		scrollView->ScrollVerticalWithTranslation(-info.m_mouseWheelDelta * scrollView->GetScrollSpeed());
	}

	return true;
}


//-------------------------------------------------------------------------------------------------
static bool OnHover_Passthrough(UIElement* element, const UIMouseInfo& info)
{
	UNUSED(element);
	UNUSED(info);

	// Don't block input
	return false;
}


//-------------------------------------------------------------------------------------------------
static bool OnHold_DownButton(UIElement* element, const UIMouseInfo& info)
{
	UIScrollView* scrollView = element->GetParent()->GetParent()->GetAsType<UIScrollView>();
	scrollView->ScrollVerticalWithTranslation(info.m_deltaSeconds * scrollView->GetScrollSpeed());

	return true;
}


//-------------------------------------------------------------------------------------------------
static bool OnHold_UpButton(UIElement* element, const UIMouseInfo& info)
{
	UIScrollView* scrollView = element->GetParent()->GetParent()->GetAsType<UIScrollView>();
	scrollView->ScrollVerticalWithTranslation(-1.0f * info.m_deltaSeconds * scrollView->GetScrollSpeed());

	return true;
}


//-------------------------------------------------------------------------------------------------
static bool OnHold_LeftButton(UIElement* element, const UIMouseInfo& info)
{
	UIScrollView* scrollView = element->GetParent()->GetParent()->GetAsType<UIScrollView>();
	scrollView->ScrollHorizontalWithTranslation(info.m_deltaSeconds * scrollView->GetScrollSpeed());

	return true;
}


//-------------------------------------------------------------------------------------------------
static bool OnHold_RightButton(UIElement* element, const UIMouseInfo& info)
{
	UIScrollView* scrollView = element->GetParent()->GetParent()->GetAsType<UIScrollView>();
	scrollView->ScrollHorizontalWithTranslation(-1.0f * info.m_deltaSeconds * scrollView->GetScrollSpeed());

	return true;
}


//-------------------------------------------------------------------------------------------------
static bool OnHold_VerticalScrollbar(UIElement* element, const UIMouseInfo& info)
{
	UIPanel* panel				= element->GetAsType<UIPanel>();
	UIScrollView* scrollView	= panel->GetParent()->GetAsType<UIScrollView>();
	UIImage* slider				= panel->GetFirstChildOfType<UIImage>();

	OBB2 bounds = slider->m_transform.GetBounds();
	if (info.m_position.y < bounds.m_alignedBounds.mins.y)
	{
		scrollView->ScrollVerticalWithTranslation(info.m_deltaSeconds * scrollView->GetScrollSpeed());
	}
	else if (info.m_position.y > bounds.m_alignedBounds.mins.y + slider->m_transform.GetHeight())
	{
		scrollView->ScrollVerticalWithTranslation(-info.m_deltaSeconds * scrollView->GetScrollSpeed());
	}

	return true;
}


//-------------------------------------------------------------------------------------------------
static bool OnHold_HorizontalScrollbar(UIElement* element, const UIMouseInfo& info)
{
	UIPanel* panel = element->GetAsType<UIPanel>();
	UIScrollView* scrollView = panel->GetParent()->GetAsType<UIScrollView>();
	UIImage* slider = panel->GetFirstChildOfType<UIImage>();

	OBB2 bounds = slider->m_transform.GetBounds();
	if (info.m_position.x < bounds.m_alignedBounds.mins.x)
	{
		scrollView->ScrollHorizontalWithTranslation(info.m_deltaSeconds * scrollView->GetScrollSpeed());
	}
	else if (info.m_position.x > bounds.m_alignedBounds.mins.x + slider->m_transform.GetWidth())
	{
		scrollView->ScrollHorizontalWithTranslation(-info.m_deltaSeconds * scrollView->GetScrollSpeed());
	}

	return true;
}


//-------------------------------------------------------------------------------------------------
static bool OnHold_VerticalSlider(UIElement* element, const UIMouseInfo& info)
{
	UIImage* slider				= element->GetAsType<UIImage>();
	UIScrollView* scrollView	= slider->GetParent()->GetParent()->GetAsType<UIScrollView>();
	float deltaY				= info.m_cursorCanvasDelta.y;

	scrollView->ScrollFromVerticalSlider(-deltaY);

	return true;
}


//-------------------------------------------------------------------------------------------------
static bool OnHold_HorizontalSlider(UIElement* element, const UIMouseInfo& info)
{
	UIImage* slider = element->GetAsType<UIImage>();
	UIScrollView* scrollView = slider->GetParent()->GetParent()->GetAsType<UIScrollView>();
	float deltaX = info.m_cursorCanvasDelta.x;

	scrollView->ScrollFromHorizontalSlider(-deltaX);

	return true;
}


//-------------------------------------------------------------------------------------------------
static bool OnMouseClick_Button(UIElement* element, const UIMouseInfo& info)
{
	UNUSED(info);

	UIButton* button = element->GetAsType<UIButton>();
	Image* image = new Image(IntVector2(2), Rgba::YELLOW);
	button->SetImage(image);

	return true;
}


//-------------------------------------------------------------------------------------------------
static bool OnMouseRelease_Button(UIElement* element, const UIMouseInfo& info)
{
	UNUSED(info);

	UIButton* button = element->GetAsType<UIButton>();
	Image* image = new Image(IntVector2(2), Rgba::BLUE);
	button->SetImage(image);

	return true;
}


//-------------------------------------------------------------------------------------------------
static bool OnMouseRelease_Slider(UIElement* element, const UIMouseInfo& info)
{
	UNUSED(info);

	UIImage* slider = element->GetAsType<UIImage>();
	Image* image = new Image(IntVector2(2), Rgba::RED);
	slider->SetImage(image);

	return true;
}


//-------------------------------------------------------------------------------------------------
static bool OnMouseClick_Slider(UIElement* element, const UIMouseInfo& info)
{
	UNUSED(info);

	UIImage* slider = element->GetAsType<UIImage>();
	Image* image = new Image(IntVector2(2), Rgba::YELLOW);
	slider->SetImage(image);

	return true;
}


//-------------------------------------------------------------------------------------------------
void ShrinkTextElementBoundsToFit(UIText* textElement)
{
	// NOTE: Word wrap does not work with this, as it is done in the render step and can add an
	//		 unknown number of lines to it
	float totalHeight = textElement->GetTotalLinesHeight();
	textElement->m_transform.SetHeight(totalHeight);

	float maxLineLength = textElement->GetMaxLineLength();
	textElement->m_transform.SetWidth(maxLineLength);
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
UIScrollView::UIScrollView(Canvas* canvas)
	: UIElement(canvas)
{
	m_onHover = OnHover_MouseWheelScroll;
}


//-------------------------------------------------------------------------------------------------
UIScrollView::~UIScrollView()
{
	// These exist in m_children, so they will get deleted there
	m_downButton = nullptr;
	m_upButton = nullptr;
	m_leftButton = nullptr;
	m_rightButton = nullptr;
	m_verticalSlider = nullptr;
	m_horizontalSlider = nullptr;
	m_verticalPanel = nullptr;
	m_horizontalPanel = nullptr;
	m_textElement = nullptr;
}


//-------------------------------------------------------------------------------------------------
void UIScrollView::InitializeFromXML(const XMLElem& element)
{
	UIElement::InitializeFromXML(element);

	// Still set up some sort of default
	const XMLElem* scrollTextElem = nullptr;
	const XMLElem* currChild = element.FirstChildElement();
	while (currChild != nullptr)
	{
		if (IsXMLElemForUIText(*currChild))
		{
			if (scrollTextElem != nullptr)
			{
				ERROR_RECOVERABLE("Element %s: Has more than one text element, using the first we found!", element.Name());
				break;
			}

			scrollTextElem = currChild;
		}

		currChild = currChild->NextSiblingElement();
	}

	if (scrollTextElem != nullptr)
	{
		// Cache off the Text for convenience
		for (size_t childIndex = 0; childIndex < m_children.size(); ++childIndex)
		{
			if (m_children[childIndex]->IsOfType<UIText>())
			{
				m_textElement = m_children[childIndex]->GetAsType<UIText>();
			}
		}

		// Ensure it's not word wrapped
		ASSERT_RECOVERABLE(m_textElement->GetTextDrawMode() != TEXT_DRAW_WORD_WRAP, "Cannot word wrap with scrollable text!");
		m_textElement->SetTextDrawMode(TEXT_DRAW_DEFAULT);
		m_textElement->m_onHover = OnHover_Passthrough;
	}
	else
	{
		SetupDefaultScrollText();
	}

	m_buttonSize = XML::ParseAttribute(element, "button_size", m_buttonSize);
	m_scrollSpeed = XML::ParseAttribute(element, "scroll_speed", m_scrollSpeed);

	CreateVerticalScrollbar();
	CreateHorizontalScrollbar();
	SetupInitialTransforms();
}


//-------------------------------------------------------------------------------------------------
void UIScrollView::Update()
{
	// Also update height to be based on number of lines in the log
	ShrinkTextElementBoundsToFit(m_textElement);

	// TODO: Check for text changes

	UIElement::Update();
}


//-------------------------------------------------------------------------------------------------
void UIScrollView::Render()
{
	OBB2 canvasBounds = GetCanvasBounds();
	canvasBounds.m_alignedBounds.mins.y += m_buttonSize;

	float screenLeft = (float)m_canvas->ToPixelWidth(canvasBounds.m_alignedBounds.left + m_buttonSize);
	float screenTop = (float)(m_canvas->ToPixelHeight(m_canvas->GetResolution().y) - m_canvas->ToPixelHeight(canvasBounds.m_alignedBounds.top));
	float screenRight = (float)m_canvas->ToPixelWidth(canvasBounds.m_alignedBounds.right);
	float screenBottom = (float)(m_canvas->ToPixelHeight(m_canvas->GetResolution().y) - m_canvas->ToPixelHeight(canvasBounds.m_alignedBounds.bottom));
	AABB2 screenRect = AABB2(screenLeft, screenTop, screenRight, screenBottom);

	// TODO: Fix this with a shader instance on text
	m_textElement->GetMaterial()->GetShader()->EnableScissor(screenRect);
	UIElement::Render();
	m_textElement->GetMaterial()->GetShader()->DisableScissor();
}


//-------------------------------------------------------------------------------------------------
void UIScrollView::SetFont(Font* font)
{
	m_textElement->SetFont(font);
}


//-------------------------------------------------------------------------------------------------
void UIScrollView::SetFontHeight(float height)
{
	m_textElement->SetFontHeight(height);
}


//-------------------------------------------------------------------------------------------------
void UIScrollView::AddTextToScroll(const std::string& text)
{
	m_textElement->AddLine(text);
}


//-------------------------------------------------------------------------------------------------
void UIScrollView::ScrollVerticalWithTranslation(float translation)
{
	if (translation != 0.f)
	{
		// Clamp the scroll from moving out of the view
		float newY = m_textElement->m_transform.GetYPosition() + translation;

		float viewHeight = m_transform.GetHeight() - m_buttonSize;
		float total = m_textElement->m_transform.GetHeight();

		if (total > viewHeight)
		{
			newY = Clamp(newY, m_buttonSize - (total - viewHeight), m_buttonSize);
			m_textElement->m_transform.SetYPosition(newY);
		}

		UpdateVerticalSlider();
	}
}


//-------------------------------------------------------------------------------------------------
void UIScrollView::ScrollHorizontalWithTranslation(float translation)
{
	if (translation != 0.f)
	{
		// Clamp the scroll from moving out of the view
		float newX = m_textElement->m_transform.GetXPosition() + translation;

		float viewWidth = m_transform.GetWidth() - m_buttonSize;
		float total = m_textElement->m_transform.GetWidth();

		if (total > viewWidth)
		{
			newX = Clamp(newX, m_transform.GetXPosition() + m_buttonSize - (total - viewWidth), m_buttonSize);
			m_textElement->m_transform.SetXPosition(newX);
		}

		UpdateHorizontalSlider();
	}
}


//-------------------------------------------------------------------------------------------------
void UIScrollView::ScrollFromVerticalSlider(float deltaScroll)
{
	float sliderRange = m_transform.GetHeight() - 2.f * m_buttonSize - m_verticalSlider->m_transform.GetHeight();
	float fraction = deltaScroll / sliderRange;

	float outRange = m_textElement->m_transform.GetHeight();
	float deltaTranslation = outRange * fraction;
	ScrollVerticalWithTranslation(deltaTranslation);
}


//-------------------------------------------------------------------------------------------------
void UIScrollView::ScrollFromHorizontalSlider(float deltaScroll)
{
	float sliderRange = m_transform.GetWidth() - 2.f * m_buttonSize - m_verticalSlider->m_transform.GetWidth();
	float fraction = deltaScroll / sliderRange;

	float outRange = m_textElement->m_transform.GetWidth();
	float deltaTranslation = outRange * fraction;
	ScrollHorizontalWithTranslation(deltaTranslation);
}


//-------------------------------------------------------------------------------------------------
AABB2 UIScrollView::GetLocalViewBounds() const
{
	// Account for scrollbars, as they exist inside the scrollview's transform
	Vector2 bottomLeft = Vector2(m_buttonSize);
	Vector2 topRight = m_transform.GetDimensions();

	return AABB2(bottomLeft, topRight);
}


//-------------------------------------------------------------------------------------------------
void UIScrollView::SetupDefaultScrollText()
{
	m_textElement = new UIText(m_canvas);
	m_textElement->m_transform.SetParentTransform(&m_transform);
	m_textElement->m_transform.SetAnchors(AnchorPreset::TOP_LEFT);
	m_textElement->m_transform.SetPivot(Vector2(0.f, 1.0f));
	m_textElement->m_transform.SetPosition(Vector2(0.f, 0.f));
	m_textElement->m_onHover = OnHover_Passthrough;

	AddChild(m_textElement);
}


//-------------------------------------------------------------------------------------------------
void UIScrollView::CreateVerticalScrollbar()
{
	// TODO: Delete this from orbit
	Image* image1 = new Image(IntVector2(2), Rgba::BLUE);
	Image* image2 = new Image(IntVector2(2), Rgba::BLUE);
	Image* image3 = new Image(IntVector2(2), Rgba::RED);

	// Create the elements
	m_downButton = new UIButton(m_canvas);
	m_downButton->Initialize();
	m_downButton->m_transform.SetAnchors(AnchorPreset::BOTTOM_LEFT);
	m_downButton->m_transform.SetPosition(Vector2::ZERO);
	m_downButton->m_transform.SetPivot(Vector2::ZERO);
	m_downButton->m_transform.SetDimensions(Vector2(m_buttonSize));
	m_downButton->SetImage(image1);
	m_downButton->m_onMouseHold = OnHold_DownButton;
	m_downButton->m_onMouseClick = OnMouseClick_Button;
	m_downButton->m_onMouseRelease = OnMouseRelease_Button;
	m_downButton->m_onHover = OnHover_Passthrough;

	m_upButton = new UIButton(m_canvas);
	m_upButton->Initialize();
	m_upButton->m_transform.SetAnchors(AnchorPreset::TOP_LEFT);
	m_upButton->m_transform.SetPosition(Vector2::ZERO);
	m_upButton->m_transform.SetPivot(Vector2(0.f, 1.0f));
	m_upButton->m_transform.SetDimensions(Vector2(m_buttonSize));
	m_upButton->SetImage(image2);
	m_upButton->m_onMouseHold = OnHold_UpButton;
	m_upButton->m_onMouseClick = OnMouseClick_Button;
	m_upButton->m_onMouseRelease = OnMouseRelease_Button;
	m_upButton->m_onHover = OnHover_Passthrough;

	m_verticalSlider = new UIImage(m_canvas);
	m_verticalSlider->m_transform.SetAnchors(AnchorPreset::BOTTOM_LEFT);
	m_verticalSlider->m_transform.SetPosition(Vector2(0.f, m_buttonSize));
	m_verticalSlider->m_transform.SetPivot(Vector2::ZERO);
	m_verticalSlider->m_transform.SetDimensions(Vector2(m_buttonSize)); // Height will be set correctly in first Update()
	m_verticalSlider->SetImage(image3);

	m_verticalSlider->m_onMouseHold = OnHold_VerticalSlider;
	m_verticalSlider->m_onMouseClick = OnMouseClick_Slider;
	m_verticalSlider->m_onMouseRelease = OnMouseRelease_Slider;
	m_verticalSlider->m_onHover = OnHover_Passthrough;

	m_verticalPanel = new UIPanel(m_canvas);
	m_verticalPanel->m_transform.SetAnchors(AnchorPreset::BOTTOM_LEFT);
	m_verticalPanel->m_transform.SetPosition(Vector2(0.f, m_buttonSize));
	m_verticalPanel->m_transform.SetPivot(Vector2::ZERO);
	m_verticalPanel->m_transform.SetDimensions(Vector2(m_buttonSize, m_transform.GetHeight() - m_buttonSize));
	m_verticalPanel->m_onMouseHold = OnHold_VerticalScrollbar;
	m_verticalPanel->m_onHover = OnHover_Passthrough;

	m_verticalPanel->AddChild(m_downButton);
	m_verticalPanel->AddChild(m_upButton);
	m_verticalPanel->AddChild(m_verticalSlider);

	AddChild(m_verticalPanel);
}


//-------------------------------------------------------------------------------------------------
void UIScrollView::CreateHorizontalScrollbar()
{
	// TODO: Delete this from orbit
	Image* image1 = new Image(IntVector2(2), Rgba::BLUE);
	Image* image2 = new Image(IntVector2(2), Rgba::BLUE);
	Image* image3 = new Image(IntVector2(2), Rgba::RED);

	// Create the elements
	m_leftButton = new UIButton(m_canvas);
	m_leftButton->Initialize();
	m_leftButton->m_transform.SetAnchors(AnchorPreset::BOTTOM_LEFT);
	m_leftButton->m_transform.SetPosition(Vector2::ZERO);
	m_leftButton->m_transform.SetPivot(Vector2::ZERO);
	m_leftButton->m_transform.SetDimensions(Vector2(m_buttonSize));
	m_leftButton->SetImage(image1);
	m_leftButton->m_onMouseHold = OnHold_LeftButton;
	m_leftButton->m_onMouseClick = OnMouseClick_Button;
	m_leftButton->m_onMouseRelease = OnMouseRelease_Button;
	m_leftButton->m_onHover = OnHover_Passthrough;

	m_rightButton = new UIButton(m_canvas);
	m_rightButton->Initialize();
	m_rightButton->m_transform.SetAnchors(AnchorPreset::BOTTOM_RIGHT);
	m_rightButton->m_transform.SetPosition(Vector2::ZERO);
	m_rightButton->m_transform.SetPivot(Vector2(1.f, 0.f));
	m_rightButton->m_transform.SetDimensions(Vector2(m_buttonSize));
	m_rightButton->SetImage(image2);
	m_rightButton->m_onMouseHold = OnHold_RightButton;
	m_rightButton->m_onMouseClick = OnMouseClick_Button;
	m_rightButton->m_onMouseRelease = OnMouseRelease_Button;
	m_rightButton->m_onHover = OnHover_Passthrough;

	m_horizontalSlider = new UIImage(m_canvas);
	m_horizontalSlider->m_transform.SetAnchors(AnchorPreset::BOTTOM_LEFT);
	m_horizontalSlider->m_transform.SetPosition(Vector2(m_buttonSize, 0.f));
	m_horizontalSlider->m_transform.SetPivot(Vector2::ZERO);
	m_horizontalSlider->m_transform.SetDimensions(Vector2(m_buttonSize)); // Height will be set correctly in first Update()
	m_horizontalSlider->SetImage(image3);
	m_horizontalSlider->m_onMouseHold = OnHold_HorizontalSlider;
	m_horizontalSlider->m_onMouseClick = OnMouseClick_Slider;
	m_horizontalSlider->m_onMouseRelease = OnMouseRelease_Slider;
	m_horizontalSlider->m_onHover = OnHover_Passthrough;

	m_horizontalPanel = new UIPanel(m_canvas);
	m_horizontalPanel->m_transform.SetAnchors(AnchorPreset::BOTTOM_LEFT);
	m_horizontalPanel->m_transform.SetPosition(Vector2(m_buttonSize, 0.f));
	m_horizontalPanel->m_transform.SetPivot(Vector2::ZERO);
	m_horizontalPanel->m_transform.SetDimensions(Vector2(m_transform.GetWidth() - m_buttonSize, m_buttonSize));
	m_horizontalPanel->m_onMouseHold = OnHold_HorizontalScrollbar;
	m_horizontalPanel->m_onHover = OnHover_Passthrough;

	m_horizontalPanel->AddChild(m_leftButton);
	m_horizontalPanel->AddChild(m_rightButton);
	m_horizontalPanel->AddChild(m_horizontalSlider);

	AddChild(m_horizontalPanel);
}


//-------------------------------------------------------------------------------------------------
void UIScrollView::SetupInitialTransforms()
{
	// Offset text element to be inside the scrollbars
	AABB2 viewBounds = GetLocalViewBounds();
	m_textElement->m_transform.SetPosition(viewBounds.GetBottomLeft());

	ShrinkTextElementBoundsToFit(m_textElement);
	UpdateVerticalSlider();
	UpdateHorizontalSlider();
}


//-------------------------------------------------------------------------------------------------
void UIScrollView::UpdateVerticalSlider()
{
	float totalTextHeight = m_textElement->m_transform.GetHeight();
	float viewSize = m_verticalPanel->m_transform.GetHeight();
	float spaceBetweenButtons = viewSize - 2.f * m_buttonSize;
	float minSliderPos = m_buttonSize;

	if (totalTextHeight <= viewSize)
	{
		m_verticalSlider->m_transform.SetHeight(spaceBetweenButtons);
		m_verticalSlider->m_transform.SetYPosition(minSliderPos);
	}
	else
	{
		// Slider height
		float fractionInView = viewSize / totalTextHeight;
		float sliderHeight = spaceBetweenButtons * fractionInView;
		m_verticalSlider->m_transform.SetHeight(sliderHeight);

		// Slider position
		float amountBelow = m_buttonSize - m_textElement->m_transform.GetYPosition();
		float sliderOffset = (amountBelow / totalTextHeight) * spaceBetweenButtons;
		float yPosition = minSliderPos + sliderOffset;
		m_verticalSlider->m_transform.SetYPosition(yPosition);
	}
}


//-------------------------------------------------------------------------------------------------
void UIScrollView::UpdateHorizontalSlider()
{
	float totalTextWidth = m_textElement->m_transform.GetWidth();
	float viewSize = m_horizontalPanel->m_transform.GetWidth();
	float spaceBetweenButtons = viewSize - 2.f * m_buttonSize;
	float minSliderPos = m_buttonSize;

	if (totalTextWidth <= viewSize)
	{
		m_horizontalSlider->m_transform.SetWidth(spaceBetweenButtons);
		m_horizontalSlider->m_transform.SetXPosition(minSliderPos);
	}
	else
	{
		// Slider width
		float fractionInView = viewSize / totalTextWidth;
		float sliderWidth = spaceBetweenButtons * fractionInView;
		m_horizontalSlider->m_transform.SetWidth(sliderWidth);

		// Slider position
		float amountToLeft = m_buttonSize - m_textElement->m_transform.GetXPosition();
		float sliderOffset = (amountToLeft / totalTextWidth) * spaceBetweenButtons;
		float xPosition = minSliderPos + sliderOffset;
		m_horizontalSlider->m_transform.SetXPosition(xPosition);
	}
}

