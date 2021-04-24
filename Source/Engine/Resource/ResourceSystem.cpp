///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 17th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/DevConsole.h"
#include "Engine/Core/EngineCommon.h"
#include "Engine/IO/Image.h"
#include "Engine/Render/Font/Font.h"
#include "Engine/Render/Material.h"
#include "Engine/Render/Mesh/MeshBuilder.h"
#include "Engine/Render/Shader.h"
#include "Engine/Render/Texture/Texture2D.h"
#include "Engine/Resource/ResourceSystem.h"
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
typedef std::map<StringID, Texture2D*>::iterator Texture2DIter;
typedef std::map<StringID, Mesh*>::iterator MeshIter;
typedef std::map<StringID, Image*>::iterator ImageIter;
typedef std::map<StringID, Shader*>::iterator ShaderIter;
typedef std::map<StringID, Material*>::iterator MaterialIter;
typedef std::map<StringID, Font*>::iterator FontIter;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------
static FT_Library s_library;
ResourceSystem* g_resourceSystem = nullptr;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
void ResourceSystem::Initialize()
{
	ASSERT_OR_DIE(g_resourceSystem == nullptr, "ResourceSystem already initialized!");
	
	g_resourceSystem = new ResourceSystem();
	g_resourceSystem->CreateBuiltInAssets();

	// Initialize the FT library
	FT_Error error = FT_Init_FreeType(&s_library);
	ASSERT_RECOVERABLE(!error, "Couldn't initialize FreeType library!");
}


//-------------------------------------------------------------------------------------------------
void ResourceSystem::Shutdown()
{
	ASSERT_OR_DIE(g_resourceSystem != nullptr, "ResourceSystem not initialized!");
	SAFE_DELETE(g_resourceSystem);

	FT_Done_FreeType(s_library);
}


//-------------------------------------------------------------------------------------------------
Mesh* ResourceSystem::CreateOrGetMesh(const char* filepath)
{
	StringID id = SID(filepath);
	MeshIter itr = m_meshes.find(id);
	if (itr != m_meshes.end())
	{
		return itr->second;
	}

	// TODO: Mesh loading...
	return nullptr;
}


//-------------------------------------------------------------------------------------------------
Image* ResourceSystem::CreateOrGetImage(const char* filepath)
{
	StringID id = SID(filepath);
	ImageIter itr = m_images.find(id);
	if (itr != m_images.end())
	{
		return itr->second;
	}

	Image* image = new Image();
	bool success = image->Load(filepath);

	if (success)
	{
		image->m_resourceID = id;
	}
	else
	{
		SAFE_DELETE(image);
		ConsoleErrorf("Could not load Image %s", filepath);
	}

	m_images[id] = image;

	return image;
}


//-------------------------------------------------------------------------------------------------
Shader* ResourceSystem::CreateOrGetShader(const char* filepath)
{
	StringID id = SID(filepath);
	ShaderIter itr = m_shaders.find(id);
	if (itr != m_shaders.end())
	{
		return itr->second;
	}

	Shader* shader = new Shader();
	bool success = shader->Load(filepath);

	if (success)
	{
		shader->m_resourceID = id;
	}
	else
	{
		SAFE_DELETE(shader);
		ConsoleErrorf("Could not load Shader %s", filepath);
	}

	m_shaders[id] = shader;

	return shader;
}


//-------------------------------------------------------------------------------------------------
Material* ResourceSystem::CreateOrGetMaterial(const char* filepath)
{
	StringID id = SID(filepath);
	MaterialIter itr = m_materials.find(id);
	if (itr != m_materials.end())
	{
		return itr->second;
	}

	Material* material = new Material();
	bool success = material->Load(filepath);

	if (success)
	{
		material->m_resourceID = id;
	}
	else
	{
		SAFE_DELETE(material);
		ConsoleErrorf("Could not load Texture2D %s", filepath);
	}

	m_materials[id] = material;

	return material;
}


//-------------------------------------------------------------------------------------------------
Texture2D* ResourceSystem::CreateOrGetTexture2D(const char* filepath, TextureUsageBits textureUsage /*= TEXTURE_USAGE_SHADER_RESOURCE_BIT*/, GPUMemoryUsage memoryUsage /*= GPU_MEMORY_USAGE_STATIC*/)
{
	StringID id = SID(filepath);
	Texture2DIter itr = m_texture2Ds.find(id);
	if (itr != m_texture2Ds.end())
	{
		return itr->second;
	}

	Texture2D* texture = new Texture2D();
	bool success = texture->Load(filepath, textureUsage, memoryUsage);
	
	if (success)
	{
		texture->m_resourceID = id;
	}
	else
	{
		SAFE_DELETE(texture);
		ConsoleErrorf("Could not load Texture2D %s", filepath);
	}
	
	m_texture2Ds[id] = texture;

	return texture;
}


