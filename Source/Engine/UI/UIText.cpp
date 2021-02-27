///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 21st, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Render/Core/RenderContext.h"
#include "Engine/Render/Font/Font.h"
#include "Engine/Render/Font/FontAtlas.h"
#include "Engine/Render/Font/FontLoader.h"
#include "Engine/Render/Material.h"
#include "Engine/Render/Mesh/MeshBuilder.h"
#include "Engine/Render/Texture/Texture2D.h"
#include "Engine/UI/Canvas.h"
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
RTTI_TYPE_DEFINE(UIText);

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
static HorizontalAlignment StringToHorizontalAlignment(const std::string& text)
{
	if		(text == "left")	{ return ALIGNMENT_LEFT; }
	else if (text == "center")	{ return ALIGNMENT_CENTER; }
	else if (text == "right")	{ return ALIGNMENT_RIGHT; }
	else
	{
		ERROR_RECOVERABLE("Invalid HorizontalAlignment %s!", text.c_str());
		return ALIGNMENT_LEFT;
	}
}

//-------------------------------------------------------------------------------------------------
static VerticalAlignment StringToVerticalAlignment(const std::string& text)
{
	if		(text == "top")		{ return ALIGNMENT_TOP; }
	else if (text == "middle")	{ return ALIGNMENT_MIDDLE; }
	else if (text == "bottom")	{ return ALIGNMENT_BOTTOM; }
	else
	{
		ERROR_RECOVERABLE("Invalid VerticalAlignment %s!", text.c_str());
		return ALIGNMENT_BOTTOM;
	}
}


//-------------------------------------------------------------------------------------------------
static TextDrawMode StringToTextDrawMode(const std::string& text)
{
	if		(text == "default") { return TEXT_DRAW_DEFAULT; }
	else if (text == "shrink") { return TEXT_DRAW_SHRINK_TO_FIT; }
	else if (text == "expand") { return TEXT_DRAW_EXPAND_TO_FILL; }
	else if (text == "wrap") { return TEXT_DRAW_WORD_WRAP; }
	else
	{
		ERROR_RECOVERABLE("Invalid TextDrawMode %s!", text.c_str());
		return TEXT_DRAW_DEFAULT;
	}
}


//-------------------------------------------------------------------------------------------------
void GetTextAlignmentFromXML(const XMLElem& element, HorizontalAlignment& out_horizAlign, VerticalAlignment& out_vertAlign)
{
	bool xAlignSpecified = XML::DoesAttributeExist(element, "x_align");
	bool yAlignSpecified = XML::DoesAttributeExist(element, "y_align");
	bool combinedAlignSpecified = XML::DoesAttributeExist(element, "align");
	bool separateAlignsSpecified = xAlignSpecified && yAlignSpecified;

	GUARANTEE_OR_DIE((separateAlignsSpecified && !combinedAlignSpecified) || (!xAlignSpecified && !yAlignSpecified && combinedAlignSpecified), "Element %s: Duplicate aligns specified!", element.Name());

	if (combinedAlignSpecified)
	{
		std::string alignText = XML::ParseAttribute(element, "align", "");
		size_t underscoreIndex = alignText.find('_');

		if (underscoreIndex != std::string::npos)
		{
			// Split into the two specified alignments and try to figure out which is which
			std::string first = alignText.substr(0, underscoreIndex);
			std::string second = alignText.substr(underscoreIndex + 1);

			bool firstIsHorizontal = IsValidHorizontalAlignment(first);
			bool firstIsVertical = IsValidVerticalAlignment(first);
			bool secondIsHorizontal = IsValidHorizontalAlignment(second);
			bool secondIsVertical = IsValidVerticalAlignment(second);

			GUARANTEE_OR_DIE((firstIsHorizontal && secondIsVertical) || (secondIsHorizontal && firstIsVertical), "Element %s: Invalid align \"%s\" specified!", element.Name(), alignText.c_str());

			if (firstIsHorizontal)
			{
				out_horizAlign = StringToHorizontalAlignment(first);
				out_vertAlign = StringToVerticalAlignment(second);
			}
			else
			{
				out_horizAlign = StringToHorizontalAlignment(second);
				out_vertAlign = StringToVerticalAlignment(first);
			}
		}
		else
		{
			// Only one alignment specified, so figure out which was specified and default the other
			if (IsValidHorizontalAlignment(alignText))
			{
				std::string xAlignText = XML::ParseAttribute(element, "x_align", "left");
				out_horizAlign = StringToHorizontalAlignment(xAlignText);
				out_vertAlign = ALIGNMENT_BOTTOM;
			}
			else if (IsValidVerticalAlignment(alignText))
			{
				out_horizAlign = ALIGNMENT_LEFT;
				std::string yAlignText = XML::ParseAttribute(element, "y_align", "bottom");
				out_vertAlign = StringToVerticalAlignment(yAlignText);
			}
			else
			{
				ERROR_RECOVERABLE("Invalid align \"%s\" specified in element %s", alignText.c_str(), element.Name());
				out_horizAlign = ALIGNMENT_LEFT;
				out_vertAlign = ALIGNMENT_BOTTOM;
			}
		}
	}
	else
	{
		// Individuals were specified instead
		std::string xAlignText = XML::ParseAttribute(element, "x_align", "left");
		std::string yAlignText = XML::ParseAttribute(element, "y_align", "bottom");
		out_horizAlign = StringToHorizontalAlignment(xAlignText);
		out_vertAlign = StringToVerticalAlignment(yAlignText);
	}
}


