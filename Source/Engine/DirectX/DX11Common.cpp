#include <d3d11.h>
#include <dxgi.h>
#include <dxgidebug.h>
#include <d3dcompiler.h>
#include "Engine/Framework/Window.h"

#pragma comment (lib, "d3d11.lib")
#pragma comment( lib, "dxgi.lib" ) 
#pragma comment( lib, "d3dcompiler.lib" ) 
#include "Engine/Math/Vector3.h"

ID3D11Device *gD3DDevice = nullptr;
ID3D11DeviceContext *gD3DContext = nullptr;
IDXGISwapChain *gD3DSwapChain = nullptr;
ID3D11RenderTargetView *backbuffer;    // global declaration

ID3D11VertexShader* gVertexShader;
ID3D11PixelShader* gPixelShader;

ID3D11Buffer *gVertexBuffer;
ID3D11InputLayout* gInputLayout;

struct VertexPC
{
	VertexPC(const Vector3& position, float red, float green, float blue, float alpha)
	{
		m_position = position;
		m_color[0] = red;
		m_color[1] = green;
		m_color[2] = blue;
		m_color[3] = alpha;
	}

	Vector3 m_position;
	float m_color[4];
};

void InitPipeline();
bool D3D11Setup()
{
	// Creation Flags
	// For options, see;
	// https://www.google.com/webhp?sourceid=chrome-instant&ion=1&espv=2&ie=UTF-8#safe=off&q=device_flags+%7C%3D+D3D11_CREATE_DEVICE_DEBUG%3B
	unsigned int device_flags = 0U;

	HWND hwnd = (HWND)Window::GetInstance()->GetWindowContext();
	// Setup our Swap Chain
	// For options, see;
	// https://www.google.com/webhp?sourceid=chrome-instant&ion=1&espv=2&ie=UTF-8#safe=off&q=DXGI_SWAP_CHAIN_DESC

	DXGI_SWAP_CHAIN_DESC swap_desc;
	memset(&swap_desc, 0, sizeof(swap_desc));

	// fill the swap chain description struct
	swap_desc.BufferCount = 1;                                    // number of back buffers

	swap_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
	swap_desc.OutputWindow = hwnd;                                // the window to be copied to on present
	swap_desc.SampleDesc.Count = 1;                               // how many multisamples (1 means no multi sampling)
	swap_desc.BufferDesc.Width = Window::GetInstance()->GetClientPixelWidth();
	swap_desc.BufferDesc.Height = Window::GetInstance()->GetClientPixelHeight();

	// Default options.
	swap_desc.Windowed = TRUE;                                    // windowed/full-screen mode
	swap_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
	swap_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	//swap_desc.BufferDesc.Width = WINDOW_RES_X;
	//swap_desc.BufferDesc.Height = WINDOW_RES_Y;


	// Actually Create
	HRESULT hr = ::D3D11CreateDeviceAndSwapChain(nullptr, // Adapter, if nullptr, will use adapter window is primarily on.
		D3D_DRIVER_TYPE_HARDWARE,  // Driver Type - We want to use the GPU (HARDWARE)
		nullptr,                   // Software Module - DLL that implements software mode (we do not use)
		device_flags,              // device creation options
		nullptr,                   // feature level (use default)
		0U,                        // number of feature levels to attempt
		D3D11_SDK_VERSION,         // SDK Version to use
		&swap_desc,                // Description of our swap chain
		&gD3DSwapChain,            // Swap Chain we're creating
		&gD3DDevice,               // [out] The device created
		nullptr,                   // [out] Feature Level Acquired
		&gD3DContext);            // Context that can issue commands on this pipe.

	 // SUCCEEDED & FAILED are macros provided by Windows to checking
	 // the results.  Almost every D3D call will return one - be sure to check it.



		// get the address of the back buffer
	ID3D11Texture2D *pBackBuffer;
	gD3DSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

	// use the back buffer address to create the render target
	gD3DDevice->CreateRenderTargetView(pBackBuffer, NULL, &backbuffer);
	pBackBuffer->Release();

	// set the render target as the back buffer
	gD3DContext->OMSetRenderTargets(1, &backbuffer, NULL);

	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)Window::GetInstance()->GetClientPixelWidth();
	viewport.Height = (float)Window::GetInstance()->GetClientPixelHeight();

	gD3DContext->RSSetViewports(1, &viewport);

	InitPipeline();

	return SUCCEEDED(hr);
}
#include "Engine/Math/MathUtils.h"
void D3D11Cleanup()
{
	gVertexShader->Release();
	gVertexShader = nullptr;

	gPixelShader->Release();
	gPixelShader = nullptr;

	gD3DSwapChain->SetFullscreenState(FALSE, NULL);

	backbuffer->Release();
	backbuffer = nullptr;

	gD3DSwapChain->Release();
	gD3DSwapChain = nullptr;

	gD3DContext->Release();
	gD3DContext = nullptr;

	gD3DDevice->Release();
	gD3DDevice = nullptr;
}

