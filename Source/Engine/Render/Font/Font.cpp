///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 11th, 2020
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
	FontAtlas* atlas = GetFontAtlasForPixelHeight(pixelHeight);

	if (atlas == nullptr)
	{
		FT_Face face = (FT_Face)m_ftFace;
		FT_Set_Pixel_Sizes(face, 0, pixelHeight);

		int maxAdvance = RoundToNearestInt((1.f / 64.f) * static_cast<float>(face->size->metrics.max_advance));
		int lineHeight = RoundToNearestInt((1.f / 64.f) * static_cast<float>(face->size->metrics.height));

		atlas = new FontAtlas();
		atlas->Initialize(this, pixelHeight, maxAdvance, lineHeight);

		m_atlasRegistry[pixelHeight] = atlas;
	}

	return atlas;
}


//-------------------------------------------------------------------------------------------------
const uint8* Font::RenderGlyphForPixelHeight(const char glyph, uint32 pixelHeight, GlyphInfo& out_info) const
{
	FT_Face face = (FT_Face)m_ftFace;
	FT_Set_Pixel_Sizes(face, 0, pixelHeight);

	int glyphIndex = FT_Get_Char_Index(face, glyph);
	FT_Error error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);

	ASSERT_OR_DIE(!error, "Error occured when loading glyph!");

	FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

	FT_GlyphSlot glyphSlot = face->glyph;
	FT_Bitmap bitmap = glyphSlot->bitmap;

	out_info.m_pixelWidth = bitmap.width;
	out_info.m_pixelHeight = bitmap.rows;

	out_info.m_pixelHorizontalAdvance = RoundToNearestInt(((1.0f / 64.0f) * glyphSlot->advance.x));
	out_info.m_pixelVerticalAdvance = RoundToNearestInt(((1.0f / 64.0f) * glyphSlot->advance.y));

	// Offsets from the origin position to the top and the left of the glyph start
	out_info.m_pixelLeftSideBearing = RoundToNearestInt((1.0f / 64.0f) * glyphSlot->metrics.horiBearingX);
	out_info.m_pixelTopSideBearing = RoundToNearestInt((1.0f / 64.0f) * glyphSlot->metrics.horiBearingY);

	// Other bearings are just the leftover offsets
	out_info.m_pixelRightSideBearing = out_info.m_pixelHorizontalAdvance - out_info.m_pixelLeftSideBearing - out_info.m_pixelWidth;
	out_info.m_pixelBottomSideBearing = out_info.m_pixelHeight - out_info.m_pixelTopSideBearing;

	return bitmap.buffer;
}


//-------------------------------------------------------------------------------------------------
FontAtlas* Font::GetFontAtlasForPixelHeight(uint32 pixelHeight)
{
	bool fontExists = m_atlasRegistry.find(pixelHeight) != m_atlasRegistry.end();

	if (fontExists)
	{
		return m_atlasRegistry.at(pixelHeight);
	}

	return nullptr;
}


//-------------------------------------------------------------------------------------------------
int Font::GetKerningInPixels(uint32 pixelHeight, const char firstGlyph, const char secondGlyph) const
{
	if (!m_hasKerning)
	{
		return 0U;
	}

	FT_Face face = (FT_Face)m_ftFace;
	FT_Set_Pixel_Sizes(face, 0, pixelHeight);

	int firstGlyphIndex = FT_Get_Char_Index(face, firstGlyph);
	int secondGlyphIndex = FT_Get_Char_Index(face, secondGlyph);

	FT_Vector  kerning;
	FT_Get_Kerning(face, firstGlyphIndex, secondGlyphIndex, FT_KERNING_DEFAULT, &kerning);

	// Only doing horizontal kerning
	return RoundToNearestInt((1.f / 64.f) * static_cast<float>(kerning.x));
}
