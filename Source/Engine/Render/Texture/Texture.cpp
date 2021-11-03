///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 14, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/DevConsole.h"
#include "Engine/Render/DX11Common.h"
#include "Engine/Render/RenderContext.h"
#include "Engine/Render/Texture/Texture.h"
#include "Engine/Render/View/RenderTargetView.h"
#include "Engine/Render/View/ShaderResourceView.h"
#include "Engine/Render/View/DepthStencilView.h"
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

//-------------------------------------------------------------------------------------------------
static D3D11_SRV_DIMENSION GetDxDimensionFromViewDimension(ViewDimension dimension)
{
	switch (dimension)
	{
	case VIEW_DIMENSION_TEXTURE2D: return D3D11_SRV_DIMENSION_TEXTURE2D; break;
	case VIEW_DIMENSION_TEXTURECUBE: return D3D11_SRV_DIMENSION_TEXTURECUBE; break;
	case VIEW_DIMENSION_TEXTURE2DARRAY: return D3D11_SRV_DIMENSION_TEXTURE2DARRAY; break;
	case VIEW_DIMENSION_TEXTURECUBEARRAY: return D3D11_SRV_DIMENSION_TEXTURECUBEARRAY; break;
	default:
		ERROR_AND_DIE("Invalid dimension!");
		break;
	}
}


//-------------------------------------------------------------------------------------------------
uint32 GetDxBindFromTextureUsageFlags(TextureUsageBits usage)
{
	if (usage & TEXTURE_USAGE_NO_BIND)
	{
		return 0U;
	}

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
	if (usage & TEXTURE_USAGE_DEPTH_STENCIL_BIT)
	{
		binds |= D3D11_BIND_DEPTH_STENCIL;
	}

	return binds;
}


//-------------------------------------------------------------------------------------------------
TextureUsageBits GetTextureUsageFlagsFromDxBinds(uint32 dxBind)
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
		usageFlags |= TEXTURE_USAGE_DEPTH_STENCIL_BIT;
	}

	return usageFlags;
}


//-------------------------------------------------------------------------------------------------
int GetComponentCountFromDxTextureFormat(uint32 dxFormat)
{
	DXGI_FORMAT dxFormatAsEnum = (DXGI_FORMAT)dxFormat;
	switch (dxFormatAsEnum)
	{
	case DXGI_FORMAT_R8_UNORM: return 1; break;
	case DXGI_FORMAT_R8G8_UNORM: return 2; break;
	case DXGI_FORMAT_R8G8B8A8_UNORM: return 4; break;
	default:
		ERROR_AND_DIE("Missing DXGI_FORMAT: %i", (int)dxFormat);
		break;
	}
}


//-------------------------------------------------------------------------------------------------
// Returns the number of separate components for the various internal formats
int GetComponentCountFromTextureFormat(TextureFormat format)
{
	switch (format)
	{
	case TEXTURE_FORMAT_R8G8B8A8_UNORM: { return 4; } break;
	case TEXTURE_FORMAT_R24G8_TYPELESS:	{ return 2; } break;
	default:
		ERROR_AND_DIE("Unsupported texture format!");
		break;
	}
}


//-------------------------------------------------------------------------------------------------
// Converts the DX enumeration to my internal enumeration
TextureFormat GetTextureFormatFromDxFormat(uint32 dxFormat)
{
	if		(dxFormat == DXGI_FORMAT_R8G8B8A8_UNORM) { return TEXTURE_FORMAT_R8G8B8A8_UNORM; }
	else if (dxFormat == DXGI_FORMAT_R24G8_TYPELESS) { return TEXTURE_FORMAT_R24G8_TYPELESS; }
	else
	{
		ERROR_AND_DIE("Unsupported texture format!");
	}
}


//-------------------------------------------------------------------------------------------------
// Returns the DX format represented by the given texture format
uint32 GetDxFormatFromTextureFormat(TextureFormat format)
{
	switch (format)
	{
	case TEXTURE_FORMAT_R8G8B8A8_UNORM: { return DXGI_FORMAT_R8G8B8A8_UNORM; } break;
	case TEXTURE_FORMAT_R24G8_TYPELESS:	{ return DXGI_FORMAT_R24G8_TYPELESS; } break;
	default:
		ERROR_AND_DIE("Unsupported texture format!");
		break;
	}
}

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
Texture::~Texture()
{
	Clear();
}


