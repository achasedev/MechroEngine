///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: October 17th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/IO/Image.h"
#include "Engine/UI/UIButton.h"
#include "Engine/UI/UIImage.h"
#include "Engine/UI/UIScrollbar.h"
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
RTTI_TYPE_DEFINE(UIScrollbar);

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
static bool OnHold_MinButton(UIElement* element, const UIMouseInfo& info)
{
	UIScrollbar* bar = element->GetParent()->GetAsType<UIScrollbar>();
	bar->Scroll(info.m_deltaSeconds);

	return true;
}


//-------------------------------------------------------------------------------------------------
static bool OnHold_MaxButton(UIElement* element, const UIMouseInfo& info)
{
	UIScrollbar* bar = element->GetParent()->GetAsType<UIScrollbar>();
	bar->Scroll(-info.m_deltaSeconds);

	return true;
}

//-------------------------------------------------------------------------------------------------
static bool OnHold_ScrollArea(UIElement* element, const UIMouseInfo& info)
{
	UIScrollbar* bar = element->GetAsType<UIScrollbar>();
	UIImage* slider = bar->GetSliderElement();

	if (info.m_position.y < slider->m_transform.GetBounds().m_alignedBounds.mins.y)
	{
		bar->Scroll(info.m_deltaSeconds);
	}
	else if (info.m_position.y > slider->m_transform.GetBounds().m_alignedBounds.mins.y + slider->m_transform.GetHeight())
	{
		bar->Scroll(-info.m_deltaSeconds);
	}

	return true;
}


//-------------------------------------------------------------------------------------------------
static bool OnHold_Slider(UIElement* element, const UIMouseInfo& info)
{
	UIScrollbar* bar = element->GetParent()->GetAsType<UIScrollbar>();
	float deltaY = info.m_cursorCanvasDelta.y;

	bar->ScrollFromSlider(-deltaY);

	return true;
}


//-------------------------------------------------------------------------------------------------
static bool OnMouseClick_Button(UIElement* element, const UIMouseInfo& info)
{
	UIButton* button = element->GetAsType<UIButton>();

	Image* image = new Image(IntVector2(2), Rgba::YELLOW);
	button->SetImage(image);

	return true;
}


//-------------------------------------------------------------------------------------------------
static bool OnMouseRelease_Button(UIElement* element, const UIMouseInfo& info)
{
	UIButton* button = element->GetAsType<UIButton>();

	Image* image = new Image(IntVector2(2), Rgba::BLUE);
	button->SetImage(image);

	return true;
}


//-------------------------------------------------------------------------------------------------
static bool OnMouseRelease_Slider(UIElement* element, const UIMouseInfo& info)
{
	UIImage* slider = element->GetAsType<UIImage>();

	Image* image = new Image(IntVector2(2), Rgba::RED);
	slider->SetImage(image);
	return true;
}


//-------------------------------------------------------------------------------------------------
static bool OnMouseClick_Slider(UIElement* element, const UIMouseInfo& info)
{
	UIImage* slider = element->GetAsType<UIImage>();

	Image* image = new Image(IntVector2(2), Rgba::YELLOW);
	slider->SetImage(image);
	return true;
}