//-------------------------------------------------------------------------------------------------
Font* ResourceSystem::CreateOrGetFont(const char* filepath)
{
	StringID id = SID(filepath);
	FontIter itr = m_fonts.find(id);
	if (itr != m_fonts.end())
	{
		return itr->second;
	}

	Font* font = nullptr;
	FT_Face face;
	FT_Error error = FT_New_Face(s_library, filepath, 0, &face);

	if (error == FT_Err_Unknown_File_Format)
	{
		ConsoleErrorf("Unsupported font format for file %s", filepath);
	}
	else if (error)
	{
		ConsoleErrorf("Couldn't load font file %s", filepath);
	}
	else
	{
		font = new Font(face, filepath, FT_HAS_KERNING(face));
		font->m_resourceID = id;
	}

	m_fonts[id] = font;

	return font;
}


//-------------------------------------------------------------------------------------------------
ResourceSystem::ResourceSystem()
{
}


//-------------------------------------------------------------------------------------------------
void ResourceSystem::CreateBuiltInAssets()
{
	CreateDefaultMeshes();
	CreateDefaultShaders();
	CreateDefaultImages();
	CreateDefaultTexture2Ds();
	CreateDefaultMaterials();
}


//-------------------------------------------------------------------------------------------------
void ResourceSystem::CreateDefaultMeshes()
{
	MeshBuilder mb;
	mb.BeginBuilding(true);
	mb.PushCube(Vector3::ZERO, Vector3::ONES);
	mb.FinishBuilding();

	Mesh* cubeMesh = mb.CreateMesh<Vertex3D_PCU>();
	cubeMesh->m_resourceID = SID("unit_cube");
	m_meshes[cubeMesh->m_resourceID] = cubeMesh;

	mb.Clear();
	mb.BeginBuilding(true);
	mb.PushSphere(Vector3::ZERO, 1.f);
	mb.FinishBuilding();

	Mesh* sphereMesh = mb.CreateMesh<Vertex3D_PCU>();
	sphereMesh->m_resourceID = SID("unit_sphere");
	m_meshes[sphereMesh->m_resourceID] = sphereMesh;
}


//-------------------------------------------------------------------------------------------------
void ResourceSystem::CreateDefaultShaders()
{
	CreateOrGetShader("Data/Shader/default_opaque.shader");
	CreateOrGetShader("Data/Shader/default_alpha.shader");
	CreateOrGetShader("Data/Shader/font.shader");
	CreateOrGetShader("Data/Shader/invalid.shader");
}


//-------------------------------------------------------------------------------------------------
void ResourceSystem::CreateDefaultImages()
{
	Image* whiteImage = new Image(IntVector2(2, 2), Rgba::WHITE);
	whiteImage->m_resourceID = SID("white");
	m_images[whiteImage->m_resourceID] = whiteImage;

	CreateOrGetImage("Data/Image/debug.png");
}


//-------------------------------------------------------------------------------------------------
void ResourceSystem::CreateDefaultTexture2Ds()
{
	Image* whiteImage = CreateOrGetImage("white");
	Texture2D* whiteTexture = new Texture2D();
	whiteTexture->CreateFromImage(*whiteImage, TEXTURE_USAGE_SHADER_RESOURCE_BIT, GPU_MEMORY_USAGE_STATIC);
	whiteTexture->m_resourceID = whiteImage->m_resourceID;
	m_texture2Ds[whiteTexture->m_resourceID] = whiteTexture;

	Image* debugImage = CreateOrGetImage("Data/Image/debug.png");
	Texture2D* debugTexture = new Texture2D();
	debugTexture->CreateFromImage(*debugImage, TEXTURE_USAGE_SHADER_RESOURCE_BIT, GPU_MEMORY_USAGE_STATIC);
	debugTexture->m_resourceID = debugImage->m_resourceID;
	m_texture2Ds[debugTexture->m_resourceID] = debugTexture;
}


//-------------------------------------------------------------------------------------------------
void ResourceSystem::CreateDefaultFonts()
{
	CreateOrGetFont("Data/Font/Prototype.ttf");
}


//-------------------------------------------------------------------------------------------------
void ResourceSystem::CreateDefaultMaterials()
{
	CreateOrGetMaterial("Data/Material/default.material");
	CreateOrGetMaterial("Data/Material/debug.material");
}


//-------------------------------------------------------------------------------------------------
ResourceSystem::~ResourceSystem()
{
}
