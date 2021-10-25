///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 15th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/DevConsole.h"
#include "Engine/Core/EngineCommon.h"
#include "Engine/IO/File.h"
#include "Engine/Render/Buffer/PropertyDescription.h"
#include "Engine/Render/Buffer/PropertyBlockDescription.h"
#include "Engine/Render/Material/Material.h"
#include "Engine/Render/Material/MaterialPropertyBlock.h"
#include "Engine/Render/RenderContext.h"
#include "Engine/Render/Shader/Shader.h"
#include "Engine/Render/Shader/ShaderDescription.h"
#include "Engine/Render/Texture/Texture2D.h"
#include "Engine/Render/Texture/TextureCube.h"
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
		ConsoleLogErrorf("File \"%s\" expected extension \?%s\"", filepath, "material");
	}

	XMLDoc doc;
	XMLErr error = doc.LoadFile(filepath);

	if (error != tinyxml2::XML_SUCCESS)
	{
		ConsoleLogErrorf("Couldn't load material file %s", filepath);
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
		// Check for texture cube first
		const XMLElem* albedoCubeElem = textureElem->FirstChildElement("albedo_cube");

		if (albedoCubeElem != nullptr)
		{
			std::string cubeTextureName = XML::ParseAttribute(*albedoCubeElem, "name", "NOT_SPECIFIED_IN_XML");
			TextureCube* cubeTexture = g_resourceSystem->CreateOrGetTextureCube(cubeTextureName.c_str());
			SetAlbedoTextureView(cubeTexture->CreateOrGetShaderResourceView());
		}
		else
		{
			const XMLElem* albedoElem = textureElem->FirstChildElement("albedo");
			std::string albedoName = (albedoElem != nullptr ? XML::ParseAttribute(*albedoElem, "name", "white") : "white");
			Texture2D* albedo = g_resourceSystem->CreateOrGetTexture2D(albedoName.c_str(), TEXTURE_USAGE_SHADER_RESOURCE_BIT, GPU_MEMORY_USAGE_STATIC);
			SetAlbedoTextureView(albedo->CreateOrGetShaderResourceView());
		}

		// Normal map
		const XMLElem* normalElem = textureElem->FirstChildElement("normal");
		std::string normalName = (normalElem != nullptr ? XML::ParseAttribute(*normalElem, "name", "flat") : "flat");
		Texture2D* normal = g_resourceSystem->CreateOrGetTexture2D(normalName.c_str(), TEXTURE_USAGE_SHADER_RESOURCE_BIT, GPU_MEMORY_USAGE_STATIC);
		SetNormalTextureView(normal->CreateOrGetShaderResourceView());
	}

	return true;
}


//-------------------------------------------------------------------------------------------------
void Material::Clear()
{
	SafeDeleteVector(m_propertyBlocks);

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


//-------------------------------------------------------------------------------------------------
// Sets the normal texture to the 2nd SRV slot
void Material::SetNormalTextureView(ShaderResourceView* normalView)
{
	m_shaderResourceViews[SRV_SLOT_NORMAL] = normalView;
}


//-------------------------------------------------------------------------------------------------
// Returns true if the shader uses lights as part of its execution
bool Material::UsesLights() const
{
	if (m_shader == nullptr)
	{
		return false;
	}

	return m_shader->UsesLights();
}


//-------------------------------------------------------------------------------------------------
// Returns the block with the given name, nullptr if it's not created yet
MaterialPropertyBlock* Material::GetPropertyBlock(const StringID& blockName) const
{
	for (MaterialPropertyBlock* block : m_propertyBlocks)
	{
		if (block->GetDescription()->GetName() == blockName)
		{
			return block;
		}
	}

	return nullptr;
}


//-------------------------------------------------------------------------------------------------
// Returns the property block that binds to the given bind slot, nullptr if it's not created yet
MaterialPropertyBlock* Material::GetPropertyBlock(int bindSlot) const
{
	for (MaterialPropertyBlock* block : m_propertyBlocks)
	{
		if (block->GetDescription()->GetBindSlot() == bindSlot)
		{
			return block;
		}
	}

	return nullptr;
}


//-------------------------------------------------------------------------------------------------
// Sets the property with the specified name to hold the given data
// Returns true if the property was set, false if not (doesn't exist in description)
bool Material::SetProperty(const StringID& propertyName, const void* data, int byteSize)
{
	const ShaderDescription* shaderDesc = m_shader->GetDescription();
	int numBlocksOnShader = (int)shaderDesc->GetBlockCount();

	for (int blockIndex = 0; blockIndex < numBlocksOnShader; ++blockIndex)
	{
		const PropertyBlockDescription* blockDescription = shaderDesc->GetBlockDescription(blockIndex);

		// If the constant buffer slot is an engine reserved one, continue without checking properties
		if (blockDescription->GetBindSlot() < ENGINE_RESERVED_CONSTANT_BUFFER_COUNT)
			continue;

		const PropertyDescription* propertyDescription = blockDescription->GetPropertyDescription(propertyName);

		if (propertyDescription != nullptr)
		{
			// Found the block that contains our property!
			// Now return (or create and return) a material property block that will hold it
			StringID blockName = blockDescription->GetName();
			MaterialPropertyBlock* matBlock = GetPropertyBlock(blockName);

			if (matBlock == nullptr)
			{
				// A material block doesn't exist for this description yet, so make one
				matBlock = CreatePropertyBlock(blockDescription);
			}

			// Size check for sanity
			int actualSize = propertyDescription->GetByteSize();
			ASSERT_OR_DIE(actualSize == byteSize, Stringf("Error: Material::SetProperty() had size mismatch - for property \"%s\", the passed size was %i, where description size has size %i", propertyName, byteSize, actualSize).c_str());

			// Offset into the block
			int offset = propertyDescription->GetByteOffset();

			// Set the data and return
			matBlock->SetCPUData(data, byteSize, offset);
			return true;
		}
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
// Creates a property block for the given description on this material
MaterialPropertyBlock* Material::CreatePropertyBlock(const PropertyBlockDescription* blockDescription)
{
	// If the uniform block is an engine reserved one, panic
	int bindSlot = blockDescription->GetBindSlot();
	ASSERT_RETURN(bindSlot > ENGINE_RESERVED_CONSTANT_BUFFER_COUNT, nullptr, "Material trying to use a reserved constant buffer slot!");

	// Ensure we don't duplicate bindings or names!
	// Delete any blocks that will have the same binding or name as this block
	int numBlocks = (int)m_propertyBlocks.size();
	StringID blockName = blockDescription->GetName();

	for (int blockIndex = 0; blockIndex < numBlocks; ++blockIndex)
	{
		MaterialPropertyBlock* existingBlock = m_propertyBlocks[blockIndex];
		int existingBindSlot = existingBlock->GetDescription()->GetBindSlot();
		StringID existingBlockName = blockDescription->GetName();

		ASSERT_RETURN(existingBindSlot != bindSlot && existingBlockName != blockName, nullptr, "Material attempting to recreate a property block!");
	}

	MaterialPropertyBlock* block = new MaterialPropertyBlock(blockDescription);
	m_propertyBlocks.push_back(block);

	return block;
}
