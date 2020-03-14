///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 9th, 2020
/// Description: GLSL Shaders!
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** INCLUDES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/DirectX/DX11Common.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** DEFINES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                              *** TYPES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
struct ID3D11InputLayout;
struct ID3D11PixelShader;
struct ID3D11Resource;
struct ID3D11VertexShader;
class VertexLayout;

struct ShaderInputLayout
{
	ID3D11InputLayout* m_dxInputLayout = nullptr;
	const VertexLayout* m_vertexLayoutUsed = nullptr;
};

//-------------------------------------------------------------------------------------------------
enum ShaderStageType
{
	SHADER_STAGE_INVALID,
	SHADER_STAGE_VERTEX,
	SHADER_STAGE_FRAGMENT
};

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** STRUCTS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                        *** GLOBALS AND STATICS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** CLASSES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class ShaderStage
{
public:
	//-----Public Methods-----

	ShaderStage() {}
	~ShaderStage();

	bool LoadFromShaderSource(const char* filename, const void* source, const size_t sourceByteSize, ShaderStageType stageType);
	bool IsValid() const { return m_handle != nullptr; }

	ID3D11VertexShader* GetAsVertexShader() const { return m_vertexShader; }
	ID3D11PixelShader* GetAsFragmentShader() const { return m_fragmentShader; }
	ID3DBlob* GetCompiledSource() const { return m_compiledSource; }

private:
	//-----Private Data-----

	ShaderStageType m_stageType = SHADER_STAGE_INVALID;
	ID3DBlob* m_compiledSource = nullptr;

	union
	{
		ID3D11Resource* m_handle = nullptr;
		ID3D11VertexShader* m_vertexShader;
		ID3D11PixelShader* m_fragmentShader;
	};

};

//-------------------------------------------------------------------------------------------------
class Shader
{
public:
	//-----Public Methods-----

	bool CreateFromFile(const char* filename);
	bool CreateInputLayoutForVertexLayout(const VertexLayout* vertexLayout);

	ID3D11VertexShader* GetVertexStage() { return m_vertexShader.GetAsVertexShader(); }
	ID3D11PixelShader* GetFragmentStage() { return m_fragmentShader.GetAsFragmentShader(); }
	ID3D11InputLayout* GetInputLayout() { return m_shaderInputLayout.m_dxInputLayout; }


private:
	//-----Private Data-----

	ShaderStage m_vertexShader;
	ShaderStage m_fragmentShader;
	ShaderInputLayout m_shaderInputLayout;

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                           *** C FUNCTIONS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------
