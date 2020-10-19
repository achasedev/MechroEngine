///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: October 11th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Math/MathUtils.h"
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
	if (info.m_mouseWheelDelta != 0.f)
	{
		UIScrollView* scrollView = element->GetAsType<UIScrollView>();
		scrollView->Scroll(-info.m_mouseWheelDelta * scrollView->GetScrollSpeed());
	}

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

		// Ensure it's not word wrapped
		ASSERT_RECOVERABLE(m_scrollableText->GetTextDrawMode() != TEXT_DRAW_WORD_WRAP, "Cannot word wrap with scrollable text!");
		m_scrollableText->SetTextDrawMode(TEXT_DRAW_DEFAULT);

		m_scrollableText->m_onHover = OnHover_Child;
	}
	else
	{
		SetupDefaultScrollText();
	}

	m_scrollSpeed = XML::ParseAttribute(element, "scroll_speed", m_scrollSpeed);

	// Cache off scrollbar
	for (size_t childIndex = 0; childIndex < m_children.size(); ++childIndex)
	{
		if (m_children[childIndex]->GetID() == SID("vertical_scrollbar"))
		{
			m_verticalScrollBar = m_children[childIndex]->GetAsType<UIScrollbar>();

			m_transform.SetWidth(m_transform.GetWidth() - m_verticalScrollBar->m_transform.GetWidth());
			m_transform.TranslateX(m_verticalScrollBar->m_transform.GetWidth());
			m_verticalScrollBar->m_scrollView = this;
			m_verticalScrollBar->m_transform.TranslateX(-m_verticalScrollBar->m_transform.GetWidth());
		}
	}
}


//-------------------------------------------------------------------------------------------------
void UIScrollView::Update()
{
	// In case we shouldn't have a horizontal scroll and need to update width
	if (!m_independentScrollWidth)
	{
		m_scrollableText->m_transform.SetWidth(m_transform.GetWidth());
	}

	// Also update height to be based on number of lines in the log
	// NOTE: Word wrap does not work with this, as it is done in the render step and can add an
	//		 unknown number of lines to it
	float totalHeight = m_scrollableText->GetLineHeight() * m_scrollableText->GetNumLines();
	m_scrollableText->m_transform.SetHeight(totalHeight);

	// TODO: Check for text changes

	UIElement::Update();
}


//-------------------------------------------------------------------------------------------------
void UIScrollView::Render()
{
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
	if (translation != 0.f)
	{
		// Clamp the scroll from moving out of the view
		float newY = m_scrollableText->m_transform.GetYPosition() + translation;

		float windowHeight = m_transform.GetHeight();
		float total = m_scrollableText->m_transform.GetHeight();

		if (total > windowHeight)
		{
			newY = Clamp(newY, m_transform.GetYPosition() - (total - windowHeight), 0.f);
			m_scrollableText->m_transform.SetYPosition(newY);
		}
	}
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

