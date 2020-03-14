///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: December 8th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** INCLUDES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/DirectX/Camera.h"
#include "Engine/DirectX/ColorTargetView.h"
#include "Engine/DirectX/DX11Common.h"
#include "Engine/DirectX/Mesh.h"
#include "Engine/DirectX/RenderContext.h"
#include "Engine/DirectX/Shader.h"
#include "Engine/DirectX/UniformBuffer.h"
#include "Engine/DirectX/Vertex.h"
#include "Engine/DirectX/VertexBuffer.h"
#include "Engine/Framework/File.h"
#include "Engine/Framework/Window.h"
#include "Engine/Math/MathUtils.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** DEFINES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                              *** TYPES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
enum UniformSlot
{
	UNIFORM_SLOT_FRAME_TIME = 1,
	UNIFORM_SLOT_CAMERA = 2,
};

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** STRUCTS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
struct FrameTimeBufferData
{
	float m_deltaTimeSeconds;
	float m_totalTime;
	float m_padding0;
	float m_padding1;
};

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                        *** GLOBALS AND STATICS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

// TEMP
ID3D11VertexShader* gVertexShader;
ID3D11PixelShader* gPixelShader;
ID3D11Buffer *gVertexBuffer;
ID3D11InputLayout* gInputLayout;

RenderContext* RenderContext::s_renderContext = nullptr;

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                           *** C FUNCTIONS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** CLASSES ***
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
	m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backbuffer);

	// Store it off
	m_frameBackbufferRtv->InitForTexture(backbuffer);

	// Release the local reference to the backbuffer
	DX_SAFE_RELEASE(backbuffer);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::EndFrame()
{
	// Swap buffers
	m_swapChain->Present(0, 0);

	// TEMP - Move to EndCamera()
	m_context->OMSetRenderTargets(0, nullptr, nullptr);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::BeginCamera(Camera* camera)
{
	m_currentCamera = camera;

	// Render to the camera's target
	ColorTargetView* view = camera->GetColorTarget();
	ID3D11RenderTargetView* rtv = view->GetDX11RenderTargetView();
	m_context->OMSetRenderTargets(1, &rtv, nullptr);
	
	// Viewport
	D3D11_VIEWPORT viewport;
	memset(&viewport, 0, sizeof(viewport));
	viewport.TopLeftX = 0U;
	viewport.TopLeftY = 0U;
	viewport.Width = (FLOAT)view->GetWidth();
	viewport.Height = (FLOAT)view->GetHeight();
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	m_context->RSSetViewports(1, &viewport);

	// Uniform Buffer
	camera->UpdateUBO();
	BindUniformBuffer(UNIFORM_SLOT_CAMERA, camera->GetUniformBuffer());
}


//-------------------------------------------------------------------------------------------------
void RenderContext::EndCamera()
{
	m_context->OMSetRenderTargets(0, nullptr, nullptr);
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
	m_context->ClearRenderTargetView(m_frameBackbufferRtv->GetDX11RenderTargetView(), color);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::BindUniformBuffer(uint32 slot, UniformBuffer* ubo)
{
	ID3D11Buffer *buffer = (ubo != nullptr) ? ubo->GetBufferHandle() : nullptr;
	m_context->VSSetConstantBuffers(slot, 1U, &buffer);
	m_context->PSSetConstantBuffers(slot, 1U, &buffer);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::BindShader(Shader* shader)
{
	if (m_currentShader != shader)
	{
		m_context->VSSetShader(shader->GetVertexStage(), 0, 0);
		m_context->PSSetShader(shader->GetFragmentStage(), 0, 0);
		m_currentShader = shader;
	}
}

//-------------------------------------------------------------------------------------------------
void RenderContext::Draw(Mesh& m_mesh, Shader& m_shader)
{
	BindShader(&m_shader);
	BindVertexStream(m_mesh.GetVertexBuffer());
	BindIndexStream(m_mesh.GetIndexBuffer());
	SetInputLayout(m_mesh.GetVertexLayout());

	// Draw or DrawIndexed
	DrawInstruction draw = m_mesh.GetDrawInstruction();
	if (draw.m_useIndices)
	{
		m_context->DrawIndexed(draw.m_elementCount, draw.m_startIndex, 0);
	}
	else
	{
		m_context->Draw(draw.m_elementCount, draw.m_startIndex);
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
		&m_swapChain,				// Swap Chain we're creating
		&m_device,					// [out] The device created
		nullptr,					// [out] Feature Level Acquired
		&m_context);				// Context that can issue commands on this pipe.

	ASSERT_OR_DIE(SUCCEEDED(hr), "D3D11CreateDeviceAndSwapChain failed!");

	m_frameBackbufferRtv = new ColorTargetView();

	// TODO: Create enums for various topologies and add as state to RenderContext
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::BindVertexStream(const VertexBuffer* vbo)
{
	const VertexLayout* layout = vbo->GetVertexLayout();
	ASSERT_OR_DIE(layout != nullptr, "VertexBuffer had null layout!");

	ID3D11Buffer* handle = vbo->GetBufferHandle();
	uint32 stride = layout->GetStride();
	uint32 offset = 0U;
	
	m_context->IASetVertexBuffers(0, 1, &handle, &stride, &offset);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::BindIndexStream(const IndexBuffer* ibo)
{
	ID3D11Buffer* handle = nullptr;
	if (ibo != nullptr)
	{
		handle = ibo->GetBufferHandle();
	}

	m_context->IASetIndexBuffer(handle, DXGI_FORMAT_R32_UINT, 0);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::SetInputLayout(const VertexLayout* vertexLayout)
{
	// Won't create a new vertex layout if already created
	if (m_currVertexLayout != vertexLayout)
	{
		m_currentShader->CreateInputLayoutForVertexLayout(vertexLayout);
		m_context->IASetInputLayout(m_currentShader->GetInputLayout());
		m_currVertexLayout = vertexLayout;
	}
}


//-------------------------------------------------------------------------------------------------
RenderContext::~RenderContext()
{
	SAFE_DELETE_POINTER(m_frameBackbufferRtv);

	// DX11 cannot shutdown in full screen
	m_swapChain->SetFullscreenState(FALSE, NULL);

	DX_SAFE_RELEASE(m_swapChain);
	DX_SAFE_RELEASE(m_context);
	DX_SAFE_RELEASE(m_device);
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
	return m_device;
}


//-------------------------------------------------------------------------------------------------
ID3D11DeviceContext* RenderContext::GetDxContext()
{
	return m_context;
}


//-------------------------------------------------------------------------------------------------
IDXGISwapChain* RenderContext::GetDxSwapChain()
{
	return m_swapChain;
}


//-------------------------------------------------------------------------------------------------
ColorTargetView* RenderContext::GetBackBufferColorTarget()
{
	return m_frameBackbufferRtv;
}