//-------------------------------------------------------------------------------------------------
bool IsValidHorizontalAlignment(const std::string& text)
{
	if ((text == "left") || (text == "center") || (text == "right")) 
	{ 
		return true; 
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
bool IsValidVerticalAlignment(const std::string& text)
{
	if ((text == "top") || (text == "middle") || (text == "bottom"))
	{
		return true;
	}

	return false;
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
UIText::UIText(Canvas* canvas, const StringId& id)
	: UIElement(canvas, id)
{
	m_mesh = new Mesh();
	m_material = new Material();
}


//-------------------------------------------------------------------------------------------------
UIText::~UIText()
{
	SAFE_DELETE(m_material);
	SAFE_DELETE(m_mesh);
}


//-------------------------------------------------------------------------------------------------
void UIText::Render()
{
	if (ShouldRenderSelf() && m_lines.size() > 0)
	{
		// Check if the text or the scale changed which would require a rebuild
		OBB2 finalBounds = GetCanvasBounds();
		UpdateMeshAndMaterial(finalBounds);

		Renderable rend;
		rend.SetRenderableMatrix(CreateModelMatrix(finalBounds));
		rend.SetDrawMaterial(0, m_material);
		rend.SetDrawMesh(0, m_mesh);

		g_renderContext->DrawRenderable(rend);
	}

	UIElement::Render();
}


//-------------------------------------------------------------------------------------------------
void UIText::ClearText()
{
	m_lines.clear();
}


//-------------------------------------------------------------------------------------------------
void UIText::SetLine(int lineIndex, const std::string& text, const Rgba& color)
{
	if (lineIndex >= (int)m_lines.size())
	{
		m_lines.resize(lineIndex + 1);
	}

	m_lines[lineIndex] = ColoredText(text, color);
	m_isDirty = true;
}


//-------------------------------------------------------------------------------------------------
void UIText::SetText(const std::string& text, const Rgba& color)
{
	SetLine(0U, text, color);
}


//-------------------------------------------------------------------------------------------------
void UIText::SetLines(const std::vector<std::string>& lines, const Rgba& color)
{
	m_lines.clear();

	for (size_t lineIndex = 0; lineIndex < lines.size(); ++lineIndex)
	{
		m_lines.push_back(ColoredText(lines[lineIndex], color));
	}
}


//-------------------------------------------------------------------------------------------------
void UIText::SetText(const std::string& text)
{
	SetLine(0, text);
}


//-------------------------------------------------------------------------------------------------
void UIText::SetLine(int lineIndex, const std::string& text)
{
	if (lineIndex >= (int)m_lines.size())
	{
		m_lines.resize(lineIndex + 1);
	}

	m_lines[lineIndex].m_text = text;
	m_isDirty = true;
}


//-------------------------------------------------------------------------------------------------
void UIText::SetLines(const std::vector<std::string>& lines)
{
	m_lines.resize(lines.size());

	for (size_t lineIndex = 0; lineIndex < lines.size(); ++lineIndex)
	{
		m_lines[lineIndex].m_text = lines[lineIndex];
	}
}


//-------------------------------------------------------------------------------------------------
void UIText::SetColor(const Rgba& color)
{
	SetColor(0, color);
}


//-------------------------------------------------------------------------------------------------
void UIText::SetColor(int lineIndex, const Rgba& color)
{
	m_lines[lineIndex].m_color = color;
	m_isDirty = true;
}


//-------------------------------------------------------------------------------------------------
void UIText::AddLine(const std::string& text, const Rgba& color /*= Rgba::WHITE*/)
{
	m_lines.push_back(ColoredText(text, color));
	m_isDirty = true;
}


//-------------------------------------------------------------------------------------------------
void UIText::AddLines(const std::vector<std::string>& lines, const Rgba& color /*= Rgba::WHITE*/)
{
	for (size_t lineIndex = 0; lineIndex < lines.size(); ++lineIndex)
	{
		m_lines.push_back(ColoredText(lines[lineIndex], color));
	}
}


//-------------------------------------------------------------------------------------------------
void UIText::SetFont(Font* font)
{
	m_font = font;
	m_isDirty = true;
}


//-------------------------------------------------------------------------------------------------
void UIText::SetShader(Shader* shader)
{
	m_material->SetShader(shader);
}


//-------------------------------------------------------------------------------------------------
void UIText::SetFontHeight(float fontHeight)
{
	ASSERT_OR_DIE(fontHeight > 0.f, "Negative font height!");
	m_fontHeight = fontHeight;
}


//-------------------------------------------------------------------------------------------------
void UIText::SetTextAlignment(HorizontalAlignment horizAlign, VerticalAlignment vertAlign)
{
	SetTextHorizontalAlignment(horizAlign);
	SetTextVerticalAlignment(vertAlign);
}


//-------------------------------------------------------------------------------------------------
void UIText::SetTextHorizontalAlignment(HorizontalAlignment horizAlign)
{
	m_horizontalAlign = horizAlign;
	m_isDirty = true;
}


//-------------------------------------------------------------------------------------------------
void UIText::SetTextVerticalAlignment(VerticalAlignment vertAlign)
{
	m_verticalAlign = vertAlign;
	m_isDirty = true;
}


//-------------------------------------------------------------------------------------------------
std::string UIText::GetText() const
{
	return GetText(0U);
}


//-------------------------------------------------------------------------------------------------
std::string UIText::GetText(uint32 lineNumber) const
{
	if (m_lines.size() == 0)
	{
		return "";
	}

	ASSERT_RETURN(lineNumber < (uint32)m_lines.size(), "", "Index out of range!");
	return m_lines[lineNumber].m_text;
}


//-------------------------------------------------------------------------------------------------
std::string UIText::GetLastLine() const
{
	if (m_lines.size() == 0)
	{
		return "";
	}

	return m_lines.back().m_text;
}


//-------------------------------------------------------------------------------------------------
float UIText::GetLineHeight() const
{
	uint32 fontPixelHeight = m_canvas->ToPixelHeight(m_fontHeight * m_transform.GetScale().y);
	FontAtlas* atlas = m_font->CreateOrGetAtlasForPixelHeight(fontPixelHeight);

	return m_canvas->ToCanvasHeight(atlas->GetVerticalLineSpacingPixels());
}


//-------------------------------------------------------------------------------------------------
float UIText::GetTotalLinesHeight() const
{
	return GetLineHeight() * (float)m_lines.size();
}


//-------------------------------------------------------------------------------------------------
float UIText::GetMaxLineLength() const
{
	uint32 fontPixelHeight = m_canvas->ToPixelHeight(m_fontHeight);

	int maxLengthPixels = 0;
	for (size_t lineIndex = 0; lineIndex < m_lines.size(); ++lineIndex)
	{
		int lengthPixels = m_font->GetTextDimensionsPixels(fontPixelHeight, m_lines[lineIndex].m_text).x;
		maxLengthPixels = Max(maxLengthPixels, lengthPixels);
	}

	float maxLength = 0.f;
	if (maxLengthPixels > 0)
	{
		maxLength = m_canvas->ToCanvasWidth((uint32)maxLengthPixels);
	}

	return maxLength;
}


//-------------------------------------------------------------------------------------------------
Vector2 UIText::GetTextCanvasDimensions() const
{
	return GetTextCanvasDimensions(0U);
}


//-------------------------------------------------------------------------------------------------
Vector2 UIText::GetTextCanvasDimensions(uint32 lineIndex) const
{
	if (m_lines.size() == 0)
	{
		return Vector2::ZERO;
	}

	ASSERT_RETURN(lineIndex < (uint32)m_lines.size(), Vector2::ZERO, "Index out of range!");

	return GetTextCanvasDimensions(m_lines[lineIndex].m_text);
}


//-------------------------------------------------------------------------------------------------
Vector2 UIText::GetTextCanvasDimensions(const std::string& text) const
{
	uint32 fontPixelHeight = m_canvas->ToPixelHeight(m_fontHeight);
	IntVector2 pixelDimensions = m_font->GetTextDimensionsPixels(fontPixelHeight, text);

	return Vector2(m_canvas->ToCanvasWidth(pixelDimensions.x), m_canvas->ToCanvasHeight(pixelDimensions.y));
}


//-------------------------------------------------------------------------------------------------
AABB2 UIText::GetCharacterLocalBounds(uint32 lineNumber, uint32 charIndex)
{
	ASSERT_RETURN(m_characterLocalBounds.size() > 0, AABB2::ZERO_TO_ONE, "No lines in UIText element!");
	ASSERT_RETURN(lineNumber < (uint32)m_characterLocalBounds.size(), AABB2::ZERO_TO_ONE, "Bad line index!");
	
	std::vector<AABB2>& lineBounds = m_characterLocalBounds[lineNumber];
	ASSERT_RETURN(lineBounds.size() > 0, AABB2::ZERO_TO_ONE, "Empty line!");
	ASSERT_RETURN(charIndex < (uint32)lineBounds.size(), AABB2::ZERO_TO_ONE, "Bad character index!");

	return lineBounds[charIndex];
}


//-------------------------------------------------------------------------------------------------
void UIText::InitializeFromXML(const XMLElem& element)
{
	UIElement::InitializeFromXML(element);
	
	// Font size
	m_fontHeight = XML::ParseAttribute(element, "font_size", 10.f);

	// Font name
	std::string fontPath = XML::ParseAttribute(element, "font", "Data/Font/default.ttf");
	m_font = g_fontLoader->LoadFont(fontPath.c_str(), 0);

	// Text
	std::string rawText = XML::ParseAttribute(element, "text", "SAMPLE TEXT");
	std::vector<std::string> rawLines;
	BreakStringIntoLines(rawText, rawLines);

	// Text Color
	Rgba color = XML::ParseAttribute(element, "text_color", Rgba::WHITE);

	// Assemble the text
	for (size_t lineIndex = 0; lineIndex < rawLines.size(); ++lineIndex)
	{
		m_lines.push_back(ColoredText(rawLines[lineIndex], color));
	}

	// Alignments
	GetTextAlignmentFromXML(element, m_horizontalAlign, m_verticalAlign);

	// Draw mode
	std::string drawModeText = XML::ParseAttribute(element, "draw_mode", "default");
	m_textDrawMode = StringToTextDrawMode(drawModeText);

	m_isDirty = true;
}


//-------------------------------------------------------------------------------------------------
uint32 UIText::CalculatePixelHeightForBounds(const OBB2& finalBounds)
{
	float canvasHeight = finalBounds.m_alignedBounds.GetHeight();
	return m_canvas->ToPixelHeight(canvasHeight);
}


//-------------------------------------------------------------------------------------------------
void UIText::UpdateMeshAndMaterial(const OBB2& finalBounds)
{
	if (m_isDirty || !AreMostlyEqual(finalBounds.m_alignedBounds.GetHeight(), m_boundsHeightLastDraw))
	{	
		ASSERT_OR_DIE(m_font != nullptr, "Null Font!");
		ASSERT_OR_DIE(m_mesh != nullptr, "Null Mesh!");
		ASSERT_OR_DIE(m_material != nullptr, "Null Material!");
		ASSERT_OR_DIE(m_fontHeight > 0.f, "Font height is zero when trying to render!");

		uint32 fontPixelHeight = m_canvas->ToPixelHeight(m_fontHeight * m_transform.GetScale().y);

		MeshBuilder mb;
		mb.BeginBuilding(true);

		// Send the bounds as if they're at 0,0
		// The model matrix will handle the positioning
		// Font pixel height may be updated/adjusted based on draw modes
		m_characterLocalBounds.clear();
		fontPixelHeight = mb.PushText(m_lines, fontPixelHeight, m_font, AABB2(Vector2::ZERO, finalBounds.m_alignedBounds.GetDimensions()), m_canvas->GetCanvasUnitsPerPixel(), m_horizontalAlign, m_verticalAlign, m_textDrawMode, &m_characterLocalBounds);

		mb.FinishBuilding();
		mb.UpdateMesh<Vertex3D_PCU>(*m_mesh);
		mb.Clear();

		// The glyph spritesheet is generated during PushText() 
		// so it's important to update the material with the texture afterwards
		FontAtlas* atlas = m_font->CreateOrGetAtlasForPixelHeight(fontPixelHeight);
		Texture2D* texture = atlas->GetTexture();
		ShaderResourceView* resourceView = texture->CreateOrGetShaderResourceView();
		m_material->SetAlbedoTextureView(resourceView);

		m_boundsHeightLastDraw = finalBounds.m_alignedBounds.GetHeight();
		m_isDirty = false;
	}
}
