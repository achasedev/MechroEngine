///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 17th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Render/Buffer/RenderBuffer.h"
#include "Engine/Render/View/TextureView.h"
#include "Engine/Utility/StringID.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Font;
class Image;
class Material;
class Mesh;
class Shader;
class Texture2D;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class ResourceSystem
{
public:
	//-----Public Methods-----

	static void	Initialize();
	static void	Shutdown();

	Mesh*		CreateOrGetMesh(const char* filepath);
	Image*		CreateOrGetImage(const char* filepath);
	Shader*		CreateOrGetShader(const char* filepath);
	Material*	CreateOrGetMaterial(const char* filepath);
	Texture2D*	CreateOrGetTexture2D(const char* filepath, TextureUsageBits textureUsage = TEXTURE_USAGE_SHADER_RESOURCE_BIT, GPUMemoryUsage memoryUsage = GPU_MEMORY_USAGE_STATIC);
	Font*		CreateOrGetFont(const char* filepath);


private:
	//-----Private Methods-----

	ResourceSystem();
	~ResourceSystem();
	ResourceSystem(const ResourceSystem& other) = delete;

	void CreateBuiltInAssets();
		void CreateDefaultMeshes();
		void CreateDefaultShaders();
		void CreateDefaultImages();
		void CreateDefaultMaterials();
		void CreateDefaultTexture2Ds();
		void CreateDefaultFonts();


private:
	//-----Private Data-----

	std::map<StringID, Texture2D*>	m_texture2Ds;
	std::map<StringID, Image*>		m_images;
	std::map<StringID, Material*>	m_materials;
	std::map<StringID, Shader*>		m_shaders;
	std::map<StringID, Mesh*>		m_meshes;
	std::map<StringID, Font*>		m_fonts;

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
