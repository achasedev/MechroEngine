///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 9th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/DevConsole.h"
#include "Engine/Core/EngineCommon.h"
#include "Engine/IO/File.h"
#include "Engine/Render/DX11Common.h"
#include "Engine/Render/RenderContext.h"
#include "Engine/Render/Shader/ConstantBufferDescription.h"
#include "Engine/Render/Shader/ConstantVariableDescription.h"
#include "Engine/Render/Shader/Shader.h"
#include "Engine/Render/Mesh/Vertex.h"
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
		ERROR_RECOVERABLE("Failed to compile shader %s, Compiler gave the following output: %s", filename, errorString);
		DX_SAFE_RELEASE(errors);
	}
	else if (FAILED(hr))
	{
		ERROR_RECOVERABLE("Failed with HRESULT: %u", hr);
	}

	return code;
}


//-------------------------------------------------------------------------------------------------
D3D11_BLEND ToDXBlendFactor(BlendFactor blendFactor)
{
	switch (blendFactor)
	{
	case BLEND_FACTOR_ZERO:				return D3D11_BLEND_ZERO; break;
	case BLEND_FACTOR_ONE:				return D3D11_BLEND_ONE; break;
	case BLEND_FACTOR_SRC_COLOR:		return D3D11_BLEND_SRC_COLOR; break;
	case BLEND_FACTOR_INV_SRC_COLOR:	return D3D11_BLEND_INV_SRC_COLOR; break;
	case BLEND_FACTOR_SRC_ALPHA:		return D3D11_BLEND_SRC_ALPHA; break;
	case BLEND_FACTOR_INV_SRC_ALPHA:	return D3D11_BLEND_INV_SRC_ALPHA; break;
	case BLEND_FACTOR_DEST_ALPHA:		return D3D11_BLEND_DEST_ALPHA; break;
	case BLEND_FACTOR_INV_DEST_ALPHA:	return D3D11_BLEND_INV_DEST_ALPHA; break;
	case BLEND_FACTOR_DEST_COLOR:		return D3D11_BLEND_DEST_COLOR; break;
	case BLEND_FACTOR_INV_DEST_COLOR:	return D3D11_BLEND_INV_DEST_COLOR; break;
	case BLEND_FACTOR_SRC_ALPHA_SAT:	return D3D11_BLEND_SRC_ALPHA_SAT; break;
	case BLEND_FACTOR_BLEND_FACTOR:		return D3D11_BLEND_BLEND_FACTOR; break;
	case BLEND_FACTOR_INV_BLEND_FACTOR: return D3D11_BLEND_INV_BLEND_FACTOR; break;
	case BLEND_FACTOR_SRC1_COLOR:		return D3D11_BLEND_SRC1_COLOR; break;
	case BLEND_FACTOR_INV_SRC1_COLOR:	return D3D11_BLEND_INV_SRC1_COLOR; break;
	case BLEND_FACTOR_SRC1_ALPHA:		return D3D11_BLEND_SRC1_ALPHA; break;
	case BLEND_FACTOR_INV_SRC1_ALPHA:	return D3D11_BLEND_INV_SRC1_ALPHA; break;
	default:
		ERROR_RETURN(D3D11_BLEND_ONE, "Invalid Blend Op!");
		break;
	}
}


//-------------------------------------------------------------------------------------------------
D3D11_BLEND_OP ToDXBlendOp(BlendOp blendOp)
{
	switch (blendOp)
	{
	case BLEND_OP_ADD:			return D3D11_BLEND_OP_ADD; break;
	case BLEND_OP_SUBTRACT:		return D3D11_BLEND_OP_SUBTRACT; break;
	case BLEND_OP_REV_SUBTRACT: return D3D11_BLEND_OP_REV_SUBTRACT; break;
	case BLEND_OP_MIN:			return D3D11_BLEND_OP_MIN; break;
	case BLEND_OP_MAX:			return D3D11_BLEND_OP_MAX; break;
	default:
		ERROR_RETURN(D3D11_BLEND_OP_ADD, "Invalid Blend Op!");
		break;
	}
}


