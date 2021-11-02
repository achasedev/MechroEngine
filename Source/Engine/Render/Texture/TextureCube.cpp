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

	return CreateFromSixImages(images);
}


//-------------------------------------------------------------------------------------------------
bool TextureCube::CreateFromSixImages(const std::vector<Image*>& sixImages)
{
	Clear();

	// All 6 images should have the same dimensions
	int width = sixImages[0]->GetTexelWidth();
	int height = sixImages[0]->GetTexelHeight();
	int numComponents = sixImages[0]->GetNumComponentsPerTexel();

	ID3D11Device* dxDevice = g_renderContext->GetDxDevice();

	m_format = TEXTURE_FORMAT_R8G8B8A8_UNORM;
	m_textureUsage = TEXTURE_USAGE_SHADER_RESOURCE_BIT;
	m_memoryUsage = GPU_MEMORY_USAGE_GPU;

	D3D11_TEXTURE2D_DESC texDesc;
	memset(&texDesc, 0, sizeof(D3D11_TEXTURE2D_DESC));

	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1; // Set to 0 for full chain
	texDesc.ArraySize = 6; // 6 images
	texDesc.Usage = (D3D11_USAGE)ToDXMemoryUsage(m_memoryUsage);
	texDesc.Format = static_cast<DXGI_FORMAT>(GetDxFormatFromTextureFormat(m_format));
	texDesc.BindFlags = GetDxBindFromTextureUsageFlags(m_textureUsage);
	texDesc.MiscFlags = 0U;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	texDesc.CPUAccessFlags = 0;
	
	D3D11_SUBRESOURCE_DATA data[6];
	for (int i = 0; i < 6; ++i)
	{
		data[i].pSysMem = sixImages[i]->GetData();
		data[i].SysMemPitch = width * numComponents;
		data[i].SysMemSlicePitch = 0;
	}

	ID3D11Texture2D* tex2D = nullptr;
	HRESULT hr = dxDevice->CreateTexture2D(&texDesc, data, &tex2D);

	bool succeeded = SUCCEEDED(hr);
	ASSERT_RECOVERABLE(succeeded, "Couldn't create Texture2D!");

	if (succeeded)
	{
		m_dxHandle = tex2D;
		m_dimensions = IntVector3(width, height, 0);
		m_byteSize = 0;

		for (int i = 0; i < 6; ++i)
		{
			m_byteSize += sixImages[i]->GetSize();
		}

		DX_SET_DEBUG_NAME(m_dxHandle, Stringf("Source File: %s | Size: (%i, %i)", m_srcFilepath.c_str(), width, height));
	}
	else
	{
		Clear();
	}

	return succeeded;
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
		cubeViewInfo.m_viewUsage = TEXTURE_USAGE_DEPTH_STENCIL_TARGET_BIT;

		return Texture::CreateOrGetDepthStencilView(&cubeViewInfo);
	}

	return Texture::CreateOrGetDepthStencilView(viewInfo);
}
