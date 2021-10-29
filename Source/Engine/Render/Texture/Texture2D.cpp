///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 14th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Render/DX11Common.h"
#include "Engine/Render/RenderContext.h"
#include "Engine/Render/View/ShaderResourceView.h"
#include "Engine/Render/Texture/Texture2D.h"
#include "Engine/Resource/ResourceSystem.h"
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

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
bool Texture2D::Load(const char* filepath, TextureUsageBits textureUsage, GPUMemoryUsage memoryUsage)
{
	Image* image = g_resourceSystem->CreateOrGetImage(filepath);
	if (image != nullptr)
	{
		m_srcFilepath = filepath;
		return CreateFromImage(*image, TEXTURE_FORMAT_R8G8B8A8_UNORM, textureUsage, memoryUsage);
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
bool Texture2D::CreateFromImage(const Image& image, TextureFormat format, TextureUsageBits textureUsage, GPUMemoryUsage memoryUsage)
{
	return CreateFromBuffer(image.GetData(), image.GetSize(), image.GetTexelWidth(), image.GetTexelHeight(), format, textureUsage, memoryUsage);
}


//-------------------------------------------------------------------------------------------------
bool Texture2D::CreateWithNoData(int width, int height, TextureFormat format, TextureUsageBits textureUsage, GPUMemoryUsage memoryUsage)
{
	return CreateFromBuffer(nullptr, 0U, width, height, format, textureUsage, memoryUsage);
}


//-------------------------------------------------------------------------------------------------
bool Texture2D::CreateFromBuffer(const uint8* buffer, uint32 bufferSize, int width, int height, TextureFormat format, TextureUsageBits textureUsage, GPUMemoryUsage memoryUsage)
{
	// Safety checks
	bool isDepthStencil = (textureUsage & TEXTURE_USAGE_DEPTH_STENCIL_TARGET_BIT);
	bool isRenderTarget = (textureUsage & TEXTURE_USAGE_RENDER_TARGET_BIT);
	
	if (isDepthStencil || isRenderTarget)
	{
		ASSERT_OR_DIE(memoryUsage == GPU_MEMORY_USAGE_GPU, "Color targets and depth stencils need to be GPU_MEMORY_USAGE_GPU!");
	}

	if (isDepthStencil)
	{
		ASSERT_OR_DIE(buffer == nullptr, "No buffering data to depth stencils!");
	}
	else
	{
	}

	if (memoryUsage == GPU_MEMORY_USAGE_STAGING)
	{
		ASSERT_OR_DIE(textureUsage == TEXTURE_USAGE_NO_BIND, "Staging textures must have no bind usage only!");
	}
	else
	{
		ASSERT_OR_DIE((textureUsage & TEXTURE_USAGE_NO_BIND) == 0, "Texture needs to have a valid usage/bind!");
	}

	ASSERT_OR_DIE(width > 0 && height > 0, "Bad texture dimensions!");
	ASSERT_OR_DIE(!(buffer != nullptr && bufferSize == 0U), "Specified a buffer but no size to copy!");
	ASSERT_OR_DIE(!(buffer == nullptr && bufferSize > 0U), "Specified a size but buffer was null!");

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
	texDesc.Format = static_cast<DXGI_FORMAT>(GetDxFormatFromTextureFormat(format));
	texDesc.BindFlags = GetDxBindFromTextureUsageFlags(m_textureUsage);
	texDesc.MiscFlags = 0U;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;

	if (m_memoryUsage == GPU_MEMORY_USAGE_DYNAMIC)
	{
		texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else if (m_memoryUsage == GPU_MEMORY_USAGE_STAGING)
	{
		texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	}
	else
	{
		texDesc.CPUAccessFlags = 0U;
	}

	D3D11_SUBRESOURCE_DATA* initialData = nullptr;
	D3D11_SUBRESOURCE_DATA data;

	if (buffer != nullptr)
	{
		memset(&data, 0, sizeof(D3D11_SUBRESOURCE_DATA));

		uint32 pitch = width * 4; // Assumes component size == 1 byte
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
		m_format = format;
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
	m_byteSize = m_dimensions.x * m_dimensions.y * GetComponentCountFromTextureFormat(TEXTURE_FORMAT_R8G8B8A8_UNORM);
	m_memoryUsage = FromDXMemoryUsage(desc.Usage);
	m_textureUsage = GetTextureUsageFlagsFromDxBinds(desc.BindFlags);
	m_format = TEXTURE_FORMAT_INVALID;

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
// Creates an SRV for this texture, defaulting to reasonable defaults if no info is provided
ShaderResourceView* Texture2D::CreateOrGetShaderResourceView(const TextureViewCreateInfo* viewInfo /* = nullptr */)
{
	if (viewInfo == nullptr)
	{
		// Make sure we use the right default
		TextureViewCreateInfo cubeViewInfo;
		cubeViewInfo.m_viewDimension = VIEW_DIMENSION_TEXTURE2D;
		cubeViewInfo.m_viewUsage = TEXTURE_USAGE_SHADER_RESOURCE_BIT;

		return Texture::CreateOrGetShaderResourceView(&cubeViewInfo);
	}

	return Texture::CreateOrGetShaderResourceView(viewInfo);
}


//-------------------------------------------------------------------------------------------------
// Creates a render target view for this texture, using reasonable defaults
RenderTargetView* Texture2D::CreateOrGetColorTargetView(const TextureViewCreateInfo* viewInfo /*= nullptr*/)
{
	if (viewInfo == nullptr)
	{
		// Make sure we use the right default
		TextureViewCreateInfo cubeViewInfo;
		cubeViewInfo.m_viewDimension = VIEW_DIMENSION_TEXTURE2D;
		cubeViewInfo.m_viewUsage = TEXTURE_USAGE_RENDER_TARGET_BIT;

		return Texture::CreateOrGetColorTargetView(&cubeViewInfo);
	}

	return Texture::CreateOrGetColorTargetView(viewInfo);
}


//-------------------------------------------------------------------------------------------------
// Creates a depth stencil view for this texture, using reasonable defaults
DepthStencilView* Texture2D::CreateOrGetDepthStencilTargetView(const TextureViewCreateInfo* viewInfo /*= nullptr*/)
{
	if (viewInfo == nullptr)
	{
		// Make sure we use the right default
		TextureViewCreateInfo cubeViewInfo;
		cubeViewInfo.m_viewDimension = VIEW_DIMENSION_TEXTURE2D;
		cubeViewInfo.m_viewUsage = TEXTURE_USAGE_DEPTH_STENCIL_TARGET_BIT;

		return Texture::CreateOrGetDepthStencilTargetView(&cubeViewInfo);
	}

	return Texture::CreateOrGetDepthStencilTargetView(viewInfo);
}


//-------------------------------------------------------------------------------------------------
float Texture2D::GetAspect() const
{
	return static_cast<float>(m_dimensions.x) / static_cast<float>(m_dimensions.y);
}
