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

//-------------------------------------------------------------------------------------------------
// Converts the given dx enums to my internal data type enum
static PropertyDataType ConvertDxTypeToPropertyDataType(const D3D11_SHADER_TYPE_DESC& dxTypeDesc)
{
	
	switch (dxTypeDesc.Class)
	{
	case D3D_SVC_SCALAR:
	{
		if (dxTypeDesc.Type == D3D_SVT_INT)
		{
			return PROPERTY_TYPE_INT;
		}
		else if (dxTypeDesc.Type == D3D_SVT_FLOAT)
		{
			return PROPERTY_TYPE_FLOAT;
		}
	}
		break;
	case D3D_SVC_VECTOR:
		if (dxTypeDesc.Type == D3D_SVT_INT)
		{
			if		(dxTypeDesc.Columns == 2) { return PROPERTY_TYPE_INTVECTOR2; }
			else if (dxTypeDesc.Columns == 3) { return PROPERTY_TYPE_INTVECTOR3; }
		}
		else if (dxTypeDesc.Type == D3D_SVT_FLOAT)
		{
			if		(dxTypeDesc.Columns == 2) { return PROPERTY_TYPE_VECTOR2; }
			else if (dxTypeDesc.Columns == 3) { return PROPERTY_TYPE_VECTOR3; }
			else if (dxTypeDesc.Columns == 4) { return PROPERTY_TYPE_VECTOR4; }
		}
		break;
	case D3D_SVC_MATRIX_COLUMNS:
	{
		if (dxTypeDesc.Rows == 4 && dxTypeDesc.Columns == 4)
		{
			return PROPERTY_TYPE_MATRIX4;
		}
		if (dxTypeDesc.Rows == 3 && dxTypeDesc.Columns == 3)
		{
			return PROPERTY_TYPE_MATRIX3;
		}
	}
		break;
	case D3D_SVC_STRUCT:
	{
		return PROPERTY_TYPE_STRUCT;
	}
		break;
	default:
		break;
	}

	ERROR_AND_DIE("Unsupported PropertyDataType!");
}


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
	ASSERT_RECOVERABLE(dxVertexSource != nullptr, "Vertex shader source is null!");

	for (int i = 0; i < 2; ++i)
	{
		ID3DBlob* dxSource = (i == 0 ? dxVertexSource : dxFragmentSource);

		// The fragment shader is optional
		if (dxSource == nullptr)
			continue;

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
				bool alreadyExists = GetBlockDescriptionAtBindSlot(dxBindDesc.BindPoint) != nullptr;
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

						ID3D11ShaderReflectionType* dxType = dxVar->GetType();
						D3D11_SHADER_TYPE_DESC dxTypeDesc;
						result = dxType->GetDesc(&dxTypeDesc);
						ASSERT_OR_DIE(SUCCEEDED(result), "Couldn't get variable type description!");

						PropertyDataType dataType = ConvertDxTypeToPropertyDataType(dxTypeDesc);

						// Create and add a description for it
						PropertyDescription* propDesc = new PropertyDescription(SID(dxVarDesc.Name), blockDesc, (int)dxVarDesc.StartOffset, (int)dxVarDesc.Size, dataType);
						blockDesc->AddPropertyDescription(propDesc);
					}

					m_propertyBlocks.push_back(blockDesc);
				}
			}
		}
	}
}


//-------------------------------------------------------------------------------------------------
// Returns the block at the given index
const PropertyBlockDescription* ShaderDescription::GetBlockDescriptionAtIndex(int index) const
{
	return m_propertyBlocks[index];
}


//-------------------------------------------------------------------------------------------------
// Returns the buffer description that binds to the given slot
const PropertyBlockDescription* ShaderDescription::GetBlockDescriptionAtBindSlot(int bindSlot) const
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
const PropertyBlockDescription* ShaderDescription::GetBlockDescriptionByName(const StringID& bufferName) const
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
			const PropertyDescription* currProp = currBuffer->GetPropertyDescription(propertyIndex);

			if (currProp->GetName() == variableName)
			{
				return currProp;
			}
		}
	}

	// Not found, so return null
	return nullptr;
}
