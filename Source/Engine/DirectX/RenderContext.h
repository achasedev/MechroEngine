///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: December 8th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** INCLUDES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include <string>

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** DEFINES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                              *** TYPES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Camera;
class ColorTargetView;
class IndexBuffer;
class Mesh;
class Shader;
class Texture2D;
class UniformBuffer;
class VertexBuffer;
class VertexLayout;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;

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

	void Draw(Mesh& m_mesh, Shader& m_shader);

	Texture2D*	CreateOrGetTexture(const std::string& name);
	Shader*		CreateOrGetShader(const std::string& name);

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

	ID3D11Device* m_device = nullptr;
	ID3D11DeviceContext* m_context = nullptr;
	IDXGISwapChain* m_swapChain = nullptr;

	// Frame State
	Camera*					m_currentCamera = nullptr;
	Shader*					m_currentShader = nullptr;
	const VertexLayout*		m_currVertexLayout = nullptr;
	ColorTargetView*		m_frameBackbufferRtv = nullptr;

	static RenderContext* s_renderContext;

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                           *** C FUNCTIONS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------