void RenderFrame()
{
	// clear the back buffer to a deep blue
	static float test = 0.f;
	test += 0.0001f;
	test = ModFloat(test, 1.0f);
	float color[4] = { 0.0f, test, Clamp(test, 0.f, 1.0f), Clamp(2.f * test, 0.f, 1.0f) };
	gD3DContext->ClearRenderTargetView(backbuffer, color);

	// do 3D rendering on the back buffer here
	gD3DContext->Draw(3, 0);

	// switch the back buffer and the front buffer
	gD3DSwapChain->Present(0, 0);
}

#include "Engine/Framework/File.h"
void InitPipeline()
{
	size_t shaderSourceSize;
	void* shaderSource = FileReadToNewBuffer("Data/Shader/test.shader", shaderSourceSize);
	DWORD compile_flags = 0U;
	compile_flags |= D3DCOMPILE_DEBUG;
	compile_flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
	compile_flags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;

	ID3DBlob *compiledVsSource = nullptr;
	ID3DBlob *vsCompileErrors = nullptr;

	::D3DCompile(shaderSource, shaderSourceSize,        // src data
		nullptr,                       // optional, used for error messages
		nullptr,                            // pre-compiler defines - used more for compiling multiple versions of a single shader (different quality specs, or shaders that are mostly the same outside some constants)
		D3D_COMPILE_STANDARD_FILE_INCLUDE,  // include rules - this allows #includes in the shader to work relative to the src_file path or my current working directly
		"VertexFunction",                         // Entry Point for this shader
		"vs_5_0",                             // Compile Target (MSDN - "Specifying Compiler Targets")
		compile_flags,                      // Flags that control compilation
		0,                                  // Effect Flags (we will not be doing Effect Files)
		&compiledVsSource,
		&vsCompileErrors);


	gD3DDevice->CreateVertexShader(compiledVsSource->GetBufferPointer(), compiledVsSource->GetBufferSize(), nullptr, &gVertexShader);

	ID3DBlob *compiledPsSource = nullptr;
	ID3DBlob *psCompileErrors = nullptr;

	::D3DCompile(shaderSource, shaderSourceSize,        // src data
		nullptr,                       // optional, used for error messages
		nullptr,                            // pre-compiler defines - used more for compiling multiple versions of a single shader (different quality specs, or shaders that are mostly the same outside some constants)
		D3D_COMPILE_STANDARD_FILE_INCLUDE,  // include rules - this allows #includes in the shader to work relative to the src_file path or my current working directly
		"PixelFunction",                         // Entry Point for this shader
		"ps_5_0",                             // Compile Target (MSDN - "Specifying Compiler Targets")
		compile_flags,                      // Flags that control compilation
		0,                                  // Effect Flags (we will not be doing Effect Files)
		&compiledPsSource,
		&psCompileErrors);

	
	gD3DDevice->CreatePixelShader(compiledPsSource->GetBufferPointer(), compiledPsSource->GetBufferSize(), nullptr, &gPixelShader);

	gD3DContext->VSSetShader(gVertexShader, 0, 0);
	gD3DContext->PSSetShader(gPixelShader, 0, 0);

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
	bd.ByteWidth = sizeof(VertexPC) * 3;             // size is the VERTEX struct * 3
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer

	gD3DDevice->CreateBuffer(&bd, NULL, &gVertexBuffer);       // create the buffer

	VertexPC triangleVerts[] =
	{
		VertexPC(Vector3(0.0f, 0.5f, 0.0f), 1.0f, 0.f, 0.f, 1.0f),
		VertexPC(Vector3(0.45f, -0.5, 0.0f), 0.0f, 1.f, 0.f, 1.0f),
		VertexPC(Vector3(-0.45f, -0.5f, 0.0f), 0.0f, 0.f, 1.f, 1.0f)
	};

	D3D11_MAPPED_SUBRESOURCE ms;
	gD3DContext->Map(gVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);   // map the buffer
	memcpy(ms.pData, triangleVerts, sizeof(triangleVerts));                // copy the data
	gD3DContext->Unmap(gVertexBuffer, NULL);                                     // unmap the buffer

	D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	gD3DDevice->CreateInputLayout(ied, 2, compiledVsSource->GetBufferPointer(), compiledVsSource->GetBufferSize(), &gInputLayout);
	gD3DContext->IASetInputLayout(gInputLayout);

	UINT stride = sizeof(VertexPC);
	UINT offset = 0;
	gD3DContext->IASetVertexBuffers(0, 1, &gVertexBuffer, &stride, &offset);

	gD3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}