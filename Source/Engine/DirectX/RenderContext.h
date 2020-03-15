///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: December 8th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include <string>

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Camera;
class ColorTargetView;
class IndexBuffer;
class Mesh;
class Sampler;
class Shader;
class TextureView;
class UniformBuffer;
class VertexBuffer;
class VertexLayout;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;

enum SamplerMode
{
	SAMPLER_MODE_POINT,
	SAMPLER_MODE_LINEAR,
	NUM_SAMPLER_MODES
};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class RenderContext
{
public:
	//-----Public Methods-----

	static void Initialize();
	static void Shutdown();
	
	static RenderContext* GetInstance();

	void BeginFrame();
	void EndFrame();

	void BeginCamera(Camera* camera);
	void EndCamera();

	void ClearScreen();

	void BindUniformBuffer(uint32 slot, UniformBuffer* ubo);
	void BindShader(Shader* shader);
	void BindTextureView(uint32 slot, TextureView* view);
	void BindSampler(uint32 slot, Sampler* sampler);

	void Draw(Mesh& m_mesh, Shader& m_shader);

	ID3D11Device* GetDxDevice();
	ID3D11DeviceContext* GetDxContext();
	IDXGISwapChain* GetDxSwapChain();
	ColorTargetView* GetBackBufferColorTarget();


private:
	//-----Private Methods-----

	RenderContext();
	~RenderContext();
	RenderContext(const RenderContext& copy) = delete;


	void BindVertexStream(const VertexBuffer* vbo);
	void BindIndexStream(const IndexBuffer* ibo);
	void SetInputLayout(const VertexLayout* vertexLayout);


private:
	//-----Private Data-----

	ID3D11Device*			m_dxDevice = nullptr;
	ID3D11DeviceContext*	m_dxContext = nullptr;
	IDXGISwapChain*			m_dxSwapChain = nullptr;

	// Frame State
	Camera*					m_currentCamera = nullptr;
	Shader*					m_currentShader = nullptr;
	const VertexLayout*		m_currVertexLayout = nullptr;
	ColorTargetView*		m_frameBackbufferRtv = nullptr;

	// Sampler
	SamplerMode				m_samplerMode = SAMPLER_MODE_LINEAR;
	Sampler*				m_samplers[NUM_SAMPLER_MODES];

	static RenderContext* s_renderContext;

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
