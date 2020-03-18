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
#include "Engine/Render/Texture/ColorTargetView.h"
#include "Engine/Render/Texture/Texture2D.h"
#include "Engine/Render/Texture/TextureView2D.h"
#include "Engine/Framework/File.h"
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

// TEMP
ID3D11VertexShader* gVertexShader;
ID3D11PixelShader* gPixelShader;
ID3D11Buffer *gVertexBuffer;
ID3D11InputLayout* gInputLayout;

RenderContext* RenderContext::s_renderContext = nullptr;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
void RenderContext::Initialize()
{
	ASSERT_OR_DIE(s_renderContext == nullptr, "RenderContext is already initialized!");
	s_renderContext = new RenderContext();
}


//-------------------------------------------------------------------------------------------------
void RenderContext::Shutdown()
{
	SAFE_DELETE_POINTER(s_renderContext);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::BeginFrame()
{
	// Get the new backbuffer
	ID3D11Texture2D* backbuffer = nullptr;
	m_dxSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backbuffer);

	// Store it off
	m_frameBackbufferRtv->InitForTexture(backbuffer);

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
	ColorTargetView* view = camera->GetColorTarget();
	ID3D11RenderTargetView* rtv = view->GetDX11RenderTargetView();
	m_dxContext->OMSetRenderTargets(1, &rtv, nullptr);
	
	// Viewport
	D3D11_VIEWPORT viewport;
	memset(&viewport, 0, sizeof(viewport));
	viewport.TopLeftX = 0U;
	viewport.TopLeftY = 0U;
	viewport.Width = (FLOAT)view->GetWidth();
	viewport.Height = (FLOAT)view->GetHeight();
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
void RenderContext::ClearScreen()
{
	// clear the back buffer to a changing color
	static float test = 0.f;
	test += 0.0001f;
	test = ModFloat(test, 1.0f);

	float color[4] = { 0.0f, test, Clamp(test, 0.f, 1.0f), Clamp(2.f * test, 0.f, 1.0f) };
	m_dxContext->ClearRenderTargetView(m_frameBackbufferRtv->GetDX11RenderTargetView(), color);
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
	BindTextureView(TEXTURE_SLOT_ALBEDO, material->GetTextureView(TEXTURE_SLOT_ALBEDO));

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
void RenderContext::BindTextureView(uint32 slot, TextureView* view)
{
	// TODO: Default the view to a sensible texture if null
	ASSERT_OR_DIE(view != nullptr, "Null TextureView!");

	BindSampler(0, view->GetSampler());

	ID3D11ShaderResourceView* dxViewHandle = view->GetDxViewHandle();
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

	// TODO: Bind Model Matrix
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
RenderContext::RenderContext()
{
	Window* window = Window::GetInstance();
	HWND hwnd = (HWND)window->GetWindowContext();

	// Creation Flags
	unsigned int device_flags = 0U;

	// Setup our Swap Chain
	DXGI_SWAP_CHAIN_DESC swap_desc;
	memset(&swap_desc, 0, sizeof(swap_desc));

	swap_desc.BufferCount = 1;										// number of back buffers
	swap_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;		// how swap chain is to be used
	swap_desc.OutputWindow = hwnd;									// the window to be copied to on present
	swap_desc.SampleDesc.Count = 1;									// how many multisamples (1 means no multi sampling)
	swap_desc.BufferDesc.Width = window->GetClientPixelWidth();		// Drawable space width
	swap_desc.BufferDesc.Height = window->GetClientPixelHeight();	// Drawable space height
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
		&m_dxDevice,					// [out] The device created
		nullptr,					// [out] Feature Level Acquired
		&m_dxContext);				// Context that can issue commands on this pipe.

	ASSERT_OR_DIE(SUCCEEDED(hr), "D3D11CreateDeviceAndSwapChain failed!");

	m_frameBackbufferRtv = new ColorTargetView();

	// Only triangle lists for now
	m_dxContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Immediate Mesh
	m_immediateMesh = new Mesh();

	// Create samplers
	Sampler* pointSampler = new Sampler();
	pointSampler->SetFilterModes(FILTER_MODE_POINT, FILTER_MODE_POINT);
	m_samplers[SAMPLER_MODE_POINT] = pointSampler;

	Sampler* linearSampler = new Sampler();
	linearSampler->SetFilterModes(FILTER_MODE_LINEAR, FILTER_MODE_LINEAR);
	m_samplers[SAMPLER_MODE_LINEAR] = linearSampler;
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
	SAFE_DELETE_POINTER(m_frameBackbufferRtv);

	// DX11 cannot shutdown in full screen
	m_dxSwapChain->SetFullscreenState(FALSE, NULL);

	DX_SAFE_RELEASE(m_dxSwapChain);
	DX_SAFE_RELEASE(m_dxContext);
	DX_SAFE_RELEASE(m_dxDevice);
}


//-------------------------------------------------------------------------------------------------
RenderContext* RenderContext::GetInstance()
{
	ASSERT_OR_DIE(s_renderContext != nullptr, "RenderContext not created yet!");
	return s_renderContext;
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
ColorTargetView* RenderContext::GetBackBufferColorTarget()
{
	return m_frameBackbufferRtv;
}
