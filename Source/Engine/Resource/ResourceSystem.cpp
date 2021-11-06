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
#include "Engine/Render/Material/Material.h"
#include "Engine/Render/Mesh/MeshBuilder.h"
#include "Engine/Render/Shader/Shader.h"
#include "Engine/Render/Texture/Texture2D.h"
#include "Engine/Render/Texture/TextureCube.h"
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
typedef std::map<StringID, TextureCube*>::iterator TextureCubeIter;
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
static const char* INVALID_SHADER = "Data/Shader/invalid.shader";
static const char* INVALID_MATERIAL = "Data/Material/invalid.material";
static const char* INVALID_TEXTURE = "Data/Image/invalid.png";

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
		ConsoleLogErrorf("Could not load Image %s", filepath);
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
		shader->Clear();
		shader->Load(INVALID_SHADER);
		ConsoleLogErrorf("Could not load Shader %s", filepath);
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
		material->Clear();
		material->Load(INVALID_MATERIAL);
		ConsoleLogErrorf("Could not load Material %s", filepath);
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
		texture->Clear();
		texture->Load(INVALID_TEXTURE, TEXTURE_USAGE_SHADER_RESOURCE_BIT, GPU_MEMORY_USAGE_STATIC);
		ConsoleLogErrorf("Could not load Texture2D %s", filepath);
	}
	
	m_texture2Ds[id] = texture;

	return texture;
}


//-------------------------------------------------------------------------------------------------
TextureCube* ResourceSystem::CreateOrGetTextureCube(const char* folderpath)
{
	StringID id = SID(folderpath);
	TextureCubeIter itr = m_textureCubes.find(id);
	if (itr != m_textureCubes.end())
	{
		return itr->second;
	}

	TextureCube* texCube = new TextureCube();
	bool success = texCube->LoadSixFiles(folderpath);

	if (success)
	{
		texCube->m_resourceID = id;
	}
	else
	{
		SAFE_DELETE(texCube);
		ConsoleLogErrorf("Could not load Texture2D %s", folderpath);
	}

	m_textureCubes[id] = texCube;

	return texCube;
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
		ConsoleLogErrorf("Unsupported font format for file %s", filepath);
	}
	else if (error)
	{
		ConsoleLogErrorf("Couldn't load font file %s", filepath);
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
	CreateDefaultTextureCubes();
	CreateDefaultMaterials();
}