//-------------------------------------------------------------------------------------------------
D3D11_FILL_MODE ToDXFillMode(FillMode fillMode)
{
	switch (fillMode)
	{
	case FILL_MODE_SOLID:		return D3D11_FILL_SOLID; break;
	case FILL_MODE_WIREFRAME:	return D3D11_FILL_WIREFRAME; break;
	default:
		ERROR_RETURN(D3D11_FILL_SOLID, "Invalid Fill Mode!");
		break;
	}
}


//-------------------------------------------------------------------------------------------------
D3D11_CULL_MODE ToDxCullMode(CullMode cullMode)
{
	switch (cullMode)
	{
	case CULL_MODE_BACK:	return D3D11_CULL_BACK; break;
	case CULL_MODE_FRONT:	return D3D11_CULL_FRONT; break;
	default:
		ERROR_RETURN(D3D11_CULL_BACK, "Invalid Cull Mode!");
		break;
	}
}

//-------------------------------------------------------------------------------------------------
D3D11_COMPARISON_FUNC ToDxDepthFunc(DepthMode depthMode)
{
	switch (depthMode)
	{
	case DEPTH_MODE_LESS_THAN:				return D3D11_COMPARISON_LESS; break;
	case DEPTH_MODE_LESS_THAN_OR_EQUAL:		return D3D11_COMPARISON_LESS_EQUAL; break;
	case DEPTH_MODE_GREATER_THAN:			return D3D11_COMPARISON_GREATER; break;
	case DEPTH_MODE_GREATER_THAN_OR_EQUAL:	return D3D11_COMPARISON_GREATER_EQUAL; break;
	case DEPTH_MODE_IGNORE_DEPTH:			return D3D11_COMPARISON_ALWAYS; break;
	default:
		ERROR_AND_DIE("Invalid depth mode!");
		break;
	}
}


//-------------------------------------------------------------------------------------------------
BlendPreset StringToBlendPreset(const std::string& blendText)
{
	if		(blendText == "opaque") { return BLEND_PRESET_OPAQUE; }
	else if (blendText == "alpha") { return BLEND_PRESET_ALPHA; }
	else if (blendText == "additive" || blendText == "add") { return BLEND_PRESET_ADDITIVE; }
	else
	{
		ConsoleLogErrorf("Invalid BlendPreset %s, defaulting to opaque", blendText.c_str());
		return BLEND_PRESET_OPAQUE;
	}
}


//-------------------------------------------------------------------------------------------------
FillMode StringToFillMode(const std::string& fillText)
{
	if		(fillText == "wire" || fillText == "wireframe") { return FILL_MODE_WIREFRAME; }
	else if (fillText == "solid") { return FILL_MODE_SOLID; }
	else
	{
		ConsoleLogErrorf("Invalid FillMode %s, defaulting to solid", fillText.c_str());
		return FILL_MODE_SOLID;
	}
}


//-------------------------------------------------------------------------------------------------
CullMode StringToCullMode(const std::string& cullText)
{
	if		(cullText == "back" || cullText == "back_face") { return CULL_MODE_BACK; }
	else if (cullText == "front" || cullText == "back_front") { return CULL_MODE_FRONT; }
	else
	{
		ConsoleLogErrorf("Invalid CullMode %s, defaulting to back", cullText.c_str());
		return CULL_MODE_BACK;
	}
}


//-------------------------------------------------------------------------------------------------
DepthMode StringToDepthMode(const std::string& depthText)
{
	if		(depthText == "less_than" || depthText == "less") { return DEPTH_MODE_LESS_THAN; }
	else if (depthText == "less_than_or_equal" || depthText == "less_than_equal") { return DEPTH_MODE_LESS_THAN_OR_EQUAL; }
	else if (depthText == "greater_than" || depthText == "greater") { return DEPTH_MODE_GREATER_THAN; }
	else if (depthText == "greater_than_or_equal" || depthText == "greater_than_equal") { return DEPTH_MODE_GREATER_THAN_OR_EQUAL; }
	else if (depthText == "no_depth" || depthText == "ignore" || depthText == "ignore_depth") { return DEPTH_MODE_IGNORE_DEPTH; }
	else
	{
		ConsoleLogErrorf("Invalid CullMode %s, defaulting to less than!", depthText.c_str());
		return DEPTH_MODE_LESS_THAN;
	}
}


