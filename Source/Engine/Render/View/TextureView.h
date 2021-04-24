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

typedef uint32 TextureUsageBits;
class Texture;
struct ID3D11View;
struct ID3D11ShaderResourceView;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;

struct TextureViewCreateInfo
{
	TextureUsageBits m_viewType = 0;
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
