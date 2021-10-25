///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: Oct 24th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/Render/Buffer/PropertyBlockDescription.h"
#include "Engine/Render/Buffer/PropertyDescription.h"
#include "Engine/Render/DX11Common.h"
#include "Engine/Render/Shader/ShaderDescription.h"

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
// Destructor
ShaderDescription::~ShaderDescription()
{
	DX_SAFE_RELEASE(m_dxReflector);
	SafeDeleteVector(m_propertyBlocks);
}


//-------------------------------------------------------------------------------------------------
// Initializes the description from both compiled vertex and fragment sources
void ShaderDescription::Initialize(ID3DBlob* dxVertexSource, ID3DBlob* dxFragmentSource)
{
	for (int i = 0; i < 2; ++i)
	{
		ID3DBlob* dxSource = (i == 0 ? dxVertexSource : dxFragmentSource);

		// Create the reflector to get the information
		HRESULT result = D3DReflect(dxSource->GetBufferPointer(), dxSource->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&m_dxReflector);
		ASSERT_OR_DIE(SUCCEEDED(result), "Failed to set up reflection!");

		// Get the shader description for information on this stage as a whole
		D3D11_SHADER_DESC dxShaderDesc;
		result = m_dxReflector->GetDesc(&dxShaderDesc);
		ASSERT_OR_DIE(SUCCEEDED(result), "Couldn't get shader description!");

		for (int bufferIndex = 0; bufferIndex < (int)dxShaderDesc.ConstantBuffers; ++bufferIndex)
		{
			// For each constant buffer...
			ID3D11ShaderReflectionConstantBuffer* dxBuffer = m_dxReflector->GetConstantBufferByIndex(bufferIndex);

			if (dxBuffer != nullptr)
			{
				// Get the buffer description, as well as it's binding description
				D3D11_SHADER_BUFFER_DESC dxBufferDesc;
				result = dxBuffer->GetDesc(&dxBufferDesc);
				ASSERT_OR_DIE(SUCCEEDED(result), "Couldn't get buffer description!");

				D3D11_SHADER_INPUT_BIND_DESC dxBindDesc;
				m_dxReflector->GetResourceBindingDescByName(dxBufferDesc.Name, &dxBindDesc);
				ASSERT_OR_DIE(SUCCEEDED(result), "Couldn't get buffer binding description!");

				// Check to see if we've already made a description for this buffer
				// If the vertex shader uses this constant buffer, we already made a block description for it, so don't duplicate in the fragment
				// Since I put vertex and fragment functions in the same file, presumably if they both use the same constant buffer they're identical
				bool alreadyExists = GetBlockDescription(dxBindDesc.BindPoint) != nullptr;
				if (!alreadyExists)
				{
					// Doesn't exist, so make one
					PropertyBlockDescription* blockDesc = new PropertyBlockDescription(SID(dxBufferDesc.Name), dxBindDesc.BindPoint, (int)dxBufferDesc.Size);

					// For each member of the buffer
					for (int varIndex = 0; varIndex < (int)dxBufferDesc.Variables; ++varIndex)
					{
						ID3D11ShaderReflectionVariable* dxVar = dxBuffer->GetVariableByIndex(varIndex);

						// Get the member description
						D3D11_SHADER_VARIABLE_DESC dxVarDesc;
						result = dxVar->GetDesc(&dxVarDesc);
						ASSERT_OR_DIE(SUCCEEDED(result), "Couldn't get variable description!");

						// Create and add a description for it
						PropertyDescription* propDesc = new PropertyDescription(SID(dxVarDesc.Name), (int)dxVarDesc.StartOffset, (int)dxVarDesc.Size);
						blockDesc->AddPropertyDescription(propDesc);
					}

					m_propertyBlocks.push_back(blockDesc);
				}
			}
		}
	}
}


//-------------------------------------------------------------------------------------------------
// Returns the buffer description that binds to the given slot
const PropertyBlockDescription* ShaderDescription::GetBlockDescription(int bindSlot) const
{
	for (PropertyBlockDescription* desc : m_propertyBlocks)
	{
		if (desc->GetBindSlot() == bindSlot)
		{
			return desc;
		}
	}

	return nullptr;
}


//-------------------------------------------------------------------------------------------------
// Returns the buffer description with the given name, if it exists
const PropertyBlockDescription* ShaderDescription::GetBlockDescription(const StringID& bufferName) const
{
	for (PropertyBlockDescription* desc : m_propertyBlocks)
	{
		if (desc->GetName() == bufferName)
		{
			return desc;
		}
	}

	return nullptr;
}


//-------------------------------------------------------------------------------------------------
// Returns the description for the property with the given name, if it exists
const PropertyDescription* ShaderDescription::GetPropertyDescription(const StringID& variableName) const
{
	int numBlocks = (int)m_propertyBlocks.size();

	// Iterate across all blocks
	for (int blockIndex = 0; blockIndex < numBlocks; ++blockIndex)
	{
		PropertyBlockDescription* currBuffer = m_propertyBlocks[blockIndex];
		int numProperties = currBuffer->GetPropertyCount();

		// Iterate across all properties in the current block
		for (int propertyIndex = 0; propertyIndex < numProperties; ++propertyIndex)
		{
			const PropertyDescription* currVariable = currBuffer->GetPropertyDescription(propertyIndex);

			if (currVariable->GetName() == variableName)
			{
				return currVariable;
			}
		}
	}

	// Not found, so return null
	return nullptr;
}
