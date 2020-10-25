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
	TEXT_DRAW_DEFAULT,
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
	RTTI_DERIVED_CLASS(UIText);

	UIText(Canvas* canvas);
	virtual ~UIText();

	virtual void	InitializeFromXML(const XMLElem& element) override;
	virtual void	Render() override;
	
	void			SetText(const std::string& text, const Rgba& color = Rgba::WHITE);
	void			SetText(uint32 lineNumber, const std::string& text, const Rgba& color = Rgba::WHITE);
	void			AddLine(const std::string& text, const Rgba& color = Rgba::WHITE);
	void			AddLines(const std::vector<std::string>& lines, const Rgba& color = Rgba::WHITE);
	void			SetFont(Font* font);
	void			SetShader(Shader* shader);
	void			SetFontHeight(float fontHeight);
	void			SetTextDrawMode(TextDrawMode drawMode) { m_textDrawMode = drawMode; }
	void			MarkDirty() { m_isDirty = true; }
	void			SetTextAlignment(HorizontalAlignment horizAlign, VerticalAlignment vertAlign);
	void			SetTextHorizontalAlignment(HorizontalAlignment horizAlign);
	void			SetTextVerticalAlignment(VerticalAlignment vertAlign);

	std::string		GetText() const;
	std::string		GetText(uint32 lineNumber) const;
	uint32			GetNumLines() const { return (uint32) m_lines.size(); }
	float			GetLineHeight() const;
	float			GetTotalLinesHeight() const;
	float			GetMaxLineLength() const;
	Material*		GetMaterial() const { return m_material; }
	Font*			GetFont() const { return m_font; }
	TextDrawMode	GetTextDrawMode() const { return m_textDrawMode; }


private:
	//-----Private Data-----

	uint32			CalculatePixelHeightForBounds(const OBB2& finalBounds);
	void			UpdateMeshAndMaterial(const OBB2& finalBounds);


private:
	//-----Private Data-----

	std::vector<std::string>	m_lines;
	Rgba						m_textColor;
	Mesh*						m_mesh = nullptr;
	Material*					m_material = nullptr;
	Font*						m_font = nullptr;
	float						m_fontHeight = 0.f;
	bool						m_isDirty = true;
	float						m_boundsHeightLastDraw = -1.0f;

	HorizontalAlignment m_horizontalAlign	= ALIGNMENT_LEFT;
	VerticalAlignment m_verticalAlign		= ALIGNMENT_TOP;
	TextDrawMode m_textDrawMode				= TEXT_DRAW_DEFAULT;

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

bool IsValidHorizontalAlignment(const std::string& text);
bool IsValidVerticalAlignment(const std::string& text);
void GetTextAlignmentFromXML(const XMLElem& element, HorizontalAlignment& out_horizAlign, VerticalAlignment& out_vertAlign);
