///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 14th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/DirectX/DX11Common.h"
#include "Engine/DirectX/RenderContext.h"
#include "Engine/DirectX/Texture2D.h"
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
bool Texture2D::LoadFromFile(const char* filepath)
{
	Image image;
	if (image.LoadFromFile(filepath))
	{
		return LoadFromImage(image);
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
bool Texture2D::LoadFromImage(const Image& image)
{
	DX_SAFE_RELEASE(m_handle);

	RenderContext* context = RenderContext::GetInstance();
	ID3D11Device* dxDevice = context->GetDxDevice();

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
		m_handle = tex2D;
	}

	return succeeded;
}
