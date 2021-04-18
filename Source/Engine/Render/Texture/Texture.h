///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 14th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Math/IntVector3.h"
#include "Engine/Render/Buffer/RenderBuffer.h"
#include "Engine/Render/View/TextureView.h"
#include "Engine/Resource/Resource.h"
#include <vector>

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class DepthStencilTargetView;
class Image;
class RenderTargetView;
class ShaderResourceView;
struct ID3D11Resource;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class Texture : public Resource
{
public:
	//-----Public Methods-----

	virtual ~Texture();

	void								Clear();
	virtual ShaderResourceView*			CreateOrGetShaderResourceView(const TextureViewCreateInfo* viewInfo = nullptr);
	virtual RenderTargetView*			CreateOrGetColorTargetView(const TextureViewCreateInfo* viewInfo = nullptr);
	virtual DepthStencilTargetView*		CreateOrGetDepthStencilTargetView(const TextureViewCreateInfo* viewInfo = nullptr);

	int									GetWidth() const { return m_dimensions.x; }
	int									GetHeight() const { return m_dimensions.y; }
	uint32								GetSize() const { return m_byteSize; }
	ID3D11Resource*						GetDxHandle() const { return m_dxHandle; }


protected:
	//------Protected Methods-----

	TextureView*						GetView(const TextureViewCreateInfo* viewInfo) const;


protected:
	//-----Protected Data-----

	std::string							m_srcFilepath = "INTERNAL_TEXTURE";
	ID3D11Resource*						m_dxHandle = nullptr;
	GPUMemoryUsage						m_memoryUsage = GPU_MEMORY_USAGE_DYNAMIC;
	TextureUsageBits					m_textureUsage = 0;
	IntVector3							m_dimensions = IntVector3::ZERO;
	uint32								m_byteSize = 0;
	std::vector<TextureView*>			m_views;

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
