///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 17th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
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
void FontAtlas::Initialize(const Font* font, uint32 pixelHeight, Texture2D* texture /*= nullptr*/)
{
	if (texture == nullptr)
	{
		texture = new Texture2D();
		texture->CreateFromBuffer(nullptr, 0U, 1024, 1024, 4U, Stringf("Font %s, pixel height %u", font->GetSourceFile().c_str(), pixelHeight).c_str());
	}

	m_glyphPacker = new SpritePacker();
	m_glyphPacker->Initialize(texture);
	m_ownerFont = font;
	m_pixelHeight = pixelHeight;
}


//-------------------------------------------------------------------------------------------------
Texture2D* FontAtlas::GetTexture()
{
	return m_glyphPacker->GetTexture();
}


//-------------------------------------------------------------------------------------------------
AABB2 FontAtlas::CreateOrGetUVsForGlyph(const char glyph)
{
	bool glyphExists = m_glyphUVs.find(glyph) != m_glyphUVs.end();

	if (glyphExists)
	{
		return m_glyphUVs.at(glyph);
	}

	// Have Freetype create a rendering for our pixel height, then pack it in the atlas
	int glyphWidth = 0;
	int glyphHeight = 0;
	AABB2 glyphUVs = AABB2::ZERO_TO_ONE;

	const uint8* glyphSrc = m_ownerFont->RenderGlyphForPixelHeight(glyph, m_pixelHeight, glyphWidth, glyphHeight);
	bool glyphPacked = m_glyphPacker->PackSprite(glyphSrc, glyphWidth, glyphHeight, 1U, glyphUVs);

	if (glyphPacked)
	{
		m_glyphUVs[glyph] = glyphUVs;
	}

	return glyphUVs;
}
