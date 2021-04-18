///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 15th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/DevConsole.h"
#include "Engine/Framework/EngineCommon.h"
#include "Engine/IO/File.h"
#include "Engine/Render/Material.h"
#include "Engine/Render/Shader.h"
#include "Engine/Render/Texture/Texture2D.h"
#include "Engine/Render/View/ShaderResourceView.h"
#include "Engine/Resource/ResourceSystem.h"
#include "Engine/Utility/XMLUtils.h"

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
Material::Material()
{
	Clear();
}


//-------------------------------------------------------------------------------------------------
bool Material::Load(const char* filepath)
{
	if (!DoesFilePathHaveExtension(filepath, "material"))
	{
		ConsoleErrorf("File \"%s\" expected extension \?%s\"", filepath, "material");
	}

	XMLDoc doc;
	XMLErr error = doc.LoadFile(filepath);

	if (error != tinyxml2::XML_SUCCESS)
	{
		ConsoleErrorf("Couldn't load material file %s", filepath);
		return false;
	}

	const XMLElem* rootElem = doc.RootElement();

	// Shader
	const XMLElem* shaderElem = rootElem->FirstChildElement("shader");
	std::string shaderName = (shaderElem != nullptr ? XML::ParseAttribute(*shaderElem, "file", "Data/Shader/invalid.shader") : "Data/Shader/invalid.shader");
	SetShader(g_resourceSystem->CreateOrGetShader(shaderName.c_str()));
	
	// Textures
	const XMLElem* textureElem = rootElem->FirstChildElement("texture");
	if (textureElem != nullptr)
	{
		const XMLElem* albedoElem = textureElem->FirstChildElement("albedo");
		std::string albedoName = (albedoElem != nullptr ? XML::ParseAttribute(*albedoElem, "name", "white") : "white");
		Texture2D* albedo = g_resourceSystem->CreateOrGetTexture2D(albedoName.c_str(), TEXTURE_USAGE_SHADER_RESOURCE_BIT, GPU_MEMORY_USAGE_STATIC);
		SetAlbedoTextureView(albedo->CreateOrGetShaderResourceView());
	}

	return true;
}


//-------------------------------------------------------------------------------------------------
void Material::Clear()
{
	m_shader = nullptr;
	
	for (int viewIndex = 0; viewIndex < MAX_SRV_SLOTS; ++viewIndex)
	{
		m_shaderResourceViews[viewIndex] = nullptr;
	}
}


//-------------------------------------------------------------------------------------------------
void Material::SetShader(Shader* shader)
{
	m_shader = shader;
}


//-------------------------------------------------------------------------------------------------
void Material::SetShaderResourceView(uint32 slot, ShaderResourceView* textureView)
{
	ASSERT_OR_DIE(slot < MAX_SRV_SLOTS, "Material texture index out of bounds!");
	m_shaderResourceViews[slot] = textureView;
}


//-------------------------------------------------------------------------------------------------
void Material::SetAlbedoTextureView(ShaderResourceView* albedoView)
{
	m_shaderResourceViews[SRV_SLOT_ALBEDO] = albedoView;
}
