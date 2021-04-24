///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 17th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Render/Font/Font.h"
#include "Engine/Render/Font/FontAtlas.h"
#include "Engine/Render/Texture/Texture2D.h"
#include "Engine/Utility/SpritePacker.h"
#include "Engine/Utility/StringUtils.h"

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
FontAtlas::~FontAtlas()
{
	m_pixelHeight = 1U;
	m_glyphUVs.clear();
	m_ownerFont = nullptr;
	m_maxHorizontalPixelAdvance = 0U;
	m_verticalPixelLineSpacing = 0U;
	m_maxGlyphPixelAscent = 0;
	m_maxGlyphPixelDecent = 0;

	SAFE_DELETE(m_glyphPacker);
}


//-------------------------------------------------------------------------------------------------
void FontAtlas::Initialize(const Font* font, uint32 pixelHeight, uint32 maxHorizontalPixelAdvance, uint32 pixelLineSpacing, int maxPixelAscent, int maxPixelDescent, Texture2D* texture /*= nullptr*/)
{
	if (texture == nullptr)
	{
		texture = new Texture2D();

		// Just an estimation on the smallest power of two dimensions for the texture that can hold all the sprites
		// This may not be large enough, but most likely will be too large
		uint32 maxSpriteDimension = Max(maxHorizontalPixelAdvance, pixelLineSpacing);
		uint32 size = 2U;
		while (size < 8U * maxSpriteDimension && size <= 2048U)
		{
			size *= 2U;
		}

		texture->CreateFromBuffer(nullptr, 0U, size, size, 4U, TEXTURE_USAGE_SHADER_RESOURCE_BIT, GPU_MEMORY_USAGE_GPU);
	}

	m_glyphPacker = new SpritePacker();
	m_glyphPacker->Initialize(texture);
	m_ownerFont = font;
	m_pixelHeight = pixelHeight;
	m_maxHorizontalPixelAdvance = maxHorizontalPixelAdvance;
	m_verticalPixelLineSpacing = pixelLineSpacing;
	m_maxGlyphPixelAscent = maxPixelAscent;
	m_maxGlyphPixelDecent = maxPixelDescent;
}


//-------------------------------------------------------------------------------------------------
Texture2D* FontAtlas::GetTexture()
{
	return m_glyphPacker->GetTexture();
}


//-------------------------------------------------------------------------------------------------
GlyphInfo FontAtlas::CreateOrGetGlyphInfo(const char glyph)
{
	bool glyphExists = m_glyphUVs.find(glyph) != m_glyphUVs.end();

	if (glyphExists)
	{
		return m_glyphUVs.at(glyph);
	}

	// Have Freetype create a rendering for our pixel height, then pack it in the atlas
	GlyphInfo info;
	const uint8* glyphSrc = m_ownerFont->RenderGlyphForPixelHeight(glyph, m_pixelHeight, info);

	if (glyphSrc != nullptr)
	{
		m_glyphPacker->PackSprite(glyphSrc, info.m_pixelWidth, info.m_pixelHeight, 1U, info.m_glyphUVs);
	}

	m_glyphUVs[glyph] = info;

	return info;
}


//-------------------------------------------------------------------------------------------------
IntVector2 FontAtlas::GetTextDimensionsPixels(const std::string& text)
{
	IntVector2 dimensions = IntVector2::ZERO;

	for (size_t charIndex = 0; charIndex < text.size(); ++charIndex)
	{
		GlyphInfo info = CreateOrGetGlyphInfo(text[charIndex]);
		
		dimensions.x += info.m_pixelHorizontalAdvance;
		dimensions.y = Max(dimensions.y, (int)info.m_pixelHeight);
	}

	return dimensions;
}
