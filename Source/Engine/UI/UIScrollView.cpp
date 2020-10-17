///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: October 11th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/UI/UIScrollView.h"

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
UIScrollView::UIScrollView(Canvas* canvas)
	: UIElement(canvas)
	, m_scrollTransform(canvas)
{
	m_scrollTransform.SetParentTransform(&m_transform);
	m_scrollTransform.SetAnchors(AnchorPreset::TOP_LEFT);
	m_scrollTransform.SetPivot(Vector2(0.f, 1.0f));
	m_scrollTransform.SetPosition(Vector2(0.f, 0.f));
}


//-------------------------------------------------------------------------------------------------
void UIScrollView::Render()
{
	// Need to ensure if the ScrollView's dimensions get changed that the scroll field here also gets updated
	m_scrollTransform.SetDimensions(m_transform.GetDimensions());

	UIElement::Render();
}


//-------------------------------------------------------------------------------------------------
void UIScrollView::AddChildToScroll(UIElement* element)
{
	AddChild(element);
	
	// Set up the transform to work correctly with the other elements
	element->m_transform.SetParentTransform(&m_scrollTransform);
	element->m_transform.SetAnchors(AnchorPreset::TOP_LEFT);
	element->m_transform.SetPivot(Vector2(0.f, 1.0f));
	element->m_transform.SetXPosition(0.f);
	element->m_transform.SetYPosition(GetBottomOfListY());
	
	m_scrollableElements.push_back(element);
}


//-------------------------------------------------------------------------------------------------
float UIScrollView::GetBottomOfListY() const
{
	if (m_scrollableElements.size() == 0)
	{
		return 0.f;
	}

	size_t lastIndex = m_scrollableElements.size() - 1;
	UIElement* lastElement = m_scrollableElements[lastIndex];

	// Y position should be <= 0.f
	return lastElement->m_transform.GetYPosition() - lastElement->m_transform.GetHeight();
}

