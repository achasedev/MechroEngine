///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: October 18th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/IO/Image.h"
#include "Engine/Render/Core/RenderContext.h"
#include "Engine/Render/Material.h"
#include "Engine/Render/Mesh/MeshBuilder.h"
#include "Engine/Render/Shader.h"
#include "Engine/Render/Texture/Texture2D.h"
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
UIImage::UIImage(Canvas* canvas, const StringId& id)
	: UIElement(canvas, id)
{
	UpdateMesh();

	// TODO: Delete this
	m_material = new Material();
	Shader* shader = new Shader();
	shader->CreateFromFile("Data/Shader/test.shader");
	shader->SetBlend(BLEND_PRESET_ALPHA);
	SetShader(shader);
}


//-------------------------------------------------------------------------------------------------
UIImage::~UIImage()
{
	SAFE_DELETE(m_mesh);
	SAFE_DELETE(m_texture);
	SAFE_DELETE(m_material);
	SAFE_DELETE(m_image);
}


//-------------------------------------------------------------------------------------------------
void UIImage::LoadImage(const std::string& filepath)
{
	Image* image = new Image();

	if (image->LoadFromFile(filepath.c_str(), true))
	{
		SetImage(image);
	}
}


//-------------------------------------------------------------------------------------------------
void UIImage::InitializeFromXML(const XMLElem& element)
{
	UIElement::InitializeFromXML(element);

	// Check if the image is the name of a color first
	std::string imageText = XML::ParseAttribute(element, "image", "");

	if (imageText.size() > 0)
	{
		LoadImage(imageText);
	}
	else
	{
		SetImage(new Image(IntVector2::ONES, Rgba::WHITE));
	}

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
	if (ShouldRenderSelf() && m_texture != nullptr)
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
void UIImage::SetImage(Image* image)
{
	SAFE_DELETE(m_image);

	m_image = image;

	// Make a texture too
	if (m_texture == nullptr)
	{
		m_texture = new Texture2D();
		m_texture->CreateFromImage(*m_image, TEXTURE_USAGE_SHADER_RESOURCE_BIT, GPU_MEMORY_USAGE_GPU);
	}
	else
	{
		m_texture->UpdateFromImage(*m_image);
	}

	m_material->SetAlbedoTextureView(m_texture->CreateOrGetShaderResourceView());
}


//-------------------------------------------------------------------------------------------------
void UIImage::SetShader(Shader* shader)
{
	m_material->SetShader(shader);
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


