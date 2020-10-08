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
		m_material->SetAlbedoTextureView(m_font->CreateOrGetAtlasForPixelHeight(fontPixelHeight)->GetTexture()->CreateOrGetShaderResourceView());

		MeshBuilder mb;
		mb.BeginBuilding(true);

		// Send the bounds as if they're at 0,0
		// The model matrix will handle the positioning
		mb.PushText(m_text.c_str(), fontPixelHeight, m_font, AABB2(Vector2::ZERO, finalBounds.alignedBounds.GetDimensions()), m_canvas->GetCanvasUnitsPerPixel(), m_textColor);

		mb.FinishBuilding();
		mb.UpdateMesh<Vertex3D_PCU>(*m_mesh);
		mb.Clear();

		m_isDirty = false;
	}
}
