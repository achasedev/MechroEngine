///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: October 18th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/UI/Canvas.h"
#include "Engine/UI/UIButton.h"
#include "Engine/UI/UIImage.h"
#include "Engine/UI/UIText.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------
RTTI_TYPE_DEFINE(UIButton);

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
static bool DontBlockInput(UIElement* element, const UIMouseInfo& info)
{
	UNUSED(element);
	UNUSED(info);

	return false;
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
UIButton::UIButton(Canvas* canvas, const StringId& id)
	: UIElement(canvas, id)
{

}


//-------------------------------------------------------------------------------------------------
UIButton::~UIButton()
{
	// These are in m_children and will be deleted automatically
	m_imageElement = nullptr;
	m_textElement = nullptr;
}


//-------------------------------------------------------------------------------------------------
void UIButton::Initialize()
{
	SetupDefaultImageElement();
	SetupDefaultTextElement();
}


//-------------------------------------------------------------------------------------------------
void UIButton::InitializeFromXML(const XMLElem& element)
{
	UIElement::InitializeFromXML(element);

	const XMLElem* imageElem = nullptr;
	const XMLElem* currChild = element.FirstChildElement();
	while (currChild != nullptr)
	{
		if (IsXMLElemForUIImage(*currChild))
		{
			if (imageElem != nullptr)
			{
				ERROR_RECOVERABLE("Element %s: Has more than one image element, using the first we found!", element.Name());
				break;
			}

			imageElem = currChild;
		}

		currChild = currChild->NextSiblingElement();
	}

	if (imageElem != nullptr)
	{
		// Cache off the Text for convenience
		for (size_t childIndex = 0; childIndex < m_children.size(); ++childIndex)
		{
			if (m_children[childIndex]->IsOfType<UIImage>())
			{
				m_imageElement = m_children[childIndex]->GetAsType<UIImage>();
			}
		}
	}
	else
	{
		SetupDefaultImageElement();
	}

	const XMLElem* textElem = nullptr;
	currChild = element.FirstChildElement();
	while (currChild != nullptr)
	{
		if (IsXMLElemForUIText(*currChild))
		{
			if (textElem != nullptr)
			{
				ERROR_RECOVERABLE("Element %s: Has more than one text element, using the first we found!", element.Name());
				break;
			}

			textElem = currChild;
		}

		currChild = currChild->NextSiblingElement();
	}

	if (textElem != nullptr)
	{
		// Cache off the Text for convenience
		for (size_t childIndex = 0; childIndex < m_children.size(); ++childIndex)
		{
			if (m_children[childIndex]->IsOfType<UIText>())
			{
				m_textElement = m_children[childIndex]->GetAsType<UIText>();
			}
		}
	}
	else
	{
		SetupDefaultTextElement();
	}
}


//-------------------------------------------------------------------------------------------------
void UIButton::Update()
{
	UIElement::Update();
}


//-------------------------------------------------------------------------------------------------
void UIButton::Render()
{
	// Button's two children are considered part of self.....
	if (ShouldRenderSelf())
	{
		UIElement::Render();
	}
}


//-------------------------------------------------------------------------------------------------
void UIButton::SetImage(Image* image)
{
	m_imageElement->SetImage(image);
}


//-------------------------------------------------------------------------------------------------
void UIButton::SetupDefaultImageElement()
{
	m_imageElement = new UIImage(m_canvas, m_canvas->GetNextUnspecifiedID());
	m_imageElement->m_transform.SetParentTransform(&m_transform);
	m_imageElement->m_transform.SetAnchors(AnchorPreset::STRETCH_ALL);
	m_imageElement->m_transform.SetPadding(0.f, 0.f, 0.f, 0.f);
	m_imageElement->m_onHover = DontBlockInput;
	m_imageElement->m_onJustHovered = DontBlockInput;
	m_imageElement->m_onUnhovered = DontBlockInput;
	m_imageElement->m_onMouseClick = DontBlockInput;
	m_imageElement->m_onMouseHold = DontBlockInput;
	m_imageElement->m_onMouseRelease = DontBlockInput;
	AddChild(m_imageElement);
}


//-------------------------------------------------------------------------------------------------
void UIButton::SetupDefaultTextElement()
{
	m_textElement = new UIText(m_canvas, m_canvas->GetNextUnspecifiedID());
	m_textElement->m_transform.SetParentTransform(&m_transform);
	m_textElement->m_transform.SetAnchors(AnchorPreset::STRETCH_ALL);
	m_textElement->m_transform.SetPadding(0.f, 0.f, 0.f, 0.f);
	m_textElement->m_onHover = DontBlockInput;
	m_textElement->m_onJustHovered = DontBlockInput;
	m_textElement->m_onUnhovered = DontBlockInput;
	m_textElement->m_onMouseClick = DontBlockInput;
	m_textElement->m_onMouseHold = DontBlockInput;
	m_textElement->m_onMouseRelease = DontBlockInput;
	AddChild(m_textElement);
}
