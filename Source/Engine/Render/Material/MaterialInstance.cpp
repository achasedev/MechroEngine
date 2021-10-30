///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: Oct 30th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/Render/Material/MaterialInstance.h"
#include "Engine/Render/Material/MaterialPropertyBlock.h"

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
// Constructor from base material; Shallow copy shader + views to avoid instancing
// Deep copy property blocks for the instance
MaterialInstance::MaterialInstance(Material* baseMaterial)
	: m_baseMaterial(baseMaterial)
{
	ASSERT_OR_DIE(baseMaterial != nullptr, "Material instance being initialized from null material!");
	ASSERT_OR_DIE(!baseMaterial->IsInstance(), "Instance being created from an instance!");

	m_shader = baseMaterial->GetShader();

	for (uint32 i = 0; i < MAX_SRV_SLOTS; ++i)
	{
		m_shaderResourceViews[i] = baseMaterial->GetShaderResourceView(i);
	}

	CloneBasePropertyBlocks();
	m_isInstance = true;
}


//-------------------------------------------------------------------------------------------------
// Restores this material back
void MaterialInstance::ResetToBaseMaterial()
{
	SafeDeleteVector(m_propertyBlocks);
	CloneBasePropertyBlocks();
}


//-------------------------------------------------------------------------------------------------
// Deep copy the base material's property blocks to instance them
void MaterialInstance::CloneBasePropertyBlocks()
{
	int numBlocks = (int)m_baseMaterial->GetPropertyBlockCount();
	for (int i = 0; i < numBlocks; ++i)
	{
		MaterialPropertyBlock* block = m_baseMaterial->GetPropertyBlockAtIndex(i);
		MaterialPropertyBlock* clone = block->CreateClone();
		m_propertyBlocks.push_back(clone);
	}
}
