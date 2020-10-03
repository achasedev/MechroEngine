///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 14, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Render/Core/DX11Common.h"
#include "Engine/Render/Core/RenderContext.h"
#include "Engine/Render/Texture/Texture.h"
#include "Engine/Render/View/RenderTargetView.h"
#include "Engine/Render/View/ShaderResourceView.h"
#include "Engine/Render/View/DepthStencilTargetView.h"
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
Texture::~Texture()
{
	Clear();
}


//-------------------------------------------------------------------------------------------------
ShaderResourceView* Texture::CreateOrGetShaderResourceView(const TextureViewCreateInfo* viewInfo /*= nullptr*/)
{
	ASSERT_OR_DIE(m_dxHandle != nullptr, "Attempted to create a view for an uninitialized Texture!");
	ASSERT_OR_DIE((m_textureUsage & TEXTURE_USAGE_SHADER_RESOURCE_BIT) != 0, "Attempted to create a ShaderResourceView for a texture that doesn't support it!");

	// Default the info
	TextureViewCreateInfo defaultInfo;
	defaultInfo.m_viewType = m_textureUsage;

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
		ID3D11Device* dxDevice = g_renderContext->GetDxDevice();
		ID3D11ShaderResourceView* dxSRV = nullptr;
		dxDevice->CreateShaderResourceView(m_dxHandle, nullptr, &dxSRV);

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
	ASSERT_OR_DIE(m_dxHandle != nullptr, "Attempted to create a view for an uninitialized Texture!");
	ASSERT_OR_DIE((m_textureUsage & TEXTURE_USAGE_RENDER_TARGET_BIT) != 0, "Attempted to create a ColorTargetView for a texture that doesn't support it!");

	// Default the info
	TextureViewCreateInfo defaultInfo;
	defaultInfo.m_viewType = m_textureUsage;

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
DepthStencilTargetView* Texture::CreateOrGetDepthStencilTargetView(const TextureViewCreateInfo* viewInfo /*= nullptr*/)
{
	ASSERT_OR_DIE(m_dxHandle != nullptr, "Attempted to create a view for an uninitialized Texture!");
	ASSERT_OR_DIE((m_textureUsage & TEXTURE_USAGE_DEPTH_STENCIL_TARGET_BIT) != 0, "Attempted to create a DepthStencilView for a texture that doesn't support it!");

	// Default the info
	TextureViewCreateInfo defaultInfo;
	defaultInfo.m_viewType = m_textureUsage;

	if (viewInfo == nullptr)
	{
		viewInfo = &defaultInfo;
	}

	TextureView* view = GetView(viewInfo);
	if (view != nullptr)
	{
		DepthStencilTargetView* viewAsDSV = dynamic_cast<DepthStencilTargetView*>(view);
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
			DepthStencilTargetView* depthStencilView = new DepthStencilTargetView();

			depthStencilView->m_dxDSV = dxDSV;
			depthStencilView->m_sourceTexture = this;
			depthStencilView->m_byteSize = m_byteSize;
			depthStencilView->m_usage = TEXTURE_USAGE_DEPTH_STENCIL_TARGET_BIT;
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
