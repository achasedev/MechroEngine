///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: October 11th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
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
RTTI_TYPE_DEFINE(UIScrollView);

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
static bool OnHover_Scroll(UIElement* element, const UIMouseInfo& info)
{
	UIScrollView* scrollView = element->GetAsType<UIScrollView>();

	scrollView->Scroll(info.m_mouseWheelDelta * 50.f);

	return true;
}


//-------------------------------------------------------------------------------------------------
static bool OnHover_Child(UIElement* element, const UIMouseInfo& info)
{
	// Don't block input
	return false;
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
UIScrollView::UIScrollView(Canvas* canvas)
	: UIElement(canvas)
{
	m_onHover = OnHover_Scroll;
	SetupDefaultScrollText();
}


//-------------------------------------------------------------------------------------------------
UIScrollView::~UIScrollView()
{
	// These exist in m_children, so they will get deleted there
	m_verticalScrollBar = nullptr;
	m_horizontalScrollBar = nullptr;
	m_scrollableText = nullptr;
}


//-------------------------------------------------------------------------------------------------
void UIScrollView::InitializeFromXML(const XMLElem& element)
{
	UIElement::InitializeFromXML(element);

	// Check for scrollbars
	const XMLElem* vertScrollElem = element.FirstChildElement("vertical_scrollbar");
	if (vertScrollElem != nullptr)
	{
		m_verticalScrollBar = new UIScrollbar(m_canvas);
		m_verticalScrollBar->InitializeFromXML(*vertScrollElem);
	}

	const XMLElem* horizScrollElem = element.FirstChildElement("horizontal_scrollbar");
	if (horizScrollElem != nullptr)
	{
		m_horizontalScrollBar = new UIScrollbar(m_canvas);
		m_horizontalScrollBar->InitializeFromXML(*horizScrollElem);
	}

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
				m_scrollableText = m_children[childIndex]->GetAsType<UIText>();
			}
		}

		m_scrollableText->m_onHover = OnHover_Child;
	}
	else
	{
		SetupDefaultScrollText();
	}

	m_scrollSpeed = XML::ParseAttribute(element, "scroll_speed", m_scrollSpeed);
}


//-------------------------------------------------------------------------------------------------
void UIScrollView::Update()
{
	// In case we shouldn't have a horizontal scroll and need to update width
	if (!m_independentScrollWidth)
	{
		m_scrollableWidth = m_transform.GetWidth();
	}

	// TODO: Check for text changes
}


//-------------------------------------------------------------------------------------------------
void UIScrollView::Render()
{
	// Need to ensure if the ScrollView's dimensions get changed that the scroll field here also gets updated
	m_scrollableText->m_transform.SetDimensions(m_transform.GetDimensions());

	UIElement::Render();
}


//-------------------------------------------------------------------------------------------------
void UIScrollView::SetFont(Font* font)
{
	m_scrollableText->SetFont(font);
}


//-------------------------------------------------------------------------------------------------
void UIScrollView::SetFontHeight(float height)
{
	m_scrollableText->SetFontHeight(height);
}


//-------------------------------------------------------------------------------------------------
void UIScrollView::AddTextToScroll(const std::string& text)
{
	m_scrollableText->AddLine(text);
}


//-------------------------------------------------------------------------------------------------
void UIScrollView::Scroll(float translation)
{
	m_scrollableText->m_transform.TranslateY(translation);

	// If we have scroll bars, tell them to update too
}


//-------------------------------------------------------------------------------------------------
void UIScrollView::SetupDefaultScrollText()
{
	m_scrollableText = new UIText(m_canvas);
	m_scrollableText->m_transform.SetParentTransform(&m_transform);
	m_scrollableText->m_transform.SetAnchors(AnchorPreset::TOP_LEFT);
	m_scrollableText->m_transform.SetPivot(Vector2(0.f, 1.0f));
	m_scrollableText->m_transform.SetPosition(Vector2(0.f, 0.f));
	m_scrollableText->m_onHover = OnHover_Child;
	AddChild(m_scrollableText);

	m_independentScrollWidth = false; // Match my width
}