//-------------------------------------------------------------------------------------------------
RenderQueue StringToSortingQueue(const std::string& queueText)
{
	if (queueText == "opaque") { return RENDER_QUEUE_OPAQUE; }
	else if (queueText == "alpha") { return RENDER_QUEUE_ALPHA; }
	else
	{
		ConsoleLogErrorf("Invalid sorting queue %s, defaulting to opaque", queueText.c_str());
		return RENDER_QUEUE_OPAQUE;
	}
}


//-------------------------------------------------------------------------------------------------
bool StringToLightsBool(const std::string& lightText)
{
	if (lightText == "no" || lightText == "false") { return false; }
	else if (lightText == "yes" || lightText == "true") { return true; }
	else
	{
		ConsoleLogErrorf("Invalid light parameter %s, defaulting to no lights used", lightText.c_str());
		return false;
	}
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
ShaderStage::~ShaderStage()
{
	Clear();
}


//-------------------------------------------------------------------------------------------------
void ShaderStage::Clear()
{
	SafeDeleteVector(m_constantBufferDescriptions);

	DX_SAFE_RELEASE(m_dxReflector);
	DX_SAFE_RELEASE(m_dxCompiledSource);
	DX_SAFE_RELEASE(m_dxHandle);
	m_stageType = SHADER_STAGE_INVALID;
}


//-------------------------------------------------------------------------------------------------
bool ShaderStage::LoadFromShaderSource(const char* filename, const void* source, const size_t sourceByteSize, ShaderStageType stageType)
{
	ASSERT_OR_DIE(stageType != SHADER_STAGE_INVALID, "Attempted to make an invalid shader stage!");
	ASSERT_OR_DIE(m_dxHandle == nullptr, "ShaderStage was already initialized!");

	ID3D11Device* dxDevice = g_renderContext->GetDxDevice();

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
		dxDevice->CreateVertexShader(byteCode->GetBufferPointer(), byteCode->GetBufferSize(), nullptr, &m_dxVertexShader);
		m_dxCompiledSource = byteCode; // Save off byte code for input layouts and reflection
		m_stageType = SHADER_STAGE_VERTEX;
		SetUpReflection();
		break;
	case SHADER_STAGE_FRAGMENT:
		dxDevice->CreatePixelShader(byteCode->GetBufferPointer(), byteCode->GetBufferSize(), nullptr, &m_dxFragmentShader);
		m_dxCompiledSource = byteCode; // Save off byte code for reflection
		m_stageType = SHADER_STAGE_FRAGMENT;
		SetUpReflection();
		break;
	default:
		break;
	}

	return IsValid();
}


//-------------------------------------------------------------------------------------------------
// Returns the description for the buffer at the given bind point, if it exists
const ConstantBufferDescription* ShaderStage::GetBufferDescription(int bindSlot) const
{
	for (ConstantBufferDescription* desc : m_constantBufferDescriptions)
	{
		if (desc->GetBindSlot() == bindSlot)
		{
			return desc;
		}
	}

	return nullptr;
}


//-------------------------------------------------------------------------------------------------
// Returns the description for the buffer with the given name, if it exists
const ConstantBufferDescription* ShaderStage::GetBufferDescription(const StringID& bufferName) const
{
	for (ConstantBufferDescription* desc : m_constantBufferDescriptions)
	{
		if (desc->GetName() == bufferName)
		{
			return desc;
		}
	}

	return nullptr;
}