//-------------------------------------------------------------------------------------------------
static ScrollDirection StringToScrollDirection(const std::string& text)
{
	if (text == "vertical") { return SCROLL_DIRECTION_VERTICAL; }
	else if (text == "horizontal") { return SCROLL_DIRECTION_HORIZONTAL; }
	else
	{
		ERROR_RECOVERABLE("Invalid ScrollDirection text!");
	}

	return SCROLL_DIRECTION_VERTICAL;
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
UIScrollbar::UIScrollbar(Canvas* canvas)
	: UIElement(canvas)
{

}


//-------------------------------------------------------------------------------------------------
UIScrollbar::~UIScrollbar()
{
	m_scrollView = nullptr; // I don't own this

	// These are in m_children, and will get deleted then
	m_minButton = nullptr;
	m_maxButton = nullptr;
	m_slider = nullptr;
}


//-------------------------------------------------------------------------------------------------
void UIScrollbar::InitializeFromXML(const XMLElem& element)
{
	UIElement::InitializeFromXML(element);

	std::string direction = XML::ParseAttribute(element, "direction", "vertical");
	m_scrollDirection = StringToScrollDirection(direction);

	m_thickness = XML::ParseAttribute(element, "thickness", m_thickness);
	m_scrollSpeed = XML::ParseAttribute(element, "scroll_speed", m_scrollSpeed);

	// TODO: Delete this from orbit
	Image* image1 = new Image(IntVector2(2), Rgba::BLUE);
	Image* image2 = new Image(IntVector2(2), Rgba::BLUE);
	Image* image3 = new Image(IntVector2(2), Rgba::RED);

	// Create the elements
	m_minButton = new UIButton(m_canvas);
	m_minButton->Initialize();
	m_minButton->m_transform.SetAnchors(AnchorPreset::BOTTOM_LEFT);
	m_minButton->m_transform.SetPosition(Vector2::ZERO);
	m_minButton->m_transform.SetPivot(Vector2::ZERO);
	m_minButton->m_transform.SetDimensions(Vector2(m_thickness));
	m_minButton->SetImage(image1);
	m_minButton->m_onMouseHold = OnHold_MinButton;
	m_minButton->m_onMouseClick = OnMouseClick_Button;
	m_minButton->m_onMouseRelease = OnMouseRelease_Button;

	m_maxButton = new UIButton(m_canvas);
	m_maxButton->Initialize();
	m_maxButton->m_transform.SetAnchors(AnchorPreset::TOP_LEFT);
	m_maxButton->m_transform.SetPosition(Vector2::ZERO);
	m_maxButton->m_transform.SetPivot(Vector2(0.f, 1.0f));
	m_maxButton->m_transform.SetDimensions(Vector2(m_thickness));
	m_maxButton->SetImage(image2);
	m_maxButton->m_onMouseHold = OnHold_MaxButton;
	m_maxButton->m_onMouseClick = OnMouseClick_Button;
	m_maxButton->m_onMouseRelease = OnMouseRelease_Button;

	m_slider = new UIImage(m_canvas);
	m_slider->m_transform.SetAnchors(AnchorPreset::BOTTOM_LEFT);
	m_slider->m_transform.SetPosition(Vector2::ZERO);
	m_slider->m_transform.SetPivot(Vector2::ZERO);
	m_slider->m_transform.SetDimensions(Vector2(m_thickness)); // Height will be set correctly in first Update()
	m_slider->SetImage(image3);
	m_slider->m_onMouseHold = OnHold_Slider;
	m_slider->m_onMouseClick = OnMouseClick_Slider;
	m_slider->m_onMouseRelease = OnMouseRelease_Slider;
	m_onMouseHold = OnHold_ScrollArea;

	AddChild(m_minButton);
	AddChild(m_maxButton);
	AddChild(m_slider);
}


//-------------------------------------------------------------------------------------------------
void UIScrollbar::Update()
{
	RectTransform* windowTransform = &m_scrollView->m_transform;
	RectTransform* scrollTransform = &m_scrollView->GetScrollTextElement()->m_transform;

	float windowSize = windowTransform->GetHeight();
	float totalSize = scrollTransform->GetHeight();

	if (windowSize >= totalSize)
	{
		// Slider takes the full size of the bar
		m_slider->m_transform.SetHeight(m_transform.GetHeight() - 2.f * m_thickness);
		m_slider->m_transform.SetYPosition(m_transform.GetYPosition() + m_thickness);
	}
	else
	{
		float amountOutsideView = totalSize - windowSize;

		float amountBelow = (windowTransform->GetYPosition() - scrollTransform->GetYPosition());
		float amountAbove = totalSize - windowSize - amountBelow;

		float sliderFraction = windowSize / amountOutsideView;
		float sliderHeight = (m_transform.GetHeight() - 2.f * m_thickness) * sliderFraction;

		m_slider->m_transform.SetHeight(sliderHeight);

		float posFraction = amountBelow / amountOutsideView;
		float maxOffset = (m_transform.GetHeight() - 2.f * m_thickness) - sliderHeight;

		float newY = m_transform.GetYPosition() + m_thickness + posFraction * maxOffset;

		newY = Clamp(newY, m_transform.GetYPosition(), m_transform.GetYPosition() + m_transform.GetHeight() - m_thickness);
		m_slider->m_transform.SetYPosition(newY);
	}
}


//-------------------------------------------------------------------------------------------------
void UIScrollbar::Render()
{
	UIElement::Render();
}


//-------------------------------------------------------------------------------------------------
void UIScrollbar::ScrollFromSlider(float sliderDeltaY)
{
	float slideableSpace = m_transform.GetHeight() - 2.f * m_thickness - m_slider->m_transform.GetHeight();
	float fraction = sliderDeltaY / slideableSpace;


	float outRange = m_scrollView->GetScrollTextElement()->m_transform.GetHeight();
	float deltaY = outRange * fraction;
	m_scrollView->Scroll(deltaY);
}


//-------------------------------------------------------------------------------------------------
void UIScrollbar::Scroll(float deltaSeconds)
{
	RectTransform* scrollTransform = &m_scrollView->GetScrollTextElement()->m_transform;

	switch (m_scrollDirection)
	{
	case SCROLL_DIRECTION_HORIZONTAL:
		ERROR_AND_DIE("not yet");
		break;
	case SCROLL_DIRECTION_VERTICAL:
		m_scrollView->Scroll(m_scrollSpeed * deltaSeconds);
		break;
	default:
		ERROR_RECOVERABLE("Invalid scroll direction!");
		break;
	}
}