//-------------------------------------------------------------------------------------------------
ShaderResourceView* Texture::CreateOrGetShaderResourceView(const TextureViewCreateInfo* viewInfo /*= nullptr*/)
{
	if (viewInfo == nullptr)
	{
		ConsoleLogErrorf("Couldn't create SRV for texture %s, viewInfo was nullptr so it wasn't defaulted.", m_resourceID.ToString());
		return nullptr;
	}

	if (m_dxHandle == nullptr)
	{
		ConsoleLogErrorf("Couldn't create SRV for texture %s, texture wasn't created.", m_resourceID.ToString());
		return nullptr;
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

		// Set format
		// If the texture is a depth stencil, it needs to have the right view format to be sampled
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

		// Set view dimension and dimension-specific fields
		srvDesc.ViewDimension = GetDxDimensionFromViewDimension(viewInfo->m_viewDimension);

		switch (viewInfo->m_viewDimension)
		{
		case VIEW_DIMENSION_TEXTURE2D:
			srvDesc.Texture2D.MostDetailedMip = viewInfo->m_mostDetailedMip;
			srvDesc.Texture2D.MipLevels = viewInfo->m_numMipLevels;
			break;
		case VIEW_DIMENSION_TEXTURE2DARRAY:
			srvDesc.Texture2DArray.MostDetailedMip = viewInfo->m_mostDetailedMip;
			srvDesc.Texture2DArray.MipLevels = viewInfo->m_numMipLevels;
			srvDesc.Texture2DArray.FirstArraySlice = viewInfo->m_firstTextureIndex;
			srvDesc.Texture2DArray.ArraySize = viewInfo->m_numTextures;
			break;
		case VIEW_DIMENSION_TEXTURECUBE:
			srvDesc.TextureCube.MipLevels = viewInfo->m_numMipLevels;
			srvDesc.TextureCube.MostDetailedMip = viewInfo->m_mostDetailedMip;
			break;
		case VIEW_DIMENSION_TEXTURECUBEARRAY:
			srvDesc.TextureCubeArray.First2DArrayFace = viewInfo->m_firstTextureIndex;
			srvDesc.TextureCubeArray.MipLevels = viewInfo->m_numMipLevels;
			srvDesc.TextureCubeArray.MostDetailedMip = viewInfo->m_mostDetailedMip;
			srvDesc.TextureCubeArray.NumCubes = viewInfo->m_numTextures / 6;
			break;
		default:
			ConsoleLogErrorf("Couldn't create SRV for %s, view dimension was invalid", m_resourceID.ToString());
			return nullptr;
			break;
		}


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


//-------------------------------------------------------------------------------------------------
RenderTargetView* Texture::CreateOrGetColorTargetView(const TextureViewCreateInfo* viewInfo /*= nullptr*/)
{
	if (viewInfo == nullptr)
	{
		ConsoleLogErrorf("Couldn't create color target view for texture %s, viewInfo was nullptr so it wasn't defaulted.", m_resourceID.ToString());
		return nullptr;
	}

	if (m_dxHandle == nullptr)
	{
		ConsoleLogErrorf("Couldn't create color target view for texture %s, texture wasn't created.", m_resourceID.ToString());
		return nullptr;
	}

	// Default the info
	TextureViewCreateInfo defaultInfo;
	defaultInfo.m_viewUsage = TEXTURE_USAGE_RENDER_TARGET_BIT;

	if (viewInfo == nullptr)
	{
		viewInfo = &defaultInfo;
	}

	TextureView* view = GetView(viewInfo);
	if (view != nullptr)
	{
		RenderTargetView* viewAsRTV = dynamic_cast<RenderTargetView*>(view);
		ASSERT_OR_DIE(viewAsRTV != nullptr, "Couldn't cast view into ColorTargetView!");

		return viewAsRTV;
	}
	else
	{
		// Create a RenderTargetView view of this texture
		ID3D11Device* dxDevice = g_renderContext->GetDxDevice();
		ID3D11RenderTargetView* dxRTV = nullptr;
		HRESULT hr = dxDevice->CreateRenderTargetView(m_dxHandle, nullptr, &dxRTV);

		if (SUCCEEDED(hr))
		{
			RenderTargetView* colorTargetView = new RenderTargetView();

			colorTargetView->m_dxRTV = dxRTV;
			colorTargetView->m_sourceTexture = this;
			colorTargetView->m_byteSize = m_byteSize;
			colorTargetView->m_usage = TEXTURE_USAGE_RENDER_TARGET_BIT;
			colorTargetView->m_createInfo = *viewInfo;
			uint32 infoHash = HashData((void*)viewInfo, sizeof(viewInfo));
			colorTargetView->m_createInfoHash = infoHash;

			// Add it to the map
			m_views.push_back(colorTargetView);

			return colorTargetView;
		}
		else
		{
			ERROR_RECOVERABLE("Failed to create RenderTargetView, error: %i", hr);
			return nullptr;
		}
	}
}


//-------------------------------------------------------------------------------------------------
DepthStencilView* Texture::CreateOrGetDepthStencilView(const TextureViewCreateInfo* viewInfo /*= nullptr*/)
{
	if (viewInfo == nullptr)
	{
		ConsoleLogErrorf("Couldn't create depth stencil view for texture %s, viewInfo was nullptr so it wasn't defaulted.", m_resourceID.ToString());
		return nullptr;
	}

	if (m_dxHandle == nullptr)
	{
		ConsoleLogErrorf("Couldn't create depth stencil view for texture %s, texture wasn't created.", m_resourceID.ToString());
		return nullptr;
	}

	// Default the info
	TextureViewCreateInfo defaultInfo;
	defaultInfo.m_viewUsage = TEXTURE_USAGE_DEPTH_STENCIL_BIT;

	if (viewInfo == nullptr)
	{
		viewInfo = &defaultInfo;
	}

	TextureView* view = GetView(viewInfo);
	if (view != nullptr)
	{
		DepthStencilView* viewAsDSV = dynamic_cast<DepthStencilView*>(view);
		ASSERT_OR_DIE(viewAsDSV != nullptr, "Couldn't cast view into DepthStencilView!");

		return viewAsDSV;
	}
	else
	{
		// Create a DepthStencilView of this texture
		ID3D11Device* dxDevice = g_renderContext->GetDxDevice();
		ID3D11DepthStencilView* dxDSV = nullptr;

		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
		memset(&desc, 0, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		
		HRESULT hr = dxDevice->CreateDepthStencilView(m_dxHandle, &desc, &dxDSV);

		if (dxDSV != nullptr)
		{
			DepthStencilView* depthStencilView = new DepthStencilView();

			depthStencilView->m_dxDSV = dxDSV;
			depthStencilView->m_sourceTexture = this;
			depthStencilView->m_byteSize = m_byteSize;
			depthStencilView->m_usage = TEXTURE_USAGE_DEPTH_STENCIL_BIT;
			depthStencilView->m_createInfo = *viewInfo;
			uint32 infoHash = HashData((void*)viewInfo, sizeof(viewInfo));
			depthStencilView->m_createInfoHash = infoHash;

			// Add it to the map
			m_views.push_back(depthStencilView);

			return depthStencilView;
		}
		else
		{
			ERROR_RECOVERABLE("Failed to create DepthStencilView, error: %i", hr);
			return nullptr;
		}
	}
}


//-------------------------------------------------------------------------------------------------
void Texture::Clear()
{	
	uint32 numViews = (uint32) m_views.size();
	for (uint32 viewIndex = 0; viewIndex < numViews; ++viewIndex)
	{
		SAFE_DELETE(m_views[viewIndex]);
	}

	m_views.clear();

	DX_SAFE_RELEASE(m_dxHandle);

	m_memoryUsage = GPU_MEMORY_USAGE_DYNAMIC;
	m_textureUsage = 0U;
	m_dimensions = IntVector3::ZERO;
	m_format = TEXTURE_FORMAT_INVALID;
	m_byteSize = 0;
}


//-------------------------------------------------------------------------------------------------
TextureView* Texture::GetView(const TextureViewCreateInfo* viewInfo) const
{
	if (viewInfo != nullptr)
	{
		uint32 infoHash = HashData((void*)viewInfo, sizeof(viewInfo));
		uint32 numViews = (uint32)m_views.size();
		for (uint32 viewIndex = 0; viewIndex < numViews; ++viewIndex)
		{
			if (m_views[viewIndex]->GetCreateInfoHash() == infoHash)
			{
				return m_views[viewIndex];
			}
		}
	}

	return nullptr;
}
