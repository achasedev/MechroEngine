///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 21st, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
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
int UIText::s_type = 0;

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
static bool IsValidHorizontalAlignment(const std::string& text)
{
	if ((text == "left") || (text == "center") || (text == "right")) 
	{ 
		return true; 
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
static bool IsValidVerticalAlignment(const std::string& text)
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
UIText::UIText(Canvas* canvas)
	: UIElement(canvas)
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
	if (m_text.size() > 0)
	{
		// Check if the text or the scale changed which would require a rebuild
		OBB2 finalBounds = CalculateFinalBounds();
		UpdateMeshAndMaterial(finalBounds);

		Renderable rend;
		rend.SetRenderableMatrix(CalculateModelMatrix(finalBounds));
		rend.SetDrawMaterial(0, m_material);
		rend.SetDrawMesh(0, m_mesh);

		g_renderContext->DrawRenderable(rend);
	}

	UIElement::Render();
}


//-------------------------------------------------------------------------------------------------
void UIText::SetText(const std::string& text, const Rgba& color /*= Rgba::WHITE*/)
{
	m_text = text;
	m_textColor = color;
	m_isDirty = true;
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
void UIText::InitializeFromXML(const XMLElem& element)
{
	UIElement::InitializeFromXML(element);
	
	// Font size
	m_fontHeight = XML::ParseAttribute(element, "font_size", 10.f);

	// Font name
	std::string fontPath = XML::ParseAttribute(element, "font", "Data/Font/default.ttf");
	m_font = g_fontLoader->LoadFont(fontPath.c_str(), 0);

	// Text
	m_text = XML::ParseAttribute(element, "text", "SAMPLE TEXT");

	// Text Color
	m_textColor = XML::ParseAttribute(element, "text_color", Rgba::WHITE);

	// Alignments
	bool xAlignSpecified			= XML::DoesAttributeExist(element, "x_align");
	bool yAlignSpecified			= XML::DoesAttributeExist(element, "y_align");
	bool combinedAlignSpecified		= XML::DoesAttributeExist(element, "align");
	bool separateAlignsSpecified	= xAlignSpecified && yAlignSpecified;

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

			bool firstIsHorizontal	= IsValidHorizontalAlignment(first);
			bool firstIsVertical	= IsValidVerticalAlignment(first);
			bool secondIsHorizontal = IsValidHorizontalAlignment(second);
			bool secondIsVertical	= IsValidVerticalAlignment(second);

			GUARANTEE_OR_DIE((firstIsHorizontal && secondIsVertical) || (secondIsHorizontal && firstIsVertical), "Element %s: Invalid align \"%s\" specified!", element.Name(), alignText.c_str());

			if (firstIsHorizontal)
			{
				m_horizontalAlign = StringToHorizontalAlignment(first);
				m_verticalAlign = StringToVerticalAlignment(second);
			}
			else
			{
				m_horizontalAlign = StringToHorizontalAlignment(second);
				m_verticalAlign = StringToVerticalAlignment(first);
			}
		}
		else
		{
			// Only one alignment specified, so figure out which was specified and default the other
			if (IsValidHorizontalAlignment(alignText))
			{
				std::string xAlignText = XML::ParseAttribute(element, "x_align", "left");
				m_horizontalAlign = StringToHorizontalAlignment(xAlignText);
				m_verticalAlign = ALIGNMENT_BOTTOM;
			}
			else if (IsValidVerticalAlignment(alignText))
			{
				m_horizontalAlign = ALIGNMENT_LEFT;
				std::string yAlignText = XML::ParseAttribute(element, "y_align", "bottom");
				m_verticalAlign = StringToVerticalAlignment(yAlignText);
			}
			else
			{
				ERROR_RECOVERABLE("Invalid align \"%s\" specified in element %s", alignText.c_str(), element.Name());
				m_horizontalAlign = ALIGNMENT_LEFT;
				m_verticalAlign = ALIGNMENT_BOTTOM;
			}
		}
	}
	else
	{
		// Individuals were specified instead
		std::string xAlignText = XML::ParseAttribute(element, "x_align", "left");
		std::string yAlignText = XML::ParseAttribute(element, "y_align", "bottom");
		m_horizontalAlign = StringToHorizontalAlignment(xAlignText);
		m_verticalAlign = StringToVerticalAlignment(yAlignText);
	}

	m_isDirty = true;
}


//-------------------------------------------------------------------------------------------------
uint32 UIText::CalculatePixelHeightForBounds(const OBB2& finalBounds)
{
	float canvasHeight = finalBounds.alignedBounds.GetHeight();
	return m_canvas->ToPixelHeight(canvasHeight);
}


//-------------------------------------------------------------------------------------------------
void UIText::UpdateMeshAndMaterial(const OBB2& finalBounds)
{
	if (m_isDirty)
	{
		uint32 fontPixelHeight = m_canvas->ToPixelHeight(m_fontHeight);

		MeshBuilder mb;
		mb.BeginBuilding(true);

		// Send the bounds as if they're at 0,0
		// The model matrix will handle the positioning
		mb.PushText(m_text.c_str(), fontPixelHeight, m_font, AABB2(Vector2::ZERO, finalBounds.alignedBounds.GetDimensions()), m_canvas->GetCanvasUnitsPerPixel(), m_textColor, m_horizontalAlign, m_verticalAlign);

		mb.FinishBuilding();
		mb.UpdateMesh<Vertex3D_PCU>(*m_mesh);
		mb.Clear();

		// The glyph spritesheet is generated during PushText() 
		// so it's important to update the material with the texture afterwards
		FontAtlas* atlas = m_font->CreateOrGetAtlasForPixelHeight(fontPixelHeight);
		Texture2D* texture = atlas->GetTexture();
		ShaderResourceView* resourceView = texture->CreateOrGetShaderResourceView();
		m_material->SetAlbedoTextureView(resourceView);

		m_isDirty = false;
	}
}