//-------------------------------------------------------------------------------------------------
// Creates a shader reflection object to read uniforms on the stage
void ShaderStage::SetUpReflection()
{
	HRESULT result = D3DReflect(m_dxCompiledSource->GetBufferPointer(), m_dxCompiledSource->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&m_dxReflector);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to set up reflection!");

	D3D11_SHADER_DESC dxShaderDesc;
	result = m_dxReflector->GetDesc(&dxShaderDesc);
	ASSERT_OR_DIE(SUCCEEDED(result), "Couldn't get shader description!");

	int buffCount = 0;
	while (buffCount < (int)dxShaderDesc.ConstantBuffers)
	{
		ID3D11ShaderReflectionConstantBuffer* dxBuffer = m_dxReflector->GetConstantBufferByIndex(buffCount);
		
		if (dxBuffer != nullptr)
		{
			D3D11_SHADER_BUFFER_DESC dxBufferDesc;
			result = dxBuffer->GetDesc(&dxBufferDesc);
			ASSERT_OR_DIE(SUCCEEDED(result), "Couldn't get buffer description!");

			D3D11_SHADER_INPUT_BIND_DESC bindDesc;
			m_dxReflector->GetResourceBindingDescByName(dxBufferDesc.Name, &bindDesc);
			ASSERT_OR_DIE(SUCCEEDED(result), "Couldn't get buffer binding description!");

			ConstantBufferDescription* bufferDesc = new ConstantBufferDescription(SID(dxBufferDesc.Name), bindDesc.BindPoint, (int)dxBufferDesc.Size);

			for (int varIndex = 0; varIndex < (int)dxBufferDesc.Variables; ++varIndex)
			{
				ID3D11ShaderReflectionVariable* dxVar = dxBuffer->GetVariableByIndex(varIndex);

				D3D11_SHADER_VARIABLE_DESC dxVarDesc;
				result = dxVar->GetDesc(&dxVarDesc);
				
				ASSERT_OR_DIE(SUCCEEDED(result), "Couldn't get variable description!");

				ConstantVariableDescription* varDesc = new ConstantVariableDescription(SID(dxVarDesc.Name), (int)dxVarDesc.StartOffset, (int)dxVarDesc.Size);
				bufferDesc->AddVariableDescription(varDesc);
			}

			m_constantBufferDescriptions.push_back(bufferDesc);
		}

		buffCount++;
	}
}


//-------------------------------------------------------------------------------------------------
Shader::Shader()
{
	// Default blend state to opaque
	SetBlend(BLEND_PRESET_OPAQUE);
}


//-------------------------------------------------------------------------------------------------
Shader::~Shader()
{
	Clear();
}


//-------------------------------------------------------------------------------------------------
bool Shader::Load(const char* filepath)
{
	if (!DoesFilePathHaveExtension(filepath, "shader"))
	{
		ConsoleLogErrorf("File \"%s\" expected extension \?%s\"", filepath, "shader");
	}

	XMLDoc doc;
	XMLErr error = doc.LoadFile(filepath);

	if (error != tinyxml2::XML_SUCCESS)
	{
		ConsoleLogErrorf("Couldn't load resource file %s", filepath);
		return false;
	}

	const XMLElem* rootElem = doc.RootElement();

	if (rootElem == nullptr)
	{
		Clear();
		return false;
	}

	std::string sourceFilepath = XML::ParseAttribute(*rootElem, "source", "Data/Shader/invalid.shadersource");
	bool success = LoadAndCompileShaderSource(sourceFilepath.c_str());

	if (!success)
	{
		ConsoleLogErrorf("Couldn't load shader source file %s", sourceFilepath.c_str());
		Clear();
		return false;
	}
	
	// Blend
	std::string blendText = XML::ParseAttribute(*rootElem, "blend", "opaque");
	SetBlend(StringToBlendPreset(blendText));

	// Fill
	std::string fillText = XML::ParseAttribute(*rootElem, "fill", "solid");
	SetFillMode(StringToFillMode(fillText));

	// Cull
	std::string cullText = XML::ParseAttribute(*rootElem, "cull", "back");
	SetCullMode(StringToCullMode(cullText));

	// Depth
	std::string depthText = XML::ParseAttribute(*rootElem, "depth", "less");
	SetDepthMode(StringToDepthMode(depthText));

	// Sorting queue and layer
	std::string queueText = XML::ParseAttribute(*rootElem, "queue", "opaque");
	m_renderQueue = StringToSortingQueue(queueText);
	m_renderLayer = XML::ParseAttribute(*rootElem, "layer", 0);

	return true;
}


