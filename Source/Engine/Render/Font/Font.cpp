///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 11th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Render/Font/Font.h"
#include "Engine/Render/Font/FontAtlas.h"
#include "Engine/Render/Texture/Texture2D.h"
#include "ThirdParty/freetype/include/ft2build.h"
#include FT_FREETYPE_H

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
FontAtlas* Font::CreateOrGetAtlasForPixelHeight(uint32 pixelHeight)
{
	bool fontExists = m_atlasRegistry.find(pixelHeight) != m_atlasRegistry.end();

	if (fontExists)
	{
		return m_atlasRegistry.at(pixelHeight);
	}

	FontAtlas* atlas = new FontAtlas();
	atlas->Initialize(this, pixelHeight);

	m_atlasRegistry[pixelHeight] = atlas;

	return atlas;
}


//-------------------------------------------------------------------------------------------------
const uint8* Font::RenderGlyphForPixelHeight(const char glyph, uint32 pixelHeight, int& out_glyphWidth, int& out_glyphHeight) const
{
	FT_Face face = (FT_Face)m_ftFace;
	FT_Set_Pixel_Sizes(face, 0, pixelHeight);

	int glyphIndex = FT_Get_Char_Index(face, glyph);
	FT_Error error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);

	ASSERT_OR_DIE(!error, "Error occured when loading glyph!");

	FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

	FT_Bitmap bitmap = face->glyph->bitmap;

	out_glyphWidth = bitmap.width;
	out_glyphHeight = bitmap.rows;

	return bitmap.buffer;
}


//-------------------------------------------------------------------------------------------------
const FontAtlas* Font::GetFontAtlasForPixelHeight(uint32 pixelHeight)
{
	bool fontExists = m_atlasRegistry.find(pixelHeight) != m_atlasRegistry.end();

	if (fontExists)
	{
		return m_atlasRegistry.at(pixelHeight);
	}

	return nullptr;
}
