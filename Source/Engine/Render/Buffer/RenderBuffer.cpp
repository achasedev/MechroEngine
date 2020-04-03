///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: December 15th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Render/Core/DX11Common.h"
#include "Engine/Render/Buffer/RenderBuffer.h"
#include "Engine/Render/Core/RenderContext.h"

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
uint32 ToDXMemoryUsage(GPUMemoryUsage usage)
{
	switch (usage)
	{
	case GPU_MEMORY_USAGE_GPU: return D3D11_USAGE_DEFAULT; break;
	case GPU_MEMORY_USAGE_STATIC: return D3D11_USAGE_IMMUTABLE; break;
	case GPU_MEMORY_USAGE_DYNAMIC: return D3D11_USAGE_DYNAMIC; break;
	case GPU_MEMORY_USAGE_STAGING: return D3D11_USAGE_STAGING; break;
	default:
		ERROR_AND_DIE("Bad Memory Usage");
		break;
	}
}


//-------------------------------------------------------------------------------------------------
GPUMemoryUsage FromDXMemoryUsage(uint32 dxUsage)
{
	switch (dxUsage)
	{
	case D3D11_USAGE_DEFAULT: return GPU_MEMORY_USAGE_GPU; break;
	case D3D11_USAGE_IMMUTABLE: return GPU_MEMORY_USAGE_STATIC; break;
	case D3D11_USAGE_DYNAMIC: return GPU_MEMORY_USAGE_DYNAMIC; break;
	case D3D11_USAGE_STAGING: return GPU_MEMORY_USAGE_STAGING; break;
	default:
		ERROR_AND_DIE("Bad Memory Usage");
		break;
	}
}


//-------------------------------------------------------------------------------------------------
static uint32 GetDxBindFromRenderBufferFlags(RenderBufferUsageBitFlags flags)
{
	uint32 dxFlags = 0U;

	if (flags & RENDER_BUFFER_USAGE_VERTEX_STREAM_BIT)
	{
		dxFlags |= D3D11_BIND_VERTEX_BUFFER;
	}

	if (flags & RENDER_BUFFER_USAGE_INDEX_STREAM_BIT)
	{
		dxFlags |= D3D11_BIND_INDEX_BUFFER;
	}

	if (flags & RENDER_BUFFER_USAGE_UNIFORMS_BIT)
	{
		dxFlags |= D3D11_BIND_CONSTANT_BUFFER;
	}

	return dxFlags;
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
RenderBuffer::~RenderBuffer()
{
	DX_SAFE_RELEASE(m_dxHandle);
}


//-------------------------------------------------------------------------------------------------
void RenderBuffer::Reset()
{
	DX_SAFE_RELEASE(m_dxHandle);

	m_bufferSizeBytes = 0;
	m_elementSize = 0;
	m_memoryUsage = GPU_MEMORY_USAGE_GPU;
	m_usageFlags = 0;
}


//-------------------------------------------------------------------------------------------------
bool RenderBuffer::CreateOnGPU(const void* data, size_t bufferSizeBytes, size_t elementSize, RenderBufferUsageBitFlags bufferUsageFlags, GPUMemoryUsage memoryUsage)
{
	Reset();
		
	if (bufferSizeBytes == 0 || elementSize == 0)
	{
		return false;
	}

	// Static buffer needs data at creation
	if (memoryUsage == GPU_MEMORY_USAGE_STATIC && data == nullptr)
	{
		return false;
	}

	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(bufferDesc));

	bufferDesc.ByteWidth = (UINT)bufferSizeBytes;
	bufferDesc.StructureByteStride = (UINT)elementSize;
	bufferDesc.Usage = (D3D11_USAGE)ToDXMemoryUsage(memoryUsage);
	bufferDesc.BindFlags = GetDxBindFromRenderBufferFlags(bufferUsageFlags);

	// Give CPU read/write permissions
	if (memoryUsage == GPU_MEMORY_USAGE_DYNAMIC)
	{
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else if (memoryUsage == GPU_MEMORY_USAGE_STAGING)
	{
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
	}

	// Map data if it was given
	D3D11_SUBRESOURCE_DATA resourceData;
	D3D11_SUBRESOURCE_DATA* dataPtr = nullptr;
	if (data != nullptr) {
		memset(&resourceData, 0, sizeof(resourceData));
		resourceData.pSysMem = data;
		dataPtr = &resourceData;
	}

	ID3D11Device* device = g_renderContext->GetDxDevice();
	HRESULT hr = device->CreateBuffer(&bufferDesc, dataPtr, &m_dxHandle);

	if (SUCCEEDED(hr))
	{
		m_bufferSizeBytes = bufferSizeBytes;
		m_elementSize = elementSize;
		m_memoryUsage = memoryUsage;
		m_usageFlags = bufferUsageFlags;

		return true;
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
bool RenderBuffer::CopyToGPU(const void* data, size_t byteSize)
{
	ASSERT_OR_DIE(m_dxHandle != nullptr, "RenderBuffer not created on GPU!");
	ASSERT_OR_DIE(m_memoryUsage != GPU_MEMORY_USAGE_STATIC, "CopyToGpu called on a static buffer!");
	ASSERT_OR_DIE(byteSize <= m_bufferSizeBytes, "Not enough room in buffer to copy!");

	ID3D11DeviceContext* dxContext = g_renderContext->GetDxContext();

	D3D11_MAPPED_SUBRESOURCE resource;
	HRESULT hr = dxContext->Map(m_dxHandle, 0, D3D11_MAP_WRITE_DISCARD, 0U, &resource);

	if (SUCCEEDED(hr))
	{
		memcpy(resource.pData, data, byteSize);
		dxContext->Unmap(m_dxHandle, 0);

		return true;
	}

	return false;
}