//-------------------------------------------------------------------------------------------------
void Shader::Clear()
{
	DX_SAFE_RELEASE(m_dxBlendState);
	DX_SAFE_RELEASE(m_dxRasterizerState);
	DX_SAFE_RELEASE(m_shaderInputLayout.m_dxInputLayout);
	DX_SAFE_RELEASE(m_shaderInputLayout.m_dxInputLayout);

	m_vertexShader.Clear();
	m_fragmentShader.Clear();
	m_shaderInputLayout.m_vertexLayoutUsed = nullptr;
	m_blendStateDirty = true;
	m_scissorEnabled = false;
	m_rasterizerStateDirty = true;
	m_blendStateDirty = true;
	m_depthStateDirty = true;
}


//-------------------------------------------------------------------------------------------------
bool Shader::LoadAndCompileShaderSource(const char* filename)
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
		DX_SAFE_RELEASE(m_shaderInputLayout.m_dxInputLayout);

		uint32 numAttributes = vertexLayout->GetAttributeCount();
		D3D11_INPUT_ELEMENT_DESC* desc = (D3D11_INPUT_ELEMENT_DESC*)malloc(sizeof(D3D11_INPUT_ELEMENT_DESC) * numAttributes);
		memset(desc, 0, sizeof(D3D11_INPUT_ELEMENT_DESC) * numAttributes);

		for (uint32 attributeIndex = 0; attributeIndex < numAttributes; ++attributeIndex)
		{
			const VertexAttribute& currAttribute = vertexLayout->GetAttribute(attributeIndex);

			desc[attributeIndex].SemanticName = currAttribute.m_name.c_str();
			desc[attributeIndex].SemanticIndex = 0;
			desc[attributeIndex].Format = static_cast<DXGI_FORMAT>(GetDXFormatForAttribute(currAttribute));
			desc[attributeIndex].InputSlot = 0U;
			desc[attributeIndex].AlignedByteOffset = (uint32)currAttribute.m_memberOffset;
			desc[attributeIndex].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			desc[attributeIndex].InstanceDataStepRate = 0U;
		}

		ID3DBlob* vsByteCode = m_vertexShader.GetCompiledSource();
		ASSERT_OR_DIE(vsByteCode != nullptr, "Attempted to create input layout for vertex stage that didn't compile!");

		ID3D11Device* dxDevice = g_renderContext->GetDxDevice();

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

		SAFE_FREE(desc);
	}

	return createdNewLayout;
}


//-------------------------------------------------------------------------------------------------
void Shader::UpdateBlendState()
{
	if (m_dxBlendState == nullptr || m_blendStateDirty)
	{
		DX_SAFE_RELEASE(m_dxBlendState);

		D3D11_BLEND_DESC blendDesc;
		memset(&blendDesc, 0, sizeof(blendDesc));

		blendDesc.AlphaToCoverageEnable = false;	// used in MSAA to treat alpha as coverage (i.e. for foliage rendering)
		blendDesc.IndependentBlendEnable = false;	// For different blends for different outputs

		// Blending is setting put the equation...
		// FinalColor = BlendOp( SrcFactor * outputColor, DestFactor * destColor )
		// where outputColor is what the pixel shader outputs
		// and destColor is the color already in the pixel shader

		// With independent blend disabled, we only need to set the 0th blend
		blendDesc.RenderTarget[0].BlendEnable = TRUE;

		// Color
		blendDesc.RenderTarget[0].BlendOp = ToDXBlendOp(m_colorBlend.m_op);      
		blendDesc.RenderTarget[0].SrcBlend = ToDXBlendFactor(m_colorBlend.m_srcFactor);
		blendDesc.RenderTarget[0].DestBlend = ToDXBlendFactor(m_colorBlend.m_dstFactor);

		// Alpha
		blendDesc.RenderTarget[0].BlendOpAlpha = ToDXBlendOp(m_alphaBlend.m_op);
		blendDesc.RenderTarget[0].SrcBlendAlpha = ToDXBlendFactor(m_alphaBlend.m_srcFactor);
		blendDesc.RenderTarget[0].DestBlendAlpha = ToDXBlendFactor(m_alphaBlend.m_dstFactor);

		// No masking
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		ID3D11Device* dxDevice = g_renderContext->GetDxDevice();
		HRESULT hr = dxDevice->CreateBlendState(&blendDesc, &m_dxBlendState);

		ASSERT_RETURN(SUCCEEDED(hr), NO_RETURN_VAL, "Couldn't create blend state!");

		m_blendStateDirty = false;
	}
}