//-------------------------------------------------------------------------------------------------
void ResourceSystem::CreateDefaultMeshes()
{
	MeshBuilder mb;
	mb.BeginBuilding(TOPOLOGY_TRIANGLE_LIST, true);
	mb.PushCube(Vector3::ZERO, Vector3::ONES);
	mb.FinishBuilding();

	Mesh* cubeMesh = mb.CreateMesh<VertexLit>();
	cubeMesh->m_resourceID = SID("unit_cube");
	m_meshes[cubeMesh->m_resourceID] = cubeMesh;

	mb.Clear();
	mb.BeginBuilding(TOPOLOGY_TRIANGLE_LIST, true);
	mb.PushSphere(Vector3::ZERO, 1.f);
	mb.FinishBuilding();

	Mesh* sphereMesh = mb.CreateMesh<VertexLit>();
	sphereMesh->m_resourceID = SID("unit_sphere");
	m_meshes[sphereMesh->m_resourceID] = sphereMesh;

	mb.Clear();
	mb.BeginBuilding(TOPOLOGY_TRIANGLE_LIST, true);
	mb.PushTopHemiSphere(Vector3::ZERO, 1.f, Rgba::WHITE, 10, 5, (2.f / 3.f), 1.f);
	mb.FinishBuilding();

	Mesh* capsuleTop = mb.CreateMesh<VertexLit>();
	capsuleTop->m_resourceID = SID("capsule_top");
	m_meshes[capsuleTop->m_resourceID] = capsuleTop;

	mb.Clear();
	mb.BeginBuilding(TOPOLOGY_TRIANGLE_LIST, true);
	mb.PushBottomHemiSphere(Vector3::ZERO, 1.f, Rgba::WHITE, 10, 5, 0.f, (1.f / 3.f));
	mb.FinishBuilding();

	Mesh* capsuleBottom = mb.CreateMesh<VertexLit>();
	capsuleBottom->m_resourceID = SID("capsule_bottom");
	m_meshes[capsuleBottom->m_resourceID] = capsuleBottom;

	mb.Clear();
	mb.BeginBuilding(TOPOLOGY_TRIANGLE_LIST, true);
	mb.PushTube(Vector3(0.f, -0.5f, 0.f), Vector3(0.f, 0.5f, 0.f), 1.f, Rgba::WHITE, 10, (1.f / 3.f), (2.f / 3.f));
	mb.FinishBuilding();

	Mesh* capsuleMiddle = mb.CreateMesh<VertexLit>();
	capsuleMiddle->m_resourceID = SID("capsule_middle");
	m_meshes[capsuleMiddle->m_resourceID] = capsuleMiddle;

	mb.Clear();
	mb.BeginBuilding(TOPOLOGY_TRIANGLE_LIST, true);
	mb.PushQuad3D(Vector3(-0.5f, 0.f, -0.5f), Vector3(-0.5f, 0.f, 0.5f), Vector3(0.5f, 0.f, 0.5f), Vector3(0.5f, 0.f, -0.5f));
	mb.FinishBuilding();

	Mesh* horizontalQuad = mb.CreateMesh<VertexLit>();
	horizontalQuad->m_resourceID = SID("horizontal_quad");
	m_meshes[horizontalQuad->m_resourceID] = horizontalQuad;

	// Plane with normal in the +z direction, so bottom left is +x, -y
	mb.Clear();
	mb.BeginBuilding(TOPOLOGY_TRIANGLE_LIST, true);
	int numSteps = 20;
	float stepSize = 1.f / (float)numSteps;
	for (int i = 0; i < numSteps; ++i)
	{
		for (int j = 0; j < numSteps; ++j)
		{
			Vector3 bl = Vector3(0.5f, -0.5f, 0.f) + Vector3((float)i * -stepSize, (float)j * stepSize, 0.f);
			Vector3 tl = bl + Vector3(0.f, stepSize, 0.f);
			Vector3 tr = bl + Vector3(-stepSize, stepSize, 0.f);
			Vector3 br = bl + Vector3(-stepSize, 0.f, 0.f);

			mb.PushQuad3D(bl, tl, tr, br);
		}
	}
	mb.FinishBuilding();

	Mesh* plane = mb.CreateMesh<VertexLit>();
	plane->m_resourceID = SID("plane");
	m_meshes[plane->m_resourceID] = plane;

	mb.Clear();
	mb.BeginBuilding(TOPOLOGY_LINE_LIST, false);
	mb.PushLine3D(Vector3::ZERO, Vector3::X_AXIS, Rgba::RED);
	mb.PushLine3D(Vector3::ZERO, Vector3::Y_AXIS, Rgba::GREEN);
	mb.PushLine3D(Vector3::ZERO, Vector3::Z_AXIS, Rgba::BLUE);
	mb.FinishBuilding();

	Mesh* transformMesh = mb.CreateMesh<Vertex3D_PCU>();
	transformMesh->m_resourceID = SID("transform");
	m_meshes[transformMesh->m_resourceID] = transformMesh;

	mb.Clear();
	mb.BeginBuilding(TOPOLOGY_LINE_LIST, false);
	mb.PushLine3D(Vector3::MINUS_X_AXIS, Vector3::X_AXIS);
	mb.PushLine3D(Vector3::MINUS_Y_AXIS, Vector3::Y_AXIS);
	mb.PushLine3D(Vector3::MINUS_Z_AXIS, Vector3::Z_AXIS);
	mb.PushLine3D(Vector3(-1.f).GetNormalized(), Vector3::ONES.GetNormalized());
	mb.PushLine3D(Vector3(-1.f, 1.f, -1.f).GetNormalized(), Vector3(1.f, -1.f, 1.f).GetNormalized());
	mb.PushLine3D(Vector3(-1.f, -1.f, 1.f).GetNormalized(), Vector3(1.f, 1.f, -1.f).GetNormalized());
	mb.PushLine3D(Vector3(-1.f, 1.f, 1.f).GetNormalized(), Vector3(1.f, -1.f, -1.f).GetNormalized());
	mb.FinishBuilding();

	Mesh* pointMesh = mb.CreateMesh<Vertex3D_PCU>();
	pointMesh->m_resourceID = SID("point");
	m_meshes[pointMesh->m_resourceID] = pointMesh;

	mb.Clear();
	mb.BeginBuilding(TOPOLOGY_TRIANGLE_LIST, true);
	mb.PushCylinder(Vector3(0.f, -0.5f, 0.f), Vector3(0.f, 0.5f, 0.f), 1.f, Rgba::WHITE);
	mb.FinishBuilding();

	Mesh* cylinderMesh = mb.CreateMesh<VertexLit>();
	cylinderMesh->m_resourceID = SID("cylinder");
	m_meshes[cylinderMesh->m_resourceID] = cylinderMesh;
}


//-------------------------------------------------------------------------------------------------
void ResourceSystem::CreateDefaultShaders()
{
	Shader* invalidShader = CreateOrGetShader(INVALID_SHADER);
	ASSERT_OR_DIE(invalidShader != nullptr, "Couldn't load the invalid shader!");

	CreateOrGetShader("Data/Shader/default_opaque.shader");
	CreateOrGetShader("Data/Shader/default_alpha.shader");
	CreateOrGetShader("Data/Shader/font.shader");
	CreateOrGetShader("Data/Shader/skybox.shader");
}


