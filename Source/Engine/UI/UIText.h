///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 21st, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/Rgba.h"
#include "Engine/UI/UIElement.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Font;
class OBB2;
class Shader;

enum HorizontalAlignment
{
	ALIGNMENT_LEFT,
	ALIGNMENT_CENTER,
	ALIGNMENT_RIGHT
};

enum VerticalAlignment
{
	ALIGNMENT_TOP,
	ALIGNMENT_MIDDLE,
	ALIGNMENT_BOTTOM
};

enum TextDrawMode
{
	TEXT_DRAW_OVERRUN,
	TEXT_DRAW_SHRINK_TO_FIT,
	TEXT_DRAW_EXPAND_TO_FILL,
	TEXT_DRAW_WORD_WRAP
};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class UIText : public UIElement
{
public:
	//-----Public Methods-----

	UIText(Canvas* canvas);
	virtual ~UIText();

	virtual void	InitializeFromXML(const XMLElem& element) override;
	virtual void	Render() override;
	
	void			SetText(const std::string& text, const Rgba& color = Rgba::WHITE);
	void			SetFont(Font* font);
	void			SetShader(Shader* shader);
	
	virtual void*	GetType() const override { return &s_type; }

	static void*	GetTypeStatic() { return &s_type; }


private:
	//-----Private Data-----

	uint32			CalculatePixelHeightForBounds(const OBB2& finalBounds);
	void			UpdateMeshAndMaterial(const OBB2& finalBounds);


private:
	//-----Private Data-----

	std::string m_text;
	Rgba		m_textColor;
	Mesh*		m_mesh = nullptr;
	Material*	m_material = nullptr;
	Font*		m_font = nullptr;
	float		m_fontHeight = 0.f;
	bool		m_isDirty = true;

	HorizontalAlignment m_horizontalAlign	= ALIGNMENT_LEFT;
	VerticalAlignment m_verticalAlign		= ALIGNMENT_TOP;
	TextDrawMode m_wrapMode					= TEXT_DRAW_WORD_WRAP;

	static int s_type;

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
