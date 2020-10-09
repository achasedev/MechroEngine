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
	std::string xAlignText = XML::ParseAttribute(element, "x_align", "left");
	std::string yAlignText = XML::ParseAttribute(element, "y_align", "bottom");

	m_horizontalAlign = StringToHorizontalAlignment(xAlignText);
	m_verticalAlign = StringToVerticalAlignment(yAlignText);

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
