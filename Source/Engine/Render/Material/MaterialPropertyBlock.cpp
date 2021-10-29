///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: Oct 24th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/DevConsole.h"
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
	if (offset + dataSize > m_description->GetSize())
	{
		ConsoleLogErrorf("Property block %s attempted to write off the end of the block!", m_description->GetName().ToString());
		return;

	}

	if (m_cpuData == nullptr)
	{
		// Make the cpu buffer
		m_cpuData = malloc(m_description->GetSize());
	}

	// Copy into it with offset
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
		m_buffer.CopyToGPU(m_cpuData, m_description->GetSize());
		m_gpuNeedsUpdate = false;
	}
}


//-------------------------------------------------------------------------------------------------
// Returns the name of this block (matches name of the description from shader)
StringID MaterialPropertyBlock::GetName() const
{
	return m_description->GetName();
}
