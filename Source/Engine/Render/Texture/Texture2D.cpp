///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 14th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Render/Core/DX11Common.h"
#include "Engine/Render/Core/RenderContext.h"
#include "Engine/Render/Texture/DepthStencilTargetView.h"
#include "Engine/Render/Texture/Texture2D.h"
#include "Engine/Render/Texture/TextureView2D.h"
#include "Engine/IO/Image.h"

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
static uint32 GetDxBindFromTextureUsageFlags(TextureUsageBits usage)
{
	uint32 binds = 0U;

	// Can I sample from it?
	if (usage & TEXTURE_USAGE_TEXTURE_BIT) 
	{
		binds |= D3D11_BIND_SHADER_RESOURCE;
	}

	// Can I use it as a color target?
	if (usage & TEXTURE_USAGE_COLOR_TARGET_BIT) 
	{
		binds |= D3D11_BIND_RENDER_TARGET;
	}

	// Can I use it as a depth stencil?
	if (usage & TEXTURE_USAGE_DEPTH_STENCIL_TARGET_BIT) 
	{
		binds |= D3D11_BIND_DEPTH_STENCIL;
	}

	return binds;
}


//-------------------------------------------------------------------------------------------------
static DXGI_FORMAT GetDxTextureFormatFromComponentCount(int numComponents)
{
	switch (numComponents)
	{
	case 1: return DXGI_FORMAT_R8_UNORM;break;
	case 2: return DXGI_FORMAT_R8G8_UNORM;break;
	case 4: return DXGI_FORMAT_R8G8B8A8_UNORM;break;
	default:
		ERROR_AND_DIE("Invalid number of components for texture: %i", numComponents);
		break;
	}
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
bool Texture2D::CreateFromFile(const char* filepath)
{
	Image image;
	if (image.LoadFromFile(filepath))
	{
		return CreateFromImage(image);
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
bool Texture2D::CreateFromImage(const Image& image)
{
	DX_SAFE_RELEASE(m_dxHandle);

	ID3D11Device* dxDevice = g_renderContext->GetDxDevice();

	m_textureUsage = TEXTURE_USAGE_TEXTURE_BIT; // Read only texture
	m_memoryUsage = GPU_MEMORY_USAGE_GPU; // Non-static for mip-maps

	D3D11_TEXTURE2D_DESC texDesc;
	memset(&texDesc, 0, sizeof(D3D11_TEXTURE2D_DESC));

	texDesc.Width = image.GetTexelWidth();
	texDesc.Height = image.GetTexelHeight();
	texDesc.MipLevels = 1; // Set to 0 for full chain
	texDesc.ArraySize = 1;
	texDesc.Usage = (D3D11_USAGE) ToDXMemoryUsage(m_memoryUsage);
	texDesc.Format = GetDxTextureFormatFromComponentCount(image.GetNumComponentsPerTexel());         
	texDesc.BindFlags = GetDxBindFromTextureUsageFlags(m_textureUsage);
	texDesc.CPUAccessFlags = 0U;
	texDesc.MiscFlags = 0U;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;

	
	D3D11_SUBRESOURCE_DATA data;
	memset(&data, 0, sizeof(D3D11_SUBRESOURCE_DATA));

	uint32 pitch = image.GetTexelWidth() * image.GetNumComponentsPerTexel(); // Assumes component size == 1 byte
	data.SysMemPitch = pitch;
	data.pSysMem = image.GetData();

	ID3D11Texture2D* tex2D = nullptr;
	HRESULT hr = dxDevice->CreateTexture2D(&texDesc, &data, &tex2D);

	bool succeeded = SUCCEEDED(hr);
	ASSERT_RECOVERABLE(succeeded, "Couldn't create Texture2D for image %s, error code: %u", image.GetFilePath(), hr);

	if (succeeded)
	{
		m_dxHandle = tex2D;
		m_dimensions = image.GetDimensions();
		m_size = image.GetSize();
	}

	return succeeded;
}


//-------------------------------------------------------------------------------------------------
bool Texture2D::CreateAsDepthStencil(uint32 width, uint32 height)
{
	DX_SAFE_RELEASE(m_dxHandle);

	ID3D11Device* dxDevice = g_renderContext->GetDxDevice();

	m_textureUsage = TEXTURE_USAGE_DEPTH_STENCIL_TARGET_BIT;
	m_memoryUsage = GPU_MEMORY_USAGE_GPU;

	D3D11_TEXTURE2D_DESC texDesc;
	memset(&texDesc, 0, sizeof(D3D11_TEXTURE2D_DESC));

	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Usage = (D3D11_USAGE)ToDXMemoryUsage(m_memoryUsage);
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.BindFlags = GetDxBindFromTextureUsageFlags(m_textureUsage);
	texDesc.CPUAccessFlags = 0U;
	texDesc.MiscFlags = 0U;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;

	ID3D11Texture2D* tex2D = nullptr;
	HRESULT hr = dxDevice->CreateTexture2D(&texDesc, nullptr, &tex2D);

	bool succeeded = SUCCEEDED(hr);
	ASSERT_RECOVERABLE(succeeded, "Couldn't create depth stencil Texture2D, error code: %u", hr);

	if (succeeded)
	{
		m_dxHandle = tex2D;
		m_dimensions = IntVector2((int)width, (int)height);
		m_size = width * height * 4U;
	}

	return succeeded;
}


//-------------------------------------------------------------------------------------------------
TextureView2D* Texture2D::CreateTextureView2D() const
{
	ASSERT_OR_DIE(m_dxHandle != nullptr, "Attempted to create a view for an uninitialized Texture!");

	ID3D11Device* dxDevice = g_renderContext->GetDxDevice();

	ID3D11ShaderResourceView* srv = nullptr;
	dxDevice->CreateShaderResourceView(m_dxHandle, nullptr, &srv);

	if (srv != nullptr)
	{
		TextureView2D* view2D = new TextureView2D();

		view2D->m_dxView = srv;
		view2D->m_dimensions = m_dimensions;

		m_dxHandle->AddRef();
		view2D->m_dxSource = m_dxHandle;
		view2D->m_size = m_size;

		return view2D;
	}

	return nullptr;
}


//-------------------------------------------------------------------------------------------------
DepthStencilTargetView* Texture2D::CreateDepthStencilTargetView() const
{
	ASSERT_OR_DIE(m_textureUsage & TEXTURE_USAGE_DEPTH_STENCIL_TARGET_BIT, "Attempting to make a depth stencil view for a non-depth stencil texture!");
	ASSERT_OR_DIE(m_dxHandle != nullptr, "Attempting to make a depth stencil view for uninitialized texture!");

	ID3D11Device* dxDevice = g_renderContext->GetDxDevice();
	ID3D11DepthStencilView* dxView = nullptr;

	D3D11_DEPTH_STENCIL_VIEW_DESC desc;
	memset(&desc, 0, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	dxDevice->CreateDepthStencilView(m_dxHandle, &desc, &dxView);

	if (dxView != nullptr)
	{
		DepthStencilTargetView* targetView = new DepthStencilTargetView();

		targetView->m_dxView = dxView;
		m_dxHandle->AddRef();
		targetView->m_dxSource = m_dxHandle;

		targetView->m_dimensions = m_dimensions;
		targetView->m_size = m_size;

		return targetView;
	}

	return nullptr;
}
