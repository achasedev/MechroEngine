///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 15th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Utility/StringID.h"
#include "Engine/Render/RenderContext.h"
#include "Engine/Render/Shader/Shader.h"
#include "Engine/Render/Shader/ShaderDescription.h"
#include "Engine/Render/View/ShaderResourceView.h"
#include "Engine/Resource/Resource.h"
#include <string>
#include <vector>

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class PropertyBlockDescription;
class MaterialPropertyBlock;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class Material : public Resource
{
public:
	//-----Public Methods-----

	Material();
	~Material();

	bool									Load(const char* filepath);
	void									Clear();

	void									SetShader(Shader* shader);
	void									SetShaderResourceView(uint32 slot, ShaderResourceView* textureView);
	void									SetAlbedoTextureView(ShaderResourceView* albedoView);
	void									SetNormalTextureView(ShaderResourceView* albedoView);

	Shader*									GetShader() const							{ return m_shader; }
	ShaderResourceView*						GetShaderResourceView(uint32 slot) const	{ return m_shaderResourceViews[slot]; }
	ShaderResourceView*						GetAlbedo() const							{ return m_shaderResourceViews[SRV_SLOT_ALBEDO]; }
	bool									UsesLights() const;

	int										GetPropertyBlockCount() const { return (int)m_propertyBlocks.size(); }
	MaterialPropertyBlock*					GetPropertyBlockAtIndex(int index) const;
	MaterialPropertyBlock*					GetPropertyBlockAtBindSlot(int bindSlot) const;
	MaterialPropertyBlock*					GetPropertyBlockByName(const StringID& blockName) const;

	bool SetProperty(const StringID& propertyName, const void* data, int byteSize);
	bool SetPropertyFromText(const StringID& propertyName, const std::string& propValueText);

	template <typename T>
	bool SetProperty(const StringID& propertyName, const T& value);

	template <typename T>
	bool SetPropertyBlock(const StringID& blockName, const T& blockData);


protected:
	//-----Protected Methods-----

	MaterialPropertyBlock* CreateOrGetPropertyBlock(const PropertyBlockDescription* blockDescription);
	MaterialPropertyBlock* CreatePropertyBlock(const PropertyBlockDescription* blockDescription);

private:
	//-----Private Data-----

	Shader*								m_shader = nullptr;
	ShaderResourceView*					m_shaderResourceViews[MAX_SRV_SLOTS];

	std::vector<MaterialPropertyBlock*> m_propertyBlocks;

};


//-------------------------------------------------------------------------------------------------
// Sets the property to the given value
template <typename T>
bool Material::SetProperty(const StringID& propertyName, const T& value)
{
	return SetProperty(propertyName, &value, sizeof(T));
}


//-------------------------------------------------------------------------------------------------
// Sets the entire property block to the given data
template <typename T>
bool Material::SetPropertyBlock(const StringID& blockName, const T& blockData)
{
	MaterialPropertyBlock* block = GetPropertyBlockByName(blockName);

	if (block != nullptr)
	{
		block->SetCPUData(blockData);
		return true;
	}

	// No block exists, so see if we can create it
	const ShaderDescription*		shaderDescription = m_shader->GetDescription();
	const PropertyBlockDescription* blockDescription = shaderDescription->GetBlockDescriptionByName(blockName);
	ASSERT_RETURN(blockDescription != nullptr || blockDescription->GetBindSlot() > ENGINE_RESERVED_CONSTANT_BUFFER_COUNT, false, "Couldn't set property block!");

	// Create a block and set it
	block = CreatePropertyBlock(blockDescription);
	block->SetCPUData(blockData);

	return true;
}

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
