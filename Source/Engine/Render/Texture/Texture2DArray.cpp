///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: Oct 28th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/DevConsole.h"
#include "Engine/Core/EngineCommon.h"
#include "Engine/Render/RenderContext.h"
#include "Engine/Render/Texture/Texture2DArray.h"
#include "Engine/Render/View/ShaderResourceView.h"
#include "Engine/Utility/Hash.h"

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
// Destructor
Texture2DArray::~Texture2DArray()
{
}


//-------------------------------------------------------------------------------------------------
// Creates the array, allocating space for the given number of textures; takes no data
bool Texture2DArray::Create(uint32 numTextures, int width, int height, TextureFormat format)
{
	Clear();

	ID3D11Device* dxDevice = g_renderContext->GetDxDevice();

	m_textureUsage = TEXTURE_USAGE_SHADER_RESOURCE_BIT; // Only SRV for now
	m_memoryUsage = GPU_MEMORY_USAGE_GPU;
	m_format = format;

	D3D11_TEXTURE2D_DESC texDesc;
	memset(&texDesc, 0, sizeof(D3D11_TEXTURE2D_DESC));

	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1; // Set to 0 for full chain
	texDesc.ArraySize = numTextures;
	texDesc.Usage = (D3D11_USAGE)ToDXMemoryUsage(m_memoryUsage);
	texDesc.Format = static_cast<DXGI_FORMAT>(GetDxFormatFromTextureFormat(m_format));
	texDesc.BindFlags = GetDxBindFromTextureUsageFlags(m_textureUsage);
	texDesc.MiscFlags = 0U;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.CPUAccessFlags = 0U;

	ID3D11Texture2D* tex2D = nullptr;
	HRESULT hr = dxDevice->CreateTexture2D(&texDesc, nullptr, &tex2D);

	bool succeeded = SUCCEEDED(hr);
	ASSERT_RECOVERABLE(succeeded, "Couldn't create Texture2DArray!");

	if (succeeded)
	{
		m_dxHandle = tex2D;
		m_dimensions = IntVector3(width, height, 0);
		m_byteSize = numTextures * width * height * 4;
		m_numTextures = numTextures;
		DX_SET_DEBUG_NAME(m_dxHandle, Stringf("Source File: %s | Size: (%i, %i)", m_srcFilepath.c_str(), width, height));
	}
	else
	{
		Clear();
	}

	return succeeded;
}


//-------------------------------------------------------------------------------------------------
// Creates an SRV that's compatible with Texture2DArray in the shader
ShaderResourceView* Texture2DArray::CreateOrGetShaderResourceView(const TextureViewCreateInfo* viewInfo /* = nullptr */)
{
	if (viewInfo == nullptr)
	{
		// Make sure we use the right default
		TextureViewCreateInfo cubeViewInfo;
		cubeViewInfo.m_viewDimension = VIEW_DIMENSION_TEXTURE2DARRAY;
		cubeViewInfo.m_viewUsage = TEXTURE_USAGE_SHADER_RESOURCE_BIT;
		cubeViewInfo.m_firstTextureIndex = 0;
		cubeViewInfo.m_numTextures = m_numTextures;

		return Texture::CreateOrGetShaderResourceView(&cubeViewInfo);
	}

	return Texture::CreateOrGetShaderResourceView(viewInfo);
}


//-------------------------------------------------------------------------------------------------
// Creates a render target view for this texture, using reasonable defaults
RenderTargetView* Texture2DArray::CreateOrGetColorTargetView(const TextureViewCreateInfo* viewInfo /*= nullptr*/)
{
	if (viewInfo == nullptr)
	{
		// Make sure we use the right default
		TextureViewCreateInfo cubeViewInfo;
		cubeViewInfo.m_viewDimension = VIEW_DIMENSION_TEXTURE2DARRAY;
		cubeViewInfo.m_viewUsage = TEXTURE_USAGE_RENDER_TARGET_BIT;
		cubeViewInfo.m_firstTextureIndex = 0;
		cubeViewInfo.m_numTextures = m_numTextures;

		return Texture::CreateOrGetColorTargetView(&cubeViewInfo);
	}

	return Texture::CreateOrGetColorTargetView(viewInfo);
}


//-------------------------------------------------------------------------------------------------
// Creates a depth stencil view for this texture, using reasonable defaults
DepthStencilView* Texture2DArray::CreateOrGetDepthStencilView(const TextureViewCreateInfo* viewInfo /*= nullptr*/)
{
	if (viewInfo == nullptr)
	{
		// Make sure we use the right default
		TextureViewCreateInfo cubeViewInfo;
		cubeViewInfo.m_viewDimension = VIEW_DIMENSION_TEXTURE2DARRAY;
		cubeViewInfo.m_viewUsage = TEXTURE_USAGE_DEPTH_STENCIL_TARGET_BIT;
		cubeViewInfo.m_firstTextureIndex = 0;
		cubeViewInfo.m_numTextures = m_numTextures;

		return Texture::CreateOrGetDepthStencilView(&cubeViewInfo);
	}

	return Texture::CreateOrGetDepthStencilView(viewInfo);
}
