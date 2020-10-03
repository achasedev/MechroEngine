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
		uint32 pixelHeight = CalculatePixelHeightForBounds(finalBounds);
		UpdateMesh(pixelHeight, finalBounds);

		Renderable rend;
		rend.SetRenderableMatrix(CalculateModelMatrix(finalBounds));
		m_material->SetAlbedoTextureView(m_font->GetFontAtlasForPixelHeight(m_glyphPixelHeight)->GetTexture()->CreateOrGetShaderResourceView());
		rend.SetDrawMaterial(0, m_material);
		rend.SetDrawMesh(0, m_mesh);

		g_renderContext->DrawRenderable(rend);

		static bool save = false;
		if (!save)
		{
			g_renderContext->SaveTextureToImage(m_font->GetFontAtlasForPixelHeight(m_glyphPixelHeight)->GetTexture(), "Data/Fonts/test.png");
			save = true;
		}
	}

	UIElement::Render();
}


//-------------------------------------------------------------------------------------------------
void UIText::SetText(const std::string& text, const Rgba& color /*= Rgba::WHITE*/)
{
	m_text = text;
	m_textColor = color;
	m_textIsDirty = true;
}


//-------------------------------------------------------------------------------------------------
void UIText::SetFont(Font* font, Shader* shader)
{
	m_font = font;
	m_material->SetShader(shader);
}


//-------------------------------------------------------------------------------------------------
uint32 UIText::CalculatePixelHeightForBounds(const OBB2& finalBounds)
{
	float canvasHeight = finalBounds.alignedBounds.GetHeight();
	return m_canvas->ToPixelHeight(canvasHeight);
}


//-------------------------------------------------------------------------------------------------
void UIText::UpdateMesh(uint32 currCalculatedPixelHeight, const OBB2& finalBounds)
{
	if (m_textIsDirty || m_glyphPixelHeight != currCalculatedPixelHeight)
	{
		MeshBuilder mb;
		mb.BeginBuilding(true);

		// Send the bounds as if they're at 0,0
		// The model matrix will handle the positioning
		mb.PushText(m_text.c_str(), currCalculatedPixelHeight, m_font, AABB2(Vector2::ZERO, finalBounds.alignedBounds.GetDimensions()), m_canvas->GetCanvasUnitsPerPixel(), m_textColor);

		mb.FinishBuilding();
		mb.UpdateMesh<Vertex3D_PCU>(*m_mesh);
		mb.Clear();

		m_glyphPixelHeight = currCalculatedPixelHeight;
		m_textIsDirty = false;
	}
}
