///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: December 8th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Render/Buffer/UniformBuffer.h"
#include "Engine/Render/Buffer/VertexBuffer.h"
#include "Engine/Render/Camera/Camera.h"
#include "Engine/Render/Core/DrawCall.h"
#include "Engine/Render/Core/DX11Common.h"
#include "Engine/Render/Core/Renderable.h"
#include "Engine/Render/Core/RenderContext.h"
#include "Engine/Render/Material.h"
#include "Engine/Render/Mesh/Mesh.h"
#include "Engine/Render/Mesh/Vertex.h"
#include "Engine/Render/Sampler.h"
#include "Engine/Render/Shader.h"
#include "Engine/Render/View/RenderTargetView.h"
#include "Engine/Render/View/ShaderResourceView.h"
#include "Engine/Render/View/DepthStencilTargetView.h"
#include "Engine/Render/Texture/Texture2D.h"
#include "Engine/IO/File.h"
#include "Engine/Framework/Window.h"
#include "Engine/IO/Image.h"
#include "Engine/Math/MathUtils.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
enum UniformSlot
{
	UNIFORM_SLOT_FRAME_TIME = 1,
	UNIFORM_SLOT_CAMERA = 2,
	UNIFORM_SLOT_MODEL_MATRIX = 3
};

struct FrameTimeBufferData
{
	float m_deltaTimeSeconds;
	float m_totalTime;
	float m_padding0;
	float m_padding1;
};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------
RenderContext* g_renderContext = nullptr;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
void RenderContext::Initialize()
{
	ASSERT_OR_DIE(g_renderContext == nullptr, "RenderContext is already initialized!");
	g_renderContext = new RenderContext();

	g_renderContext->DxInit();
	g_renderContext->PostDxInit();
}


