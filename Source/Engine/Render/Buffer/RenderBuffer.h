///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: December 15th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
enum GPUMemoryUsage
{
	GPU_MEMORY_USAGE_GPU,     // Can be written/read from GPU only (Color Targets are a good example)
	GPU_MEMORY_USAGE_STATIC,  // Created, and are read only after that (ex: textures from images, sprite atlas from disk)
	GPU_MEMORY_USAGE_DYNAMIC, // Update often from CPU, used by the GPU (CPU -> GPU updates, used by shaders.  ex: Uniform Buffers)
	GPU_MEMORY_USAGE_STAGING, // For getting memory from GPU to CPU (can be copied into, but not directly bound as an output.  ex: Screenshots system)
};

enum RenderBufferUsageBit : uint32
{
	RENDER_BUFFER_USAGE_VERTEX_STREAM_BIT = BIT_FLAG(0),
	RENDER_BUFFER_USAGE_INDEX_STREAM_BIT = BIT_FLAG(1),
	RENDER_BUFFER_USAGE_UNIFORMS_BIT = BIT_FLAG(2)
};

typedef unsigned int RenderBufferUsageBitFlags;
struct ID3D11Buffer;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class RenderBuffer
{
public:
	//-----Public Methods-----

	RenderBuffer() {}
	virtual ~RenderBuffer();

	size_t			GetBufferSize() const { return m_bufferSizeBytes; }
	ID3D11Buffer*	GetDxHandle() const { return m_dxHandle; }


protected:
	//-----Protected Methods-----

	void Reset();
	bool CreateOnGPU(const void* data, size_t byteSize, size_t elementSize, RenderBufferUsageBitFlags bufferUsage, GPUMemoryUsage memoryUsage);
	bool CopyToGPU(const void* data, size_t byteSize);

	bool IsStatic() const { return m_memoryUsage == GPU_MEMORY_USAGE_STATIC; }
	bool IsDynamic() const { return m_memoryUsage == GPU_MEMORY_USAGE_DYNAMIC; }


private:
	//-----Private Data------

	RenderBufferUsageBitFlags m_usageFlags = 0;
	GPUMemoryUsage m_memoryUsage = GPU_MEMORY_USAGE_GPU;

	size_t m_bufferSizeBytes = 0;
	size_t m_elementSize = 0; // Used for stride

	ID3D11Buffer* m_dxHandle = nullptr;

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
uint32 ToDXMemoryUsage(GPUMemoryUsage usage); // TODO: Move these to Dxcommon
GPUMemoryUsage FromDXMemoryUsage(uint32 dxUsage);
