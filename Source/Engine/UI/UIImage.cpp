///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: October 18th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/IO/Image.h"
#include "Engine/Render/RenderContext.h"
#include "Engine/Render/Material.h"
#include "Engine/Render/Mesh/MeshBuilder.h"
#include "Engine/Render/Shader.h"
#include "Engine/Render/Texture/Texture2D.h"
#include "Engine/Resource/ResourceSystem.h"
#include "Engine/UI/UIImage.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------
RTTI_TYPE_DEFINE(UIImage);

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
UIImage::UIImage(Canvas* canvas, const StringID& id)
	: UIElement(canvas, id)
{
	UpdateMesh();

	m_material = new Material();
	m_material->SetShader(g_resourceSystem->CreateOrGetShader("Data/Shader/default_alpha.shader"));
}


//-------------------------------------------------------------------------------------------------
UIImage::~UIImage()
{
	SAFE_DELETE(m_mesh);
	SAFE_DELETE(m_material);
}


//-------------------------------------------------------------------------------------------------
void UIImage::InitializeFromXML(const XMLElem& element)
{
	UIElement::InitializeFromXML(element);

	// Check if the image is the name of a color first
	std::string imageText = XML::ParseAttribute(element, "image", "white");
	Image* image = g_resourceSystem->CreateOrGetImage(imageText.c_str());
	Texture2D* texture = g_resourceSystem->CreateOrGetTexture2D(image->GetResourceID().ToString(), TEXTURE_USAGE_SHADER_RESOURCE_BIT, GPU_MEMORY_USAGE_STATIC);
	SetTexture(texture);
	
	Rgba color = XML::ParseAttribute(element, "color", m_colorTint);
	SetColor(color);
}


//-------------------------------------------------------------------------------------------------
void UIImage::Update()
{
	UIElement::Update();
}


//-------------------------------------------------------------------------------------------------
void UIImage::Render()
{
	if (ShouldRenderSelf())
	{
		UpdateMesh();

		// Check if the text or the scale changed which would require a rebuild
		OBB2 finalBounds = GetCanvasBounds();

		Renderable rend;
		rend.SetRenderableMatrix(CreateModelMatrix(finalBounds));
		rend.SetDrawMaterial(0, m_material);
		rend.SetDrawMesh(0, m_mesh); // TODO: Default quad mesh?

		g_renderContext->DrawRenderable(rend);
	}

	UIElement::Render();
}


//-------------------------------------------------------------------------------------------------
void UIImage::SetColor(const Rgba& color)
{
	m_colorTint = color;
	m_meshDirty = true;

}


//-------------------------------------------------------------------------------------------------
void UIImage::SetTexture(Texture2D* texture)
{
	m_material->SetAlbedoTextureView(texture->CreateOrGetShaderResourceView());
}


//-------------------------------------------------------------------------------------------------
void UIImage::UpdateMesh()
{
	if (m_meshDirty)
	{
		MeshBuilder mb;
		mb.BeginBuilding(true);
		mb.PushQuad2D(AABB2::ZERO_TO_ONE, AABB2::ZERO_TO_ONE, m_colorTint);
		mb.FinishBuilding();

		if (m_mesh == nullptr)
		{
			m_mesh = mb.CreateMesh<Vertex3D_PCU>();
		}
		else
		{
			mb.UpdateMesh<Vertex3D_PCU>(*m_mesh);
		}

		m_meshDirty = false;
	}
}


