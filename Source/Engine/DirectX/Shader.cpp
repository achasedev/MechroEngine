///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 9th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** INCLUDES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/DirectX/DX11Common.h"
#include "Engine/DirectX/RenderContext.h"
#include "Engine/DirectX/Shader.h"
#include "Engine/DirectX/Vertex.h"
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Framework/File.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** DEFINES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                              *** TYPES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** STRUCTS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                        *** GLOBALS AND STATICS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                           *** C FUNCTIONS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
static const char* GetEntryForStage(ShaderStageType stageType)
{
	switch (stageType)
	{
	case SHADER_STAGE_VERTEX: return "VertexFunction"; break;
	case SHADER_STAGE_FRAGMENT: return "FragmentFunction"; break;
	default:
		ERROR_AND_DIE("Attempted to get entry of invalid stage type!");
		break;
	}
}


//-------------------------------------------------------------------------------------------------
static const char* GetShaderModelForStage(ShaderStageType stageType)
{
	switch (stageType)
	{
	case SHADER_STAGE_VERTEX: return "vs_5_0"; break;
	case SHADER_STAGE_FRAGMENT: return "ps_5_0"; break;
	default:
		ERROR_AND_DIE("Attempted to get model for invalid stage type!");
		break;
	}
}


//-------------------------------------------------------------------------------------------------
ID3DBlob* CompileHLSL(const char* filename, const void* sourceCode, const size_t sourceCodeSize, const char* entrypoint, const char* shaderModel)
{
	DWORD compile_flags = 0U;
#ifdef DEBUG_SHADERS
	compile_flags |= D3DCOMPILE_DEBUG;
	compile_flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
	compile_flags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;
#else
	compile_flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

	ID3DBlob* code = nullptr;
	ID3DBlob* errors = nullptr;

	HRESULT hr = ::D3DCompile(sourceCode, sourceCodeSize, filename, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entrypoint, shaderModel, compile_flags, 00, &code, &errors);

	if (errors != nullptr)
	{
		const char* errorString = (const char*)errors->GetBufferPointer();
		ERROR_RECOVERABLE("Failed to compile shader %s, Compiler gave the following output: %s/n", filename, errorString);
		DX_SAFE_RELEASE(errors);
	}
	else if (FAILED(hr))
	{
		ERROR_RECOVERABLE("Failed with HRESULT: %u\n", hr);
	}

	return code;
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** CLASSES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
ShaderStage::~ShaderStage()
{
	DX_SAFE_RELEASE(m_compiledSource);
	DX_SAFE_RELEASE(m_handle);
}


//-------------------------------------------------------------------------------------------------
bool ShaderStage::LoadFromShaderSource(const char* filename, const void* source, const size_t sourceByteSize, ShaderStageType stageType)
{
	ASSERT_OR_DIE(stageType != SHADER_STAGE_INVALID, "Attempted to make an invalid shader stage!");
	ASSERT_OR_DIE(m_handle == nullptr, "ShaderStage was already initialized!");

	RenderContext* renderContext = RenderContext::GetInstance();
	ID3D11Device* dxDevice = renderContext->GetDxDevice();

	const char* entryPoint = GetEntryForStage(stageType);
	const char* shaderModel = GetShaderModelForStage(stageType);

	ID3DBlob* byteCode = CompileHLSL(filename, source, sourceByteSize, entryPoint, shaderModel);

	if (byteCode == nullptr)
	{
		return false;
	}

	switch (stageType)
	{
	case SHADER_STAGE_VERTEX:
		dxDevice->CreateVertexShader(byteCode->GetBufferPointer(), byteCode->GetBufferSize(), nullptr, &m_vertexShader);
		m_compiledSource = byteCode; // Save off byte code for input layouts
		break;
	case SHADER_STAGE_FRAGMENT:
		dxDevice->CreatePixelShader(byteCode->GetBufferPointer(), byteCode->GetBufferSize(), nullptr, &m_fragmentShader);	
		DX_SAFE_RELEASE(byteCode); // Don't need byte code for fragment shaders
		break;
	default:
		break;
	}

	return IsValid();
}

bool Shader::CreateFromFile(const char* filename)
{
	size_t shaderSourceSize = 0;
	void* shaderSource = FileReadToNewBuffer(filename, shaderSourceSize);

	// Compile both stages
	m_vertexShader.LoadFromShaderSource(filename, shaderSource, shaderSourceSize, SHADER_STAGE_VERTEX);
	m_fragmentShader.LoadFromShaderSource(filename, shaderSource, shaderSourceSize, SHADER_STAGE_FRAGMENT);

	free(shaderSource);

	return m_vertexShader.IsValid() && m_fragmentShader.IsValid();
}


//-------------------------------------------------------------------------------------------------
// Creates a DX11 input element description using the compiled vertex shader code
bool Shader::CreateInputLayoutForVertexLayout(const VertexLayout* vertexLayout)
{
	bool createdNewLayout = false;

	// Input layout hasn't been made yet or is a different set of vertex attributes, re-create it
	if (m_shaderInputLayout.m_dxInputLayout == nullptr || m_shaderInputLayout.m_vertexLayoutUsed != vertexLayout)
	{
		uint numAttributes = vertexLayout->GetAttributeCount();
		D3D11_INPUT_ELEMENT_DESC* desc = (D3D11_INPUT_ELEMENT_DESC*)malloc(sizeof(D3D11_INPUT_ELEMENT_DESC) * numAttributes);
		memset(desc, 0, sizeof(D3D11_INPUT_ELEMENT_DESC) * numAttributes);

		for (uint attributeIndex = 0; attributeIndex < numAttributes; ++attributeIndex)
		{
			const VertexAttribute& currAttribute = vertexLayout->GetAttribute(attributeIndex);

			desc[attributeIndex].SemanticName = currAttribute.m_name.c_str();
			desc[attributeIndex].SemanticIndex = 0;
			desc[attributeIndex].Format = static_cast<DXGI_FORMAT>(GetDXFormatForAttribute(currAttribute));
			desc[attributeIndex].InputSlot = 0U;
			desc[attributeIndex].AlignedByteOffset = currAttribute.m_memberOffset;
			desc[attributeIndex].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			desc[attributeIndex].InstanceDataStepRate = 0U;
		}

		ID3DBlob* vsByteCode = m_vertexShader.GetCompiledSource();
		ASSERT_OR_DIE(vsByteCode != nullptr, "Attempted to create input layout for vertex stage that didn't compile!");

		RenderContext* context = RenderContext::GetInstance();
		ID3D11Device* dxDevice = context->GetDxDevice();

		HRESULT hr = dxDevice->CreateInputLayout(desc, numAttributes, vsByteCode->GetBufferPointer(), vsByteCode->GetBufferSize(), &m_shaderInputLayout.m_dxInputLayout);
		createdNewLayout = SUCCEEDED(hr);

		if (createdNewLayout)
		{
			m_shaderInputLayout.m_vertexLayoutUsed = vertexLayout;
		}
		else
		{
			m_shaderInputLayout.m_dxInputLayout = nullptr;
			m_shaderInputLayout.m_vertexLayoutUsed = nullptr;
		}

		SAFE_FREE_POINTER(desc);
	}

	return createdNewLayout;
}