//-------------------------------------------------------------------------------------------------
void RenderContext::Shutdown()
{
	SAFE_DELETE_POINTER(g_renderContext);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::BeginFrame()
{
	// Get the new backbuffer
	ID3D11Texture2D* backbuffer = nullptr;
	m_dxSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backbuffer);

	// Store it off
	m_defaultColorTarget->CreateFromDxTexture2D(backbuffer);

	// Release the local reference to the backbuffer
	DX_SAFE_RELEASE(backbuffer);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::EndFrame()
{
	// Swap buffers
	m_dxSwapChain->Present(0, 0);

	// TEMP - Move to EndCamera()
	m_dxContext->OMSetRenderTargets(0, nullptr, nullptr);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::BeginCamera(Camera* camera)
{
	m_currentCamera = camera;

	// Render to the camera's target
	RenderTargetView* colorView = camera->GetColorTargetView();
	ASSERT_OR_DIE(colorView != nullptr, "Beginning camera will null target view!");

	ID3D11RenderTargetView* rtv = colorView->GetDxHandle();
	
	DepthStencilTargetView* depthView = camera->GetDepthStencilTargetView();
	ID3D11DepthStencilView* dsv = (depthView != nullptr ? depthView->GetDxHandle() : nullptr);
	
	m_dxContext->OMSetRenderTargets(1, &rtv, dsv);
	
	// Viewport
	D3D11_VIEWPORT viewport;
	memset(&viewport, 0, sizeof(viewport));
	viewport.TopLeftX = 0U;
	viewport.TopLeftY = 0U;
	viewport.Width = (FLOAT)colorView->GetWidth();
	viewport.Height = (FLOAT)colorView->GetHeight();
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	m_dxContext->RSSetViewports(1, &viewport);

	// Uniform Buffer
	camera->UpdateUBO();
	BindUniformBuffer(UNIFORM_SLOT_CAMERA, camera->GetUniformBuffer());
}


//-------------------------------------------------------------------------------------------------
void RenderContext::EndCamera()
{
	m_dxContext->OMSetRenderTargets(0, nullptr, nullptr);
	m_currentCamera = nullptr;
}


//-------------------------------------------------------------------------------------------------
void RenderContext::ClearScreen(const Rgba& color)
{
	ASSERT_OR_DIE(m_currentCamera != nullptr, "No Camera bound!");

	float colors[4];
	colors[0] = color.GetRedFloat();
	colors[1] = color.GetGreenFloat();
	colors[2] = color.GetBlueFloat();
	colors[3] = color.GetAlphaFloat();

	if (m_currentCamera->GetColorTargetView() != nullptr)
	{
		ID3D11RenderTargetView* dxRTV = nullptr;
		dxRTV = m_currentCamera->GetColorTargetView()->GetDxHandle();
		m_dxContext->ClearRenderTargetView(dxRTV, colors);
	}
}


//-------------------------------------------------------------------------------------------------
void RenderContext::ClearDepth(float depth /*= 1.0f*/)
{
	ASSERT_OR_DIE(m_currentCamera != nullptr, "No Camera bound!");

	if (m_currentCamera->GetDepthStencilTargetView() != nullptr)
	{
		ID3D11DepthStencilView* dxView = nullptr;
		dxView = m_currentCamera->GetDepthStencilTargetView()->GetDxHandle();
		m_dxContext->ClearDepthStencilView(dxView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, 0U);
	}
}


//-------------------------------------------------------------------------------------------------
void RenderContext::BindUniformBuffer(uint32 slot, UniformBuffer* ubo)
{
	ID3D11Buffer *buffer = (ubo != nullptr) ? ubo->GetDxHandle() : nullptr;
	m_dxContext->VSSetConstantBuffers(slot, 1U, &buffer);
	m_dxContext->PSSetConstantBuffers(slot, 1U, &buffer);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::BindMaterial(Material* material)
{
	ASSERT_OR_DIE(material != nullptr, "No material defaults set up yet!");

	// Bind Texture + Sampler
	BindShaderResourceView(SRV_SLOT_ALBEDO, material->GetShaderResourceView(SRV_SLOT_ALBEDO));

	// Bind Shader
	BindShader(material->GetShader());
}


//-------------------------------------------------------------------------------------------------
void RenderContext::BindShader(Shader* shader)
{
	if (m_currentShader != shader)
	{
		m_dxContext->VSSetShader(shader->GetVertexStage(), 0, 0);
		m_dxContext->PSSetShader(shader->GetFragmentStage(), 0, 0);
		m_currentShader = shader;
	}
}


//-------------------------------------------------------------------------------------------------
void RenderContext::BindShaderResourceView(uint32 slot, ShaderResourceView* view)
{
	// TODO: Default the view to a sensible texture if null
	ASSERT_OR_DIE(view != nullptr, "Null TextureView!");

	BindSampler(0U, view->GetSampler());

	ID3D11ShaderResourceView* dxViewHandle = view->GetDxHandle();
	m_dxContext->PSSetShaderResources(slot, 1U, &dxViewHandle);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::BindSampler(uint32 slot, Sampler* sampler)
{
	if (sampler == nullptr)
	{
		sampler = m_samplers[m_samplerMode];
	}

	sampler->CreateOrUpdate();

	ID3D11SamplerState* handle = sampler->GetDxSamplerState();
	m_dxContext->PSSetSamplers(slot, 1U, &handle);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::UpdateModelMatrixUBO(const Matrix44& modelMatrix)
{
	m_modelMatrixUBO.CopyToGPU(&modelMatrix, sizeof(Matrix44));
}


//-------------------------------------------------------------------------------------------------
void RenderContext::DrawMesh(Mesh& mesh)
{
	DrawMeshWithMaterial(mesh, nullptr);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::DrawMeshWithMaterial(Mesh& mesh, Material* material)
{
	Renderable immediateRenderable;
	immediateRenderable.AddDraw(&mesh, material);

	DrawRenderable(immediateRenderable);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::DrawRenderable(Renderable& renderable)
{
	uint32 numDraws = renderable.GetNumDrawCalls();
	ASSERT_RECOVERABLE(numDraws > 0, "Renderable drawn with no draws!");

	for (uint32 drawIndex = 0; drawIndex < numDraws; ++drawIndex)
	{
		DrawCall dc;
		dc.SetFromRenderable(renderable, drawIndex);

		Draw(dc);
	}
}


//-------------------------------------------------------------------------------------------------
void RenderContext::Draw(const DrawCall& drawCall)
{
	BindMaterial(drawCall.GetMaterial());
	
	Mesh* mesh = drawCall.GetMesh();
	ASSERT_OR_DIE(mesh != nullptr, "Attempting to draw null mesh!");

	BindVertexStream(mesh->GetVertexBuffer());
	BindIndexStream(mesh->GetIndexBuffer());
	UpdateInputLayout(mesh->GetVertexLayout());
	UpdateModelMatrixUBO(drawCall.GetModelMatrix());

	// TODO: Other render state? (cull mode, fill mode, winding order, blending, depth func)

	DrawInstruction draw = mesh->GetDrawInstruction();
	if (draw.m_useIndices)
	{
		m_dxContext->DrawIndexed(draw.m_elementCount, draw.m_startIndex, 0);
	}
	else
	{
		m_dxContext->Draw(draw.m_elementCount, draw.m_startIndex);
	}
}


//-------------------------------------------------------------------------------------------------
void RenderContext::DxInit()
{
	HWND hwnd = (HWND)g_window->GetWindowContext();

	// Creation Flags
	unsigned int device_flags = 0U;

	// Setup our Swap Chain
	DXGI_SWAP_CHAIN_DESC swap_desc;
	memset(&swap_desc, 0, sizeof(swap_desc));

	swap_desc.BufferCount = 1;										// number of back buffers
	swap_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;		// how swap chain is to be used
	swap_desc.OutputWindow = hwnd;									// the window to be copied to on present
	swap_desc.SampleDesc.Count = 1;									// how many multisamples (1 means no multi sampling)
	swap_desc.BufferDesc.Width = g_window->GetClientPixelWidth();	// Drawable space width
	swap_desc.BufferDesc.Height = g_window->GetClientPixelHeight();	// Drawable space height
	swap_desc.Windowed = TRUE;										// windowed/full-screen mode
	swap_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;		// use 32-bit color
	swap_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// Actually Create
	HRESULT hr = ::D3D11CreateDeviceAndSwapChain(nullptr, // Adapter, if nullptr, will use adapter window is primarily on.
		D3D_DRIVER_TYPE_HARDWARE,	// Driver Type - We want to use the GPU (HARDWARE)
		nullptr,					// Software Module - DLL that implements software mode (we do not use)
		device_flags,				// device creation options
		nullptr,					// feature level (use default)
		0U,							// number of feature levels to attempt
		D3D11_SDK_VERSION,			// SDK Version to use
		&swap_desc,					// Description of our swap chain
		&m_dxSwapChain,				// Swap Chain we're creating
		&m_dxDevice,				// [out] The device created
		nullptr,					// [out] Feature Level Acquired
		&m_dxContext);				// Context that can issue commands on this pipe.

	ASSERT_OR_DIE(SUCCEEDED(hr), "D3D11CreateDeviceAndSwapChain failed!");
}


//-------------------------------------------------------------------------------------------------
void RenderContext::PostDxInit()
{
	// Only triangle lists for now
	m_dxContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Samplers
	Sampler* pointSampler = new Sampler();
	pointSampler->SetFilterModes(FILTER_MODE_POINT, FILTER_MODE_POINT);
	m_samplers[SAMPLER_MODE_POINT] = pointSampler;

	Sampler* linearSampler = new Sampler();
	linearSampler->SetFilterModes(FILTER_MODE_LINEAR, FILTER_MODE_LINEAR);
	m_samplers[SAMPLER_MODE_LINEAR] = linearSampler;

	// Default color/depth target
	InitDefaultColorAndDepthViews();

	// Model matrix UBO
	UpdateModelMatrixUBO(Matrix44::IDENTITY);
	BindUniformBuffer(UNIFORM_SLOT_MODEL_MATRIX, &m_modelMatrixUBO);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::InitDefaultColorAndDepthViews()
{
	// Get current back buffer
	ID3D11Texture2D* backbuffer = nullptr;
	m_dxSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backbuffer);
	
	// Get the back buffer bounds
	D3D11_TEXTURE2D_DESC desc;
	backbuffer->GetDesc(&desc);

	// Color target
	m_defaultColorTarget = new Texture2D();
	m_defaultColorTarget->CreateFromDxTexture2D(backbuffer);

	// Depth target
	m_defaultDepthStencilTarget = new Texture2D();
	m_defaultDepthStencilTarget->CreateAsDepthStencilTarget(desc.Width, desc.Height);

	// Create default views for both
	m_defaultColorTarget->CreateOrGetColorTargetView();
	m_defaultDepthStencilTarget->CreateOrGetDepthStencilTargetView();

	// Release the local reference to the backbuffer
	DX_SAFE_RELEASE(backbuffer);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::BindVertexStream(const VertexBuffer* vbo)
{
	const VertexLayout* layout = vbo->GetVertexLayout();
	ASSERT_OR_DIE(layout != nullptr, "VertexBuffer had null layout!");

	ID3D11Buffer* handle = vbo->GetDxHandle();
	uint32 stride = layout->GetStride();
	uint32 offset = 0U;
	
	m_dxContext->IASetVertexBuffers(0, 1, &handle, &stride, &offset);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::BindIndexStream(const IndexBuffer* ibo)
{
	ID3D11Buffer* handle = nullptr;
	if (ibo != nullptr)
	{
		handle = ibo->GetDxHandle();
	}

	m_dxContext->IASetIndexBuffer(handle, DXGI_FORMAT_R32_UINT, 0);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::UpdateInputLayout(const VertexLayout* vertexLayout)
{
	// Don't rebind if it's the same layout as previous draw
	if (m_currVertexLayout != vertexLayout)
	{
		m_currentShader->CreateInputLayoutForVertexLayout(vertexLayout);
		m_dxContext->IASetInputLayout(m_currentShader->GetInputLayout());
		m_currVertexLayout = vertexLayout;
	}
}


//-------------------------------------------------------------------------------------------------
RenderContext::~RenderContext()
{
	SAFE_DELETE_POINTER(m_samplers[SAMPLER_MODE_POINT]);
	SAFE_DELETE_POINTER(m_samplers[SAMPLER_MODE_LINEAR]);

	SAFE_DELETE_POINTER(m_defaultColorTarget);
	SAFE_DELETE_POINTER(m_defaultDepthStencilTarget);

	// Shutdown DirectX
	// DX11 cannot shutdown in full screen
	m_dxSwapChain->SetFullscreenState(FALSE, NULL);

	DX_SAFE_RELEASE(m_dxSwapChain);
	DX_SAFE_RELEASE(m_dxContext);
	DX_SAFE_RELEASE(m_dxDevice);
}


//-------------------------------------------------------------------------------------------------
ID3D11Device* RenderContext::GetDxDevice()
{
	return m_dxDevice;
}


//-------------------------------------------------------------------------------------------------
ID3D11DeviceContext* RenderContext::GetDxContext()
{
	return m_dxContext;
}


//-------------------------------------------------------------------------------------------------
IDXGISwapChain* RenderContext::GetDxSwapChain()
{
	return m_dxSwapChain;
}


//-------------------------------------------------------------------------------------------------
RenderTargetView* RenderContext::GetDefaultColorTargetView() const
{
	return m_defaultColorTarget->CreateOrGetColorTargetView();
}


//-------------------------------------------------------------------------------------------------
DepthStencilTargetView* RenderContext::GetDefaultDepthStencilTargetView() const
{
	return m_defaultDepthStencilTarget->CreateOrGetDepthStencilTargetView();
}
