///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: October 16th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/IO/Image.h"
#include "Engine/Resource/ResourceSystem.h"
#include "Engine/Render/DX11Common.h"
#include "Engine/Render/RenderContext.h"
#include "Engine/Render/Texture/TextureCube.h"
#include "Engine/Render/View/ShaderResourceView.h"

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
// Assumes the files are labeled as expected below
bool TextureCube::LoadSixFiles(const char* folderPath)
{
	std::vector<Image*> images;
	images.reserve(6);

	for (int i = 0; i < 6; ++i)
	{
		std::string filepath = folderPath;
		filepath += Stringf("%i.png", i);
		Image* image = g_resourceSystem->CreateOrGetImage(filepath.c_str());

		ASSERT_RETURN(image != nullptr, false, "Couldn't find cube texture image %s!", filepath.c_str());
		images.push_back(image);
	}

	return CreateFromSixImages(images, TEXTURE_FORMAT_R8G8B8A8_UNORM, TEXTURE_USAGE_SHADER_RESOURCE_BIT, GPU_MEMORY_USAGE_GPU);
}


//-------------------------------------------------------------------------------------------------
bool TextureCube::CreateFromSixImages(const std::vector<Image*>& sixImages, TextureFormat format, TextureUsageBits textureUsage, GPUMemoryUsage memoryUsage)
{
	uint8* buffers[6];

	for (int i = 0; i < 6; ++i)
	{
		buffers[i] = sixImages[i]->GetData();
	}

	return CreateFromSixBuffers(buffers, sixImages[0]->GetSize(), sixImages[0]->GetTexelWidth(), sixImages[0]->GetTexelHeight(), format, textureUsage, memoryUsage);
}


//-------------------------------------------------------------------------------------------------
bool TextureCube::CreateFromSixBuffers(uint8* buffers[6], uint32 eachBufferSize, int width, int height, TextureFormat format, TextureUsageBits textureUsage, GPUMemoryUsage memoryUsage)
{
	Clear();

	ID3D11Device* dxDevice = g_renderContext->GetDxDevice();

	D3D11_TEXTURE2D_DESC texDesc;
	memset(&texDesc, 0, sizeof(D3D11_TEXTURE2D_DESC));

	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1; // Set to 0 for full chain
	texDesc.ArraySize = 6; // 6 images
	texDesc.Usage = (D3D11_USAGE)ToDXMemoryUsage(memoryUsage);
	texDesc.Format = static_cast<DXGI_FORMAT>(GetDxFormatFromTextureFormat(format));
	texDesc.BindFlags = GetDxBindFromTextureUsageFlags(textureUsage);
	texDesc.MiscFlags = 0U;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	texDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA data[6];
	D3D11_SUBRESOURCE_DATA* dataParam = nullptr;

	if (buffers != nullptr)
	{
		for (int i = 0; i < 6; ++i)
		{
			data[i].pSysMem = buffers[i];
			data[i].SysMemPitch = width * 4; // Hardcoding it to 4
			data[i].SysMemSlicePitch = 0;
		}

		dataParam = &data[0];
	}


	ID3D11Texture2D* tex2D = nullptr;
	HRESULT hr = dxDevice->CreateTexture2D(&texDesc, dataParam, &tex2D);

	bool succeeded = SUCCEEDED(hr);
	ASSERT_RECOVERABLE(succeeded, "Couldn't create Texture2D!");

	if (succeeded)
	{
		m_dxHandle = tex2D;
		m_dimensions = IntVector3(width, height, 0);
		m_byteSize = 6 * eachBufferSize;
		m_format = format;
		m_memoryUsage = memoryUsage;
		m_textureUsage = textureUsage;

		DX_SET_DEBUG_NAME(m_dxHandle, Stringf("Source File: %s | Size: (%i, %i)", m_srcFilepath.c_str(), width, height));
	}

	return succeeded;
}


//-------------------------------------------------------------------------------------------------
bool TextureCube::CreateWithNoData(int width, int height, TextureFormat format, TextureUsageBits textureUsage, GPUMemoryUsage memoryUsage)
{
	return CreateFromSixBuffers(nullptr, 0U, width, height, format, textureUsage, memoryUsage);
}


//-------------------------------------------------------------------------------------------------
ShaderResourceView* TextureCube::CreateOrGetShaderResourceView(const TextureViewCreateInfo* viewInfo /*= nullptr*/)
{
	if (viewInfo == nullptr)
	{
		// Make sure we use the right default
		TextureViewCreateInfo cubeViewInfo;
		cubeViewInfo.m_viewDimension = VIEW_DIMENSION_TEXTURECUBE;
		cubeViewInfo.m_viewUsage = TEXTURE_USAGE_SHADER_RESOURCE_BIT;

		return Texture::CreateOrGetShaderResourceView(&cubeViewInfo);
	}

	return Texture::CreateOrGetShaderResourceView(viewInfo);
}


//-------------------------------------------------------------------------------------------------
RenderTargetView* TextureCube::CreateOrGetColorTargetView(const TextureViewCreateInfo* viewInfo /*= nullptr*/)
{
	if (viewInfo == nullptr)
	{
		// Make sure we use the right default
		TextureViewCreateInfo cubeViewInfo;
		cubeViewInfo.m_viewDimension = VIEW_DIMENSION_TEXTURECUBE;
		cubeViewInfo.m_viewUsage = TEXTURE_USAGE_RENDER_TARGET_BIT;

		return Texture::CreateOrGetColorTargetView(&cubeViewInfo);
	}

	return Texture::CreateOrGetColorTargetView(viewInfo);
}


//-------------------------------------------------------------------------------------------------
DepthStencilView* TextureCube::CreateOrGetDepthStencilView(const TextureViewCreateInfo* viewInfo /*= nullptr*/)
{
	if (viewInfo == nullptr)
	{
		// Make sure we use the right default
		TextureViewCreateInfo cubeViewInfo;
		cubeViewInfo.m_viewDimension = VIEW_DIMENSION_TEXTURECUBE;
		cubeViewInfo.m_viewUsage = TEXTURE_USAGE_DEPTH_STENCIL_BIT;

		return Texture::CreateOrGetDepthStencilView(&cubeViewInfo);
	}

	return Texture::CreateOrGetDepthStencilView(viewInfo);
}
