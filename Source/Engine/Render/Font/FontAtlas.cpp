///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 17th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
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
void FontAtlas::Initialize(const Font* font, uint32 pixelHeight, uint32 maxHorizontalPixelAdvance, uint32 verticalPixelAdvance, Texture2D* texture /*= nullptr*/)
{
	if (texture == nullptr)
	{
		// TODO: Don't use pixel height, instead use the max dimension of the face bounding box
		texture = new Texture2D();

		// Just an estimation on the smallest power of two dimensions for the texture that can hold all the sprites
		// This may not be large enough, but most likely will be too large
		uint32 size = 2U;
		while (size < 8U * pixelHeight && size <= 2048U)
		{
			size *= 2U;
		}

		texture->CreateFromBuffer(nullptr, 0U, size, size, 4U, Stringf("Font %s, pixel height %u", font->GetSourceFile().c_str(), pixelHeight).c_str());
	}

	m_glyphPacker = new SpritePacker();
	m_glyphPacker->Initialize(texture);
	m_ownerFont = font;
	m_pixelHeight = pixelHeight;
	m_maxHorizontalPixelAdvance = maxHorizontalPixelAdvance;
	m_verticalPixelAdvance = verticalPixelAdvance;
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
