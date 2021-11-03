///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: Nov 3rd, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/DevConsole.h"
#include "Engine/Core/EngineCommon.h"
#include "Engine/Render/RenderContext.h"
#include "Engine/Render/Texture/TextureCubeArray.h"

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
TextureCubeArray::~TextureCubeArray()
{

}


//-------------------------------------------------------------------------------------------------
// Creates a texture cube array with each side of each cube having the given dimensions, and of the given format
bool TextureCubeArray::Create(uint32 numCubes, int width, int height, TextureFormat format)
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
	texDesc.ArraySize = numCubes * 6;
	texDesc.Usage = (D3D11_USAGE)ToDXMemoryUsage(m_memoryUsage);
	texDesc.Format = static_cast<DXGI_FORMAT>(GetDxFormatFromTextureFormat(m_format));
	texDesc.BindFlags = GetDxBindFromTextureUsageFlags(m_textureUsage);
	texDesc.MiscFlags = 0U;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	texDesc.CPUAccessFlags = 0U;

	ID3D11Texture2D* dxTex2D = nullptr;
	HRESULT hr = dxDevice->CreateTexture2D(&texDesc, nullptr, &dxTex2D);

	bool succeeded = SUCCEEDED(hr);
	ASSERT_RECOVERABLE(succeeded, "Couldn't create TextureCubeArray!");

	if (succeeded)
	{
		m_dxHandle = dxTex2D;
		m_dimensions = IntVector3(width, height, 0);
		m_byteSize = numCubes * width * height * 6 * 4; // 6 sides per cube, 4 bytes per texel
		m_numCubes = numCubes;
		DX_SET_DEBUG_NAME(m_dxHandle, Stringf("TextureCubeArray | Dimensions: (%i, %i) | Number of Cubes: %i ", m_srcFilepath.c_str(), width, height, numCubes));
	}
	else
	{
		Clear();
	}

	return succeeded;
}


//-------------------------------------------------------------------------------------------------
// Creates an SRV for the entire array no create info is specified
ShaderResourceView* TextureCubeArray::CreateOrGetShaderResourceView(const TextureViewCreateInfo* viewInfo /*= nullptr*/)
{
	if (viewInfo == nullptr)
	{
		// Make sure we use the right default
		TextureViewCreateInfo cubeArrayViewInfo;
		cubeArrayViewInfo.m_viewDimension = VIEW_DIMENSION_TEXTURECUBEARRAY;
		cubeArrayViewInfo.m_viewUsage = TEXTURE_USAGE_SHADER_RESOURCE_BIT;
		cubeArrayViewInfo.m_firstTextureIndex = 0;
		cubeArrayViewInfo.m_numTextures = m_numCubes * 6;

		return Texture::CreateOrGetShaderResourceView(&cubeArrayViewInfo);
	}

	return Texture::CreateOrGetShaderResourceView(viewInfo);
}


//-------------------------------------------------------------------------------------------------
// Creates a CTV for the entire array if no create info is specified
RenderTargetView* TextureCubeArray::CreateOrGetColorTargetView(const TextureViewCreateInfo* viewInfo /*= nullptr*/)
{
	ConsoleLogErrorf("No creating color target views for TextureCubeArrays!");
	return nullptr;
}


//-------------------------------------------------------------------------------------------------
// Creates a DSV for the entire array if no create info is specified
DepthStencilView* TextureCubeArray::CreateOrGetDepthStencilView(const TextureViewCreateInfo* viewInfo /*= nullptr*/)
{
	ConsoleLogErrorf("No creating depth stencil views for TextureCubeArrays!");
	return nullptr;
}
