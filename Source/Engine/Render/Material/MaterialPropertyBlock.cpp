///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: Oct 24th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/Render/Material/MaterialPropertyBlock.h"
#include "Engine/Render/Buffer/PropertyBlockDescription.h"

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
// Constructor
MaterialPropertyBlock::MaterialPropertyBlock(const PropertyBlockDescription* description)
	: m_description(description)
{
}


//-------------------------------------------------------------------------------------------------
// Sets the buffer to have the given data at the given offset. Will expand the buffer if necessary 
void MaterialPropertyBlock::SetCPUData(const void* data, int dataSize, int offset)
{
	size_t requestedLimit = dataSize + offset;
	if (m_cpuData == nullptr)
	{
		// Make the cpu buffer
		m_cpuData = malloc(requestedLimit);
		m_cpuDataSize = requestedLimit;
	}
	else if (requestedLimit > m_cpuDataSize)
	{
		// Data will run off the end of this buffer, so let's expand
		void* temp = malloc(requestedLimit);
		std::memcpy(temp, m_cpuData, m_cpuDataSize);

		free(m_cpuData);		
		m_cpuData = temp;
		m_cpuDataSize = requestedLimit; 
	}

	// Current cpu buffer is now big enough, so copy into it with offset
	unsigned char* dst = (unsigned char*)m_cpuData;

	std::memcpy(&dst[offset], data, dataSize);
	m_gpuNeedsUpdate = true;
}


//-------------------------------------------------------------------------------------------------
// Updates the gpu buffer with the current cpu data
void MaterialPropertyBlock::UpdateGPUData()
{
	if (m_gpuNeedsUpdate)
	{
		m_buffer.CopyToGPU(m_cpuData, m_cpuDataSize);
		m_gpuNeedsUpdate = false;
	}
}


//-------------------------------------------------------------------------------------------------
// Returns the name of this block (matches name of the description from shader)
StringID MaterialPropertyBlock::GetName() const
{
	return m_description->GetName();
}
