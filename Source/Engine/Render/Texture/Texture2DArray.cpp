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
	ASSERT_OR_DIE(m_dxHandle != nullptr, "Attempted to create a view for an uninitialized Texture!");
	ASSERT_OR_DIE((m_textureUsage & TEXTURE_USAGE_SHADER_RESOURCE_BIT) != 0, "Attempted to create a ShaderResourceView for a texture that doesn't support it!");

	// Default the info
	TextureViewCreateInfo defaultInfo;
	defaultInfo.m_viewType = TEXTURE_USAGE_SHADER_RESOURCE_BIT;
	defaultInfo.m_viewDimension = VIEW_DIMENSION_TEXTURE2DARRAY;

	if (viewInfo == nullptr)
	{
		viewInfo = &defaultInfo;
	}

	TextureView* view = GetView(viewInfo);
	if (view != nullptr)
	{
		ShaderResourceView* viewAsSRV = dynamic_cast<ShaderResourceView*>(view);
		ASSERT_OR_DIE(viewAsSRV != nullptr, "Couldn't cast view into ShaderResourceView!");

		return viewAsSRV;
	}
	else
	{
		// Create a ShaderResourceView for this texture
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

		DXGI_FORMAT dxFormat;
		if (m_format == TEXTURE_FORMAT_R24G8_TYPELESS)
		{
			dxFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		}
		else
		{
			dxFormat = static_cast<DXGI_FORMAT>(GetDxFormatFromTextureFormat(m_format));
		}

		srvDesc.Format = dxFormat;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Texture2DArray.MostDetailedMip = 0;
		srvDesc.Texture2DArray.MipLevels = viewInfo->m_numMipLevels;
		srvDesc.Texture2DArray.FirstArraySlice = 0;
		srvDesc.Texture2DArray.ArraySize = m_numTextures;

		ID3D11Device* dxDevice = g_renderContext->GetDxDevice();
		ID3D11ShaderResourceView* dxSRV = nullptr;
		HRESULT hr = dxDevice->CreateShaderResourceView(m_dxHandle, &srvDesc, &dxSRV);
		ASSERT_OR_DIE(SUCCEEDED(hr), "Couldn't create ShaderResourceView!");

		if (dxSRV != nullptr)
		{
			ShaderResourceView* shaderResourceView = new ShaderResourceView();

			shaderResourceView->m_dxSRV = dxSRV;
			shaderResourceView->m_sourceTexture = this;
			shaderResourceView->m_byteSize = m_byteSize;
			shaderResourceView->m_usage = TEXTURE_USAGE_SHADER_RESOURCE_BIT;
			shaderResourceView->m_createInfo = *viewInfo;
			uint32 infoHash = HashData((void*)viewInfo, sizeof(viewInfo));
			shaderResourceView->m_createInfoHash = infoHash;
			DX_SET_DEBUG_NAME(dxSRV, Stringf("ShaderResourceView | Source Texture Filepath: %s | Texture Dimensions: (%i, %i)", m_srcFilepath.c_str(), m_dimensions.x, m_dimensions.y));

			// Add it to the map
			m_views.push_back(shaderResourceView);

			return shaderResourceView;
		}

		return nullptr;
	}
}
