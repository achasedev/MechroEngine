///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 14th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Render/Font/Font.h" 
#include "Engine/Render/Font/FontLoader.h"
#include "ThirdParty/freetype/include/ft2build.h"   
#include FT_FREETYPE_H

#if defined(_WIN64)
#if defined(_DEBUG)
#pragma comment(lib, "ThirdParty/freetype/freetype_x64_Debug.lib")  
#else
#pragma comment(lib, "ThirdParty/freetype/freetype_x64_Release.lib") 
#endif
#else
#if defined (_DEBUG)
#pragma comment(lib, "ThirdParty/freetype/freetype_win32_Debug.lib")
#else
#pragma comment(lib, "ThirdParty/freetype/freetype_win32_Release.lib")
#endif
#endif

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------
static FT_Library s_library;
FontLoader* g_fontLoader = nullptr;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
void FontLoader::Initialize()
{
	g_fontLoader = new FontLoader();

	// Initialize the FT library
	FT_Error error = FT_Init_FreeType(&s_library);
	ASSERT_RECOVERABLE(!error, "Couldn't initialize FreeType library!");
}


//-------------------------------------------------------------------------------------------------
void FontLoader::Shutdown()
{
	FT_Done_FreeType(s_library);
	SAFE_DELETE(g_fontLoader);
}


//-------------------------------------------------------------------------------------------------
Font* FontLoader::LoadFont(const char* sourceFilepath, uint32 faceIndex)
{
	FT_Face face;
	FT_Error error = FT_New_Face(s_library, sourceFilepath, faceIndex, &face);

	if (error == FT_Err_Unknown_File_Format)
	{
		ERROR_AND_DIE("Unsupported font format for file %s", sourceFilepath);
	}
	else if (error)
	{
		ERROR_AND_DIE("Couldn't load font file %s", sourceFilepath);
	}

	Font* fontFace = new Font();
	fontFace->m_ftFace = (void*)face;
	fontFace->m_sourceFilepath = sourceFilepath;
	fontFace->m_hasKerning = FT_HAS_KERNING(face);

	m_fontFaces[SID(sourceFilepath)] = fontFace;

	return fontFace;
}


//-------------------------------------------------------------------------------------------------
FontLoader::~FontLoader()
{
	std::map<StringID, Font*>::iterator itr = m_fontFaces.begin();

	for (itr; itr != m_fontFaces.end(); itr++)
	{
		SAFE_DELETE(itr->second);
	}

	m_fontFaces.clear();
}
