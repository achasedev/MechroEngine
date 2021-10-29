///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 2nd, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/Math/IntVector3.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
enum TextureUsageBit : uint32
{
	TEXTURE_USAGE_NO_BIND					= BIT_FLAG(1),	// Can be used to create a staging texture (no binds)
	TEXTURE_USAGE_SHADER_RESOURCE_BIT		= BIT_FLAG(2),	// Can be used to create a ShaderResourceView
	TEXTURE_USAGE_RENDER_TARGET_BIT			= BIT_FLAG(3),	// Can be used to create a ColorTargetView
	TEXTURE_USAGE_DEPTH_STENCIL_TARGET_BIT	= BIT_FLAG(4),	// Can be used to create a DepthStencilTargetView
};

enum ViewDimension
{
	VIEW_DIMENSIONS_INVALID = -1,
	VIEW_DIMENSION_TEXTURE2D,
	VIEW_DIMENSION_TEXTURECUBE,
	VIEW_DIMENSION_TEXTURE2DARRAY
};


typedef uint32 TextureUsageBits;
class Texture;
struct ID3D11View;
struct ID3D11ShaderResourceView;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;

struct TextureViewCreateInfo
{
	TextureUsageBits	m_viewUsage = 0;
	ViewDimension		m_viewDimension = VIEW_DIMENSIONS_INVALID;

	// Texture2D and Texture2DArray
	int					m_numMipLevels = 1;
	int					m_mostDetailedMip = 0;

	// Texture2DArray
	int					m_numTextures = -1;
	int					m_firstTextureIndex = 0;

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class TextureView
{
	friend class Texture;
	friend class Texture2DArray;

public:
	//-----Public Methods-----

	virtual ~TextureView();

	int		GetWidth() const;
	int		GetHeight() const;
	float	GetAspect() const;
	uint32	GetCreateInfoHash() const;


protected:
	//-----Protected Data-----

	TextureUsageBits				m_usage = 0;
	const Texture*					m_sourceTexture = nullptr;
	uint32							m_byteSize = 0;
	TextureViewCreateInfo			m_createInfo;
	uint32							m_createInfoHash = 0U;

	union
	{
		ID3D11View*					m_dxView = nullptr;
		ID3D11ShaderResourceView*	m_dxSRV;
		ID3D11RenderTargetView*		m_dxRTV;
		ID3D11DepthStencilView*		m_dxDSV;
	};

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
