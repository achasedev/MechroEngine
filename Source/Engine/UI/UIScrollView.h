///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: October 10th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once
#include "Engine/Render/Buffer/UniformBuffer.h"
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

	UIScrollView(Canvas* canvas);
	virtual ~UIScrollView();

	virtual void	InitializeFromXML(const XMLElem& element) override;
	virtual void	Update() override;
	virtual void	Render() override;

	void			SetFont(Font* font);
	void			SetFontHeight(float height);
	void			AddTextToScroll(const std::string& text);
	void			ScrollVerticalWithTranslation(float translation);
	void			ScrollHorizontalWithTranslation(float translation);
	void			ScrollFromVerticalSlider(float deltaScroll);
	void			ScrollFromHorizontalSlider(float deltaScroll);

	AABB2			GetLocalViewBounds() const;
	float			GetScrollSpeed() const { return m_scrollSpeed; }
	UIText*			GetScrollTextElement() const { return m_textElement; }


private:
	//-----Private Methods-----

	void			SetupDefaultScrollText();
	void			CreateViewPanel();
	void			CreateVerticalScrollbar();
	void			CreateHorizontalScrollbar();
	void			SetupInitialTransforms();

	void			UpdateVerticalSlider();
	void			UpdateHorizontalSlider();
	

private:
	//-----Private Data-----
	
	UIPanel*		m_viewPanel = nullptr;
	UIText*			m_textElement = nullptr;
	float			m_scrollSpeed = 50.f;
	float			m_buttonSize = 10.f;

	// Vertical
	UIPanel*		m_verticalPanel = nullptr;
	UIButton*		m_downButton = nullptr;
	UIButton*		m_upButton = nullptr;
	UIImage*		m_verticalSlider = nullptr;

	// Horizontal
	UIPanel*		m_horizontalPanel = nullptr;
	UIButton*		m_leftButton = nullptr;
	UIButton*		m_rightButton = nullptr;
	UIImage*		m_horizontalSlider = nullptr;

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
