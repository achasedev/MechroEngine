///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: December 15th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** INCLUDES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** DEFINES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                              *** TYPES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
enum RenderBufferUsageBit : unsigned int
{
	RENDER_BUFFER_USAGE_VERTEX_STREAM_BIT	= BIT_FLAG(0),
	RENDER_BUFFER_USAGE_INDEX_STREAM_BIT	= BIT_FLAG(1),
	RENDER_BUFFER_USAGE_UNIFORMS_BIT		= BIT_FLAG(2)
};
typedef unsigned int RenderBufferUsageBitFlags;

enum GpuMemoryUsage
{
	GPU_MEMORY_USAGE_GPU,     // Can be written/read from GPU only (Color Targets are a good example)
	GPU_MEMORY_USAGE_STATIC,  // Created, and are read only after that (ex: textures from images, sprite atlas)
	GPU_MEMORY_USAGE_DYNAMIC, // Update often from CPU, used by the GPU (CPU -> GPU updates, used by shaders.  ex: Uniform Buffers)
	GPU_MEMORY_USAGE_STAGING, // For getting memory from GPU to CPU (can be copied into, but not directly bound as an output.  ex: Screenshots system)
};

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** STRUCTS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------
struct ID3D11Buffer;

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                        *** GLOBALS AND STATICS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** CLASSES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class RenderBuffer
{
public:
	//-----Public Methods-----

	RenderBuffer() {}
	virtual ~RenderBuffer();

	size_t GetBufferSize() const { return m_bufferSizeBytes; }


protected:
	//-----Protected Methods-----

	void Reset();
	bool CreateOnGpu(const void* data, size_t byteSize, size_t elementSize, RenderBufferUsageBitFlags bufferUsage, GpuMemoryUsage memoryUsage);
	bool CopyToGpu(const void* data, size_t byteSize);

	bool IsStatic() const { return m_memoryUsage == GPU_MEMORY_USAGE_STATIC; }
	bool IsDynamic() const { return m_memoryUsage == GPU_MEMORY_USAGE_DYNAMIC; }


private:
	//-----Private Data------

	RenderBufferUsageBitFlags m_usageFlags = 0;
	GpuMemoryUsage m_memoryUsage = GPU_MEMORY_USAGE_GPU;

	size_t m_bufferSizeBytes = 0;
	size_t m_elementSize = 0; // Used for stride

	ID3D11Buffer* m_bufferHandle = nullptr;

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                           *** C FUNCTIONS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------