//-------------------------------------------------------------------------------------------------
void Shader::UpdateRasterizerState()
{
	if (m_dxRasterizerState == nullptr || m_rasterizerStateDirty)
	{
		DX_SAFE_RELEASE(m_dxRasterizerState);

		D3D11_RASTERIZER_DESC rasterDesc;
		memset(&rasterDesc, 0, sizeof(rasterDesc));

		rasterDesc.FillMode = ToDXFillMode(m_fillMode);
		rasterDesc.CullMode = ToDxCullMode(m_cullMode);
		rasterDesc.FrontCounterClockwise = FALSE;
		rasterDesc.DepthBias = 0;
		rasterDesc.SlopeScaledDepthBias = 0.0f;
		rasterDesc.DepthBiasClamp = 0.0f;
		rasterDesc.DepthClipEnable = true;
		rasterDesc.ScissorEnable = m_scissorEnabled;
		rasterDesc.MultisampleEnable = false;
		rasterDesc.AntialiasedLineEnable = false;

		ID3D11Device* dxDevice = g_renderContext->GetDxDevice();
		HRESULT hr = dxDevice->CreateRasterizerState(&rasterDesc, &m_dxRasterizerState);

		ASSERT_RETURN(SUCCEEDED(hr), NO_RETURN_VAL, "Couldn't create rasterizer state!");

		// Scissor
		ID3D11DeviceContext* dxContext = g_renderContext->GetDxContext();
		if (m_scissorEnabled)
		{
			D3D11_RECT dxRect;
			dxRect = { (LONG)m_scissorRect.left, (LONG)m_scissorRect.bottom, (LONG)m_scissorRect.right, (LONG)m_scissorRect.top };
			dxContext->RSSetScissorRects(1, &dxRect);
		}
		else
		{
			dxContext->RSSetScissorRects(0, NULL);
		}

		m_rasterizerStateDirty = false;
	}
}


//-------------------------------------------------------------------------------------------------
void Shader::UpdateDepthState()
{
	if (m_dxDepthState == nullptr || m_depthStateDirty)
	{
		DX_SAFE_RELEASE(m_dxDepthState);

		D3D11_DEPTH_STENCIL_DESC dsDesc = {};

		if (m_depthMode == DEPTH_MODE_IGNORE_DEPTH)
		{
			dsDesc.DepthEnable = false;
		}
		else
		{
			// Depth test parameters
			dsDesc.DepthEnable = true;
			dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			dsDesc.DepthFunc = ToDxDepthFunc(m_depthMode);;

			// Stencil test parameters
			dsDesc.StencilEnable = true;
			dsDesc.StencilReadMask = 0xFF;
			dsDesc.StencilWriteMask = 0xFF;

			// Stencil operations if pixel is front-facing
			dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
			dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

			// Stencil operations if pixel is back-facing
			dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
			dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

			// Create depth stencil state
			HRESULT result = g_renderContext->GetDxDevice()->CreateDepthStencilState(&dsDesc, &m_dxDepthState);

			ASSERT_OR_DIE(SUCCEEDED(result), "Couldn't create a depth stencil state!");
			m_depthStateDirty = false;
		}
		
	}
}


