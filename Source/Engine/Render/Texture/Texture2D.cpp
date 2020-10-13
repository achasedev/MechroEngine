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
#include "Engine/Render/View/ShaderResourceView.h"
#include "Engine/Render/View/ShaderResourceView.h"
#include "Engine/Render/Texture/Texture2D.h"
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
	if (usage & TEXTURE_USAGE_SHADER_RESOURCE_BIT) 
	{
		binds |= D3D11_BIND_SHADER_RESOURCE;
	}

	// Can I render to it?
	if (usage & TEXTURE_USAGE_RENDER_TARGET_BIT) 
	{
		binds |= D3D11_BIND_RENDER_TARGET;
	}

	// Can I store depth info in it?
	if (usage & TEXTURE_USAGE_DEPTH_STENCIL_TARGET_BIT) 
	{
		binds |= D3D11_BIND_DEPTH_STENCIL;
	}

	return binds;
}


//-------------------------------------------------------------------------------------------------
static TextureUsageBits GetTextureUsageFlagsFromDxBinds(uint32 dxBind)
{
	TextureUsageBits usageFlags = 0;

	// Can I sample from it?
	if (dxBind & D3D11_BIND_SHADER_RESOURCE)
	{
		usageFlags |= D3D11_BIND_SHADER_RESOURCE;
	}

	// Can I render to it?
	if (dxBind & D3D11_BIND_RENDER_TARGET)
	{
		usageFlags |= TEXTURE_USAGE_RENDER_TARGET_BIT;
	}

	// Can I store depth info in it?
	if (dxBind & D3D11_BIND_DEPTH_STENCIL)
	{
		usageFlags |= TEXTURE_USAGE_DEPTH_STENCIL_TARGET_BIT;
	}

	return usageFlags;
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


//-------------------------------------------------------------------------------------------------
static int GetComponentCountFromDxTextureFormat(DXGI_FORMAT dxFormat)
{
	switch (dxFormat)
	{
	case DXGI_FORMAT_R8_UNORM: return 1; break;
	case DXGI_FORMAT_R8G8_UNORM: return 2; break;
	case DXGI_FORMAT_R8G8B8A8_UNORM: return 4; break;
	default:
		ERROR_AND_DIE("Missing DXGI_FORMAT: %i", (int)dxFormat);
		break;
	}
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
bool Texture2D::CreateFromFile(const char* filepath, TextureUsageBits textureUsage, GPUMemoryUsage memoryUsage)
{
	Image image;
	if (image.LoadFromFile(filepath))
	{
		m_srcFilepath = filepath;
		return CreateFromImage(image, textureUsage, memoryUsage);
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
bool Texture2D::CreateFromImage(const Image& image, TextureUsageBits textureUsage, GPUMemoryUsage memoryUsage)
{
	return CreateFromBuffer(image.GetData(), image.GetSize(), image.GetTexelWidth(), image.GetTexelHeight(), image.GetNumComponentsPerTexel(), textureUsage, memoryUsage);
}


//-------------------------------------------------------------------------------------------------
bool Texture2D::CreateFromBuffer(const uint8* buffer, uint32 bufferSize, int width, int height, uint32 numComponents, TextureUsageBits textureUsage, GPUMemoryUsage memoryUsage)
{
	Clear();

	ID3D11Device* dxDevice = g_renderContext->GetDxDevice();

	m_textureUsage = textureUsage;
	m_memoryUsage = memoryUsage;

	D3D11_TEXTURE2D_DESC texDesc;
	memset(&texDesc, 0, sizeof(D3D11_TEXTURE2D_DESC));

	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1; // Set to 0 for full chain
	texDesc.ArraySize = 1;
	texDesc.Usage = (D3D11_USAGE)ToDXMemoryUsage(m_memoryUsage);
	texDesc.Format = GetDxTextureFormatFromComponentCount(numComponents);
	texDesc.BindFlags = GetDxBindFromTextureUsageFlags(m_textureUsage);
	texDesc.CPUAccessFlags = (m_memoryUsage == GPU_MEMORY_USAGE_DYNAMIC ? D3D11_CPU_ACCESS_WRITE : 0U);
	texDesc.MiscFlags = 0U;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;

	D3D11_SUBRESOURCE_DATA* initialData = nullptr;
	D3D11_SUBRESOURCE_DATA data;

	if (buffer != nullptr)
	{
		memset(&data, 0, sizeof(D3D11_SUBRESOURCE_DATA));

		uint32 pitch = width * numComponents; // Assumes component size == 1 byte
		data.SysMemPitch = pitch;
		data.pSysMem = buffer;

		initialData = &data;
	}
	
	ID3D11Texture2D* tex2D = nullptr;
	HRESULT hr = dxDevice->CreateTexture2D(&texDesc, initialData, &tex2D);

	bool succeeded = SUCCEEDED(hr);
	ASSERT_RECOVERABLE(succeeded, "Couldn't create Texture2D!");

	if (succeeded)
	{
		m_dxHandle = tex2D;
		m_dimensions = IntVector3(width, height, 0);
		m_byteSize = bufferSize;
		DX_SET_DEBUG_NAME(m_dxHandle, Stringf("Source File: %s | Size: (%i, %i)", m_srcFilepath.c_str(), width, height));
	}

	return succeeded;
}


//-------------------------------------------------------------------------------------------------
bool Texture2D::CreateFromDxTexture2D(ID3D11Texture2D* dxTexture2D)
{
	Clear();

	D3D11_TEXTURE2D_DESC desc;
	dxTexture2D->GetDesc(&desc);

	m_dxHandle = dxTexture2D;
	m_dxHandle->AddRef();

	m_dimensions = IntVector3(desc.Width, desc.Height, 0U);
	m_byteSize = m_dimensions.x * m_dimensions.y * GetComponentCountFromDxTextureFormat(desc.Format);
	m_memoryUsage = FromDXMemoryUsage(desc.Usage);
	m_textureUsage = GetTextureUsageFlagsFromDxBinds(desc.BindFlags);

	return true;
}


//-------------------------------------------------------------------------------------------------
bool Texture2D::UpdateFromImage(const Image& image)
{
	ASSERT_OR_DIE(m_memoryUsage == GPU_MEMORY_USAGE_GPU, "Texture must only have GPU read/write access!");

	IntVector2 textureDimensions = m_dimensions.xy;
	ASSERT_OR_DIE(textureDimensions == image.GetDimensions(), "Cannot update texture with image of different size!");

	g_renderContext->GetDxContext()->UpdateSubresource(m_dxHandle, 0, NULL, image.GetData(), (UINT)(image.GetNumComponentsPerTexel() * image.GetTexelWidth()), (UINT)(image.GetNumComponentsPerTexel() * image.GetTexelWidth() * image.GetTexelHeight()));

	return false;
}


//-------------------------------------------------------------------------------------------------
bool Texture2D::CreateAsColorRenderTarget(uint32 width, uint32 height, bool createAsShaderResource /*= false*/)
{
	Clear();

	ID3D11Device* dxDevice = g_renderContext->GetDxDevice();

	m_textureUsage = TEXTURE_USAGE_RENDER_TARGET_BIT;

	if (createAsShaderResource)
	{
		m_textureUsage |= TEXTURE_USAGE_SHADER_RESOURCE_BIT;
	}

	m_memoryUsage = GPU_MEMORY_USAGE_GPU;

	D3D11_TEXTURE2D_DESC texDesc;
	memset(&texDesc, 0, sizeof(D3D11_TEXTURE2D_DESC));

	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Usage = (D3D11_USAGE)ToDXMemoryUsage(m_memoryUsage);
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
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
		m_dimensions = IntVector3((int)width, (int)height, 0);
		m_byteSize = width * height * 4U;
	}

	return succeeded;
}


//-------------------------------------------------------------------------------------------------
bool Texture2D::CreateAsDepthStencilTarget(uint32 width, uint32 height)
{
	Clear();

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
		m_dimensions = IntVector3((int)width, (int)height, 0);
		m_byteSize = width * height * 4U;
	}

	return succeeded;
}


//-------------------------------------------------------------------------------------------------
float Texture2D::GetAspect() const
{
	return static_cast<float>(m_dimensions.x) / static_cast<float>(m_dimensions.y);
}
