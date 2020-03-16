///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 14th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/IO/Image.h"
#define STB_IMAGE_IMPLEMENTATION
#include "ThirdParty/stb/stb_image.h"

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
Image::Image(const IntVector2& dimensions, const Rgba& color /*= Rgba::WHITE*/)
	: m_dimensions(dimensions)
	, m_numComponentsPerTexel(4)
{
	m_data = (uint8*)malloc(sizeof(uint8) * m_numComponentsPerTexel * GetTexelCount());

	for (int texelIndex = 0; texelIndex < GetTexelCount(); ++texelIndex)
	{
		int offset = texelIndex * m_numComponentsPerTexel;

		m_data[offset + 0] = color.r;
		m_data[offset + 1] = color.g;
		m_data[offset + 2] = color.b;
		m_data[offset + 3] = color.a;
	}
}


//-------------------------------------------------------------------------------------------------
Image::~Image()
{
	SAFE_FREE_POINTER(m_data);
}


//-------------------------------------------------------------------------------------------------
bool Image::LoadFromFile(const char* filepath, bool flipVertically)
{
	ASSERT_OR_DIE(m_data == nullptr, "Image already loaded!");

	int numComponentsRequested = 0;
	if (flipVertically) { stbi_set_flip_vertically_on_load(1); }
	m_data = (uint8*)stbi_load(filepath, &m_dimensions.x, &m_dimensions.y, &m_numComponentsPerTexel, numComponentsRequested);
	stbi_set_flip_vertically_on_load(1);
	
	if (m_data != nullptr)
	{
		m_filepath = filepath;
		m_size = m_dimensions.x * m_dimensions.y * m_numComponentsPerTexel;
	}

	return (m_data != nullptr);
}


//-------------------------------------------------------------------------------------------------
void Image::SetTexelColor(uint32 x, uint32 y, Rgba color)
{
	// Safety check
	ASSERT_OR_DIE(x >= 0 && y >= 0 && x < (uint32)m_dimensions.x && y < (uint32)m_dimensions.y, "Texel coordinates out of bounds: (%u, %u) for image size (%i, %i)", x, y, m_dimensions.y, m_dimensions.y);

	uint32 index = GetDataIndexForTexel(x, y);

	if (m_numComponentsPerTexel >= 1) { m_data[index + 0] = color.r; }
	if (m_numComponentsPerTexel >= 2) { m_data[index + 1] = color.g; }
	if (m_numComponentsPerTexel >= 3) { m_data[index + 2] = color.b; }
	if (m_numComponentsPerTexel >= 4) { m_data[index + 3] = color.a; }
}


//-------------------------------------------------------------------------------------------------
Rgba Image::GetTexelColor(uint32 x, uint32 y) const
{
	ASSERT_OR_DIE(x >= 0 && y >= 0 && x < (uint32)m_dimensions.x && y < (uint32)m_dimensions.y, "Image coords were out of bounds, coords were (%u, %u) for image of dimensions (%i, %i)", x, y, m_dimensions.x, m_dimensions.y)

	uint32 dataIndex = GetDataIndexForTexel(x, y);

	Rgba color;
	if (m_numComponentsPerTexel >= 1) { color.r = m_data[dataIndex + 0]; }
	if (m_numComponentsPerTexel >= 2) { color.g = m_data[dataIndex + 1]; }
	if (m_numComponentsPerTexel >= 3) { color.b = m_data[dataIndex + 2]; }
	if (m_numComponentsPerTexel >= 4) { color.a = m_data[dataIndex + 3]; }
	
	return color;
}


//-------------------------------------------------------------------------------------------------
uint32 Image::GetDataIndexForTexel(uint32 x, uint32 y) const
{
	return (y * (uint32)m_dimensions.x + x) * m_numComponentsPerTexel;
}