//-------------------------------------------------------------------------------------------------
void Shader::SetBlend(BlendPreset blendPreset)
{
	BlendInfo colorBlend, alphaBlend;
	switch (blendPreset)
	{
	case BLEND_PRESET_OPAQUE:
		colorBlend.m_op = BLEND_OP_ADD;
		colorBlend.m_srcFactor = BLEND_FACTOR_ONE;
		colorBlend.m_dstFactor = BLEND_FACTOR_ZERO;
		alphaBlend.m_op = BLEND_OP_ADD;
		alphaBlend.m_srcFactor = BLEND_FACTOR_ONE;
		alphaBlend.m_dstFactor = BLEND_FACTOR_ONE;
		break;
	case BLEND_PRESET_ALPHA:
		colorBlend.m_op = BLEND_OP_ADD;
		colorBlend.m_srcFactor = BLEND_FACTOR_SRC_ALPHA;
		colorBlend.m_dstFactor = BLEND_FACTOR_INV_SRC_ALPHA;
		alphaBlend.m_op = BLEND_OP_ADD;
		alphaBlend.m_srcFactor = BLEND_FACTOR_ONE;
		alphaBlend.m_dstFactor = BLEND_FACTOR_ONE;
		break;
	case BLEND_PRESET_ADDITIVE:
		colorBlend.m_op = BLEND_OP_ADD;
		colorBlend.m_srcFactor = BLEND_FACTOR_ONE;
		colorBlend.m_dstFactor = BLEND_FACTOR_ONE;
		alphaBlend.m_op = BLEND_OP_ADD;
		alphaBlend.m_srcFactor = BLEND_FACTOR_ONE;
		alphaBlend.m_dstFactor = BLEND_FACTOR_ONE;
		break;
	default:
		ERROR_RETURN(NO_RETURN_VAL, "Invalid Blend Preset!");
		break;
	}

	SetBlend(colorBlend, alphaBlend);
}


//-------------------------------------------------------------------------------------------------
void Shader::SetBlend(const BlendInfo& colorBlend, const BlendInfo& alphaBlend)
{
	m_colorBlend = colorBlend;
	m_alphaBlend = alphaBlend;
	m_blendStateDirty = true;
}


//-------------------------------------------------------------------------------------------------
void Shader::SetColorBlend(const BlendInfo& blend)
{
	m_colorBlend = blend;
	m_blendStateDirty = true;
}


//-------------------------------------------------------------------------------------------------
void Shader::SetAlphaBlend(const BlendInfo& blend)
{
	m_alphaBlend = blend;
	m_blendStateDirty = true;
}


//-------------------------------------------------------------------------------------------------
void Shader::SetFillMode(FillMode fillMode)
{
	m_fillMode = fillMode;
	m_rasterizerStateDirty = true;
}


//-------------------------------------------------------------------------------------------------
void Shader::SetCullMode(CullMode cullMode)
{
	m_cullMode = cullMode;
	m_rasterizerStateDirty = true;
}


//-------------------------------------------------------------------------------------------------
void Shader::SetDepthMode(DepthMode depthMode)
{
	m_depthMode = depthMode;
	m_depthStateDirty = true;
}


//-------------------------------------------------------------------------------------------------
// Expects screenSpaceRect's mins to be top left on the screen, since DX is top left (0,0)
void Shader::EnableScissor(const AABB2& screenSpaceRect)
{
	m_scissorRect = screenSpaceRect;
	m_scissorEnabled = true;
	m_rasterizerStateDirty = true;
}


//-------------------------------------------------------------------------------------------------
void Shader::DisableScissor()
{
	m_scissorEnabled = false;
	m_rasterizerStateDirty = true;
}


//-------------------------------------------------------------------------------------------------
bool Shader::IsDirty() const
{
	return m_rasterizerStateDirty || m_blendStateDirty || m_depthStateDirty;
}


//-------------------------------------------------------------------------------------------------
// Returns true if the fragment shader is using light data as part of the calculation
bool Shader::IsUsingLights() const
{
	return (m_fragmentShader.GetBufferDescription(CONSTANT_BUFFER_SLOT_LIGHT) != nullptr);
}
