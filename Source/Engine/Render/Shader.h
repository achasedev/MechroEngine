///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 9th, 2020
/// Description: GLSL Shaders!
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Math/AABB2.h"
#include "Engine/Render/DX11Common.h"
#include "Engine/Resource/Resource.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
struct ID3D11InputLayout;
struct ID3D11PixelShader;
struct ID3D11Resource;
struct ID3D11RasterizerState;
struct ID3D11VertexShader;
class VertexLayout;

// Vertex information
struct ShaderInputLayout
{
	ID3D11InputLayout* m_dxInputLayout = nullptr;
	const VertexLayout* m_vertexLayoutUsed = nullptr;
};

enum ShaderStageType
{
	SHADER_STAGE_INVALID,
	SHADER_STAGE_VERTEX,
	SHADER_STAGE_FRAGMENT
};

// 1:1 to DX blend ops
enum BlendOp
{
	BLEND_OP_ADD,
	BLEND_OP_SUBTRACT,
	BLEND_OP_REV_SUBTRACT,
	BLEND_OP_MIN,
	BLEND_OP_MAX,
	NUM_BLEND_OPS
};

// 1:1 to DX blend factors
enum BlendFactor
{
	BLEND_FACTOR_ZERO,
	BLEND_FACTOR_ONE,
	BLEND_FACTOR_SRC_COLOR,
	BLEND_FACTOR_INV_SRC_COLOR,
	BLEND_FACTOR_SRC_ALPHA,
	BLEND_FACTOR_INV_SRC_ALPHA,
	BLEND_FACTOR_DEST_ALPHA,
	BLEND_FACTOR_INV_DEST_ALPHA,
	BLEND_FACTOR_DEST_COLOR,
	BLEND_FACTOR_INV_DEST_COLOR,
	BLEND_FACTOR_SRC_ALPHA_SAT,
	BLEND_FACTOR_BLEND_FACTOR,
	BLEND_FACTOR_INV_BLEND_FACTOR,
	BLEND_FACTOR_SRC1_COLOR,
	BLEND_FACTOR_INV_SRC1_COLOR,
	BLEND_FACTOR_SRC1_ALPHA,
	BLEND_FACTOR_INV_SRC1_ALPHA,
	NUM_BLEND_FACTORS
};

// For setting blend mode factors and operators
struct BlendInfo
{
	BlendOp		m_op;
	BlendFactor m_srcFactor;
	BlendFactor m_dstFactor;
};

// Convenience common blend modes
enum BlendPreset
{
	BLEND_PRESET_OPAQUE,
	BLEND_PRESET_ALPHA,
	BLEND_PRESET_ADDITIVE
};

enum FillMode
{
	FILL_MODE_SOLID,
	FILL_MODE_WIREFRAME
};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class ShaderStage
{
public:
	//-----Public Methods-----

	ShaderStage() {}
	~ShaderStage();

	void				Clear();
	bool				LoadFromShaderSource(const char* filename, const void* source, const size_t sourceByteSize, ShaderStageType stageType);
	bool				IsValid() const { return m_handle != nullptr; }

	ID3D11VertexShader* GetAsVertexShader() const { return m_vertexShader; }
	ID3D11PixelShader*	GetAsFragmentShader() const { return m_fragmentShader; }
	ID3DBlob*			GetCompiledSource() const { return m_compiledSource; }


private:
	//-----Private Data-----

	ShaderStageType m_stageType = SHADER_STAGE_INVALID;
	ID3DBlob* m_compiledSource = nullptr;

	union
	{
		ID3D11Resource*		m_handle = nullptr;
		ID3D11VertexShader* m_vertexShader;
		ID3D11PixelShader*	m_fragmentShader;
	};

};

//-------------------------------------------------------------------------------------------------
class Shader : public Resource
{
public:
	//-----Public Methods-----

	Shader();
	~Shader();

	bool					Load(const char* filepath);
	void					Clear();

	bool					LoadAndCompileShaderSource(const char* filename);
	bool					CreateInputLayoutForVertexLayout(const VertexLayout* vertexLayout);
	void					UpdateBlendState();
	void					UpdateRasterizerState();

	void					SetBlend(BlendPreset blendPreset);
	void					SetBlend(const BlendInfo& colorBlend, const BlendInfo& alphaBlend);
	void					SetColorBlend(const BlendInfo& blend);
	void					SetAlphaBlend(const BlendInfo& blend);
	void					SetFillMode(FillMode fillMode);
	void					EnableScissor(const AABB2& screenSpaceRect);
	void					DisableScissor();

	bool					IsDirty() const;
	FillMode				GetFillMode() const { return m_fillMode; }

	ID3D11VertexShader*		GetVertexStage() const { return m_vertexShader.GetAsVertexShader(); }
	ID3D11PixelShader*		GetFragmentStage() const { return m_fragmentShader.GetAsFragmentShader(); }
	ID3D11InputLayout*		GetInputLayout() const { return m_shaderInputLayout.m_dxInputLayout; }
	ID3D11BlendState*		GetDXBlendState() const { return m_dxBlendState; }
	ID3D11RasterizerState*	GetDXRasterizerState() const { return m_dxRasterizerState; }


private:
	//-----Private Data-----

	ShaderStage				m_vertexShader;
	ShaderStage				m_fragmentShader;
	ShaderInputLayout		m_shaderInputLayout;

	BlendInfo				m_colorBlend;
	BlendInfo				m_alphaBlend;
	ID3D11BlendState*		m_dxBlendState = nullptr;
	bool					m_blendStateDirty = true;

	FillMode				m_fillMode = FILL_MODE_SOLID;
	AABB2					m_scissorRect;
	bool					m_scissorEnabled = false;
	ID3D11RasterizerState*	m_dxRasterizerState = nullptr;
	bool					m_rasterizerStateDirty = true;

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
