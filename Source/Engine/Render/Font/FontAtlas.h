///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 17th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Math/AABB2.h"
#include <map>

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Font;
class SpritePacker;
class Texture2D;

struct GlyphInfo
{
	AABB2		m_glyphUVs = AABB2::ZERO_TO_ONE;
	uint32		m_pixelWidth = 0;
	uint32		m_pixelHeight = 0;
	int			m_pixelHorizontalAdvance = 0;
	int			m_pixelVerticalAdvance = 0;
	int			m_pixelLeftSideBearing = 0;
	int			m_pixelRightSideBearing = 0;
	int			m_pixelTopSideBearing = 0;
	int			m_pixelBottomSideBearing = 0;
};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class FontAtlas
{
public:
	//-----Public Methods-----

	void			Initialize(const Font* font, uint32 pixelHeight, uint32 maxPixelAdvance, uint32 pixelLineSpacing, int maxPixelAscent, int maxPixelDescent, Texture2D* texture = nullptr);

	GlyphInfo		CreateOrGetGlyphInfo(const char glyph);
	uint32			GetPixelHeight() const { return m_pixelHeight; }
	uint32			GetVerticalLineSpacingPixels() const { return m_verticalPixelLineSpacing; }
	uint32			GetMaxAscentPixels() const { return m_maxGlyphPixelAscent; }
	uint32			GetMaxDescentPixels() const { return m_maxGlyphPixelDecent; }
	Texture2D*		GetTexture();
	IntVector2		GetTextDimensionsPixels(const std::string& text);


private:
	//-----Private Data-----

	uint32								m_pixelHeight = 1U;
	std::map<const char, GlyphInfo>		m_glyphUVs;
	const Font*							m_ownerFont = nullptr;
	SpritePacker*						m_glyphPacker = nullptr;
	uint32								m_maxHorizontalPixelAdvance = 0U;
	uint32								m_verticalPixelLineSpacing = 0U;
	int									m_maxGlyphPixelAscent = 0;
	int									m_maxGlyphPixelDecent = 0;

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