//-------------------------------------------------------------------------------------------------
void ResourceSystem::CreateDefaultImages()
{
	Image* whiteImage = new Image(IntVector2(2, 2), Rgba::WHITE);
	whiteImage->m_resourceID = SID("white");
	m_images[whiteImage->m_resourceID] = whiteImage;

	Image* flatImage = new Image(IntVector2(2, 2), Rgba(127, 127, 255, 255));
	flatImage->m_resourceID = SID("flat");
	m_images[flatImage->m_resourceID] = flatImage;

	CreateOrGetImage("Data/Image/debug.png");
}


//-------------------------------------------------------------------------------------------------
void ResourceSystem::CreateDefaultTexture2Ds()
{
	Image* invalidImage = CreateOrGetImage(INVALID_TEXTURE);
	Texture2D* invalidTexture = new Texture2D();
	invalidTexture->CreateFromImage(*invalidImage, TEXTURE_FORMAT_R8G8B8A8_UNORM, TEXTURE_USAGE_SHADER_RESOURCE_BIT, GPU_MEMORY_USAGE_STATIC);
	invalidTexture->m_resourceID = invalidImage->m_resourceID;
	m_texture2Ds[invalidImage->m_resourceID] = invalidTexture;
	ASSERT_OR_DIE(invalidTexture != nullptr, "Couldn't load the invalid texture!");

	Image* whiteImage = CreateOrGetImage("white");
	Texture2D* whiteTexture = new Texture2D();
	whiteTexture->CreateFromImage(*whiteImage, TEXTURE_FORMAT_R8G8B8A8_UNORM, TEXTURE_USAGE_SHADER_RESOURCE_BIT, GPU_MEMORY_USAGE_STATIC);
	whiteTexture->m_resourceID = whiteImage->m_resourceID;
	m_texture2Ds[whiteTexture->m_resourceID] = whiteTexture;

	Image* flatImage = CreateOrGetImage("flat");
	Texture2D* flatTexture = new Texture2D();
	flatTexture->CreateFromImage(*flatImage, TEXTURE_FORMAT_R8G8B8A8_UNORM, TEXTURE_USAGE_SHADER_RESOURCE_BIT, GPU_MEMORY_USAGE_STATIC);
	flatTexture->m_resourceID = flatImage->m_resourceID;
	m_texture2Ds[flatTexture->m_resourceID] = flatTexture;

	Image* debugImage = CreateOrGetImage("Data/Image/debug.png");
	Texture2D* debugTexture = new Texture2D();
	debugTexture->CreateFromImage(*debugImage, TEXTURE_FORMAT_R8G8B8A8_UNORM, TEXTURE_USAGE_SHADER_RESOURCE_BIT, GPU_MEMORY_USAGE_STATIC);
	debugTexture->m_resourceID = debugImage->m_resourceID;
	m_texture2Ds[debugTexture->m_resourceID] = debugTexture;
}


//-------------------------------------------------------------------------------------------------
void ResourceSystem::CreateDefaultTextureCubes()
{
	TextureCube* skybox = new TextureCube();
	skybox->LoadSixFiles("Data/Image/Skybox/");
	skybox->m_resourceID = SID("Data/Image/Skybox/");
	m_textureCubes[skybox->m_resourceID] = skybox;
}


//-------------------------------------------------------------------------------------------------
void ResourceSystem::CreateDefaultFonts()
{
	CreateOrGetFont("Data/Font/Prototype.ttf");
}


//-------------------------------------------------------------------------------------------------
void ResourceSystem::CreateDefaultMaterials()
{
	Material* invalidMaterial = CreateOrGetMaterial(INVALID_MATERIAL);
	ASSERT_OR_DIE(invalidMaterial != nullptr, "Couldn't load the invalid material!");

	CreateOrGetMaterial("Data/Material/default.material");
	CreateOrGetMaterial("Data/Material/debug.material");
	CreateOrGetMaterial("Data/Material/skybox.material");
	CreateOrGetMaterial("Data/Material/normal_local.material");
	CreateOrGetMaterial("Data/Material/normal_world.material");
}


//-------------------------------------------------------------------------------------------------
ResourceSystem::~ResourceSystem()
{
	SafeDeleteMap<StringID, TextureCube>(m_textureCubes);
	SafeDeleteMap<StringID, Texture2D>(m_texture2Ds);
	SafeDeleteMap<StringID, Image>(m_images);
	SafeDeleteMap<StringID, Material>(m_materials);
	SafeDeleteMap<StringID, Shader>(m_shaders);
	SafeDeleteMap<StringID, Mesh>(m_meshes);
	SafeDeleteMap<StringID, Font>(m_fonts);
}
