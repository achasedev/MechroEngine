///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: October 10th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once
#include "Engine/DataStructures/ColoredText.h"
#include "Engine/Render/Buffer/ConstantBuffer.h"
#include "Engine/UI/UIElement.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Font;
class UIButton;
class UIImage;
class UIPanel;
class UIScrollbar;
class UIText;

enum VerticalScrollbarType
{
	NO_VERTICAL_SCROLLBAR,
	VERTICAL_SCROLLBAR_LEFT,
	VERTICAL_SCROLLBAR_RIGHT
};

enum HorizontalScrollbarType
{
	NO_HORIZONTAL_SCROLLBAR,
	HORIZONTAL_SCROLLBAR_BOTTOM,
	HORIZONTAL_SCROLLBAR_TOP
};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class UIScrollView : public UIElement
{
public:
	//-----Public Methods-----
	RTTI_DERIVED_CLASS(UIScrollView);

	UIScrollView(Canvas* canvas, const StringID& id);
	virtual ~UIScrollView();

	virtual void	InitializeFromXML(const XMLElem& element) override;
	virtual void	Update() override;
	virtual void	Render() override;

	void			SetFont(Font* font);
	void			SetFontHeight(float height);
	int 			AddTextToScroll(const std::string& text, Rgba color = Rgba::WHITE);
	int 			AddTextToScroll(const ColoredText& coloredText);
	void			RemoveLineFromScroll(int lineIndex);
	void			ScrollVerticalWithTranslation(float translation);
	void			ScrollHorizontalWithTranslation(float translation);
	void			ScrollFromVerticalSlider(float deltaScroll);
	void			ScrollFromHorizontalSlider(float deltaScroll);

	AABB2			GetLocalViewBounds() const;
	float			GetScrollSpeed() const { return m_scrollSpeed; }
	UIText*			GetScrollTextElement() const { return m_textElement; }


private:
	//-----Private Methods-----

	void			CreateViewPanel();
	void			CreateVerticalScrollbar();
	void			CreateHorizontalScrollbar();
	void			SetupInitialTransforms();

	void			UpdateVerticalSlider();
	void			UpdateHorizontalSlider();
	

private:
	//-----Private Data-----
	
	UIPanel*					m_viewPanel = nullptr;
	UIText*						m_textElement = nullptr;
	float						m_scrollSpeed = 50.f;
	float						m_buttonSize = 10.f;

	// Vertical
	UIPanel*					m_verticalPanel = nullptr;
	UIButton*					m_downButton = nullptr;
	UIButton*					m_upButton = nullptr;
	UIImage*					m_verticalSlider = nullptr;
	VerticalScrollbarType		m_verticalScrollbarType = NO_VERTICAL_SCROLLBAR;

	// Horizontal
	UIPanel*					m_horizontalPanel = nullptr;
	UIButton*					m_leftButton = nullptr;
	UIButton*					m_rightButton = nullptr;
	UIImage*					m_horizontalSlider = nullptr;
	HorizontalScrollbarType		m_horizontalScrollbarType = NO_HORIZONTAL_SCROLLBAR;

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
