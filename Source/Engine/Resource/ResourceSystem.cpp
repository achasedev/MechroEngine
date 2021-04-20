///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 17th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/DevConsole.h"
#include "Engine/Framework/EngineCommon.h"
#include "Engine/IO/Image.h"
#include "Engine/Render/Material.h"
#include "Engine/Render/Mesh/MeshBuilder.h"
#include "Engine/Render/Shader.h"
#include "Engine/Render/Texture/Texture2D.h"
#include "Engine/Resource/ResourceSystem.h"

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

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------
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
}


//-------------------------------------------------------------------------------------------------
void ResourceSystem::Shutdown()
{
	ASSERT_OR_DIE(g_resourceSystem != nullptr, "ResourceSystem not initialized!");
	SAFE_DELETE(g_resourceSystem);
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

	Mesh* mesh = mb.CreateMesh<Vertex3D_PCU>();
	m_meshes[SID("unit_cube")] = mesh;
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
	m_images[SID("white")] = whiteImage;

	CreateOrGetImage("Data/Image/debug.png");
}


//-------------------------------------------------------------------------------------------------
void ResourceSystem::CreateDefaultTexture2Ds()
{
	Image* whiteImage = CreateOrGetImage("white");
	Texture2D* whiteTexture = new Texture2D();
	whiteTexture->CreateFromImage(*whiteImage, TEXTURE_USAGE_SHADER_RESOURCE_BIT, GPU_MEMORY_USAGE_STATIC);
	whiteTexture->m_resourceID = SID("white");
	m_texture2Ds[SID("white")] = whiteTexture;
}


//-------------------------------------------------------------------------------------------------
void ResourceSystem::CreateDefaultMaterials()
{
	CreateOrGetMaterial("Data/Material/default.material");
}


//-------------------------------------------------------------------------------------------------
ResourceSystem::~ResourceSystem()
{
}
