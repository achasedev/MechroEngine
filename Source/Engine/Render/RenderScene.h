///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: October 18th, 2021
/// Description: Class to organize a collection of objects involved in rendering (Lights, renderables, cameras, etc)
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/Rgba.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Renderable;
class Light;
class Camera;
class TextureCube;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class RenderScene
{

public:
	//-----Public Methods-----

	RenderScene(const std::string& name);
	~RenderScene() {}

	// List mutators
	void			AddRenderable(Renderable* renderable);
	void			AddLight(Light* light);
	void			AddCamera(Camera* camera);

	void			RemoveRenderable(Renderable* toRemove);
	void			RemoveLight(Light* toRemove);
	void			RemoveCamera(Camera* toRemove);
	void			Clear();

	void			SortCameras();

	void			SetSkybox(TextureCube* skybox) { m_skybox = skybox; }
	void			SetAmbience(const Rgba& ambience) { m_ambience = ambience; }

	Rgba			GetAmbience() const { return m_ambience; }
	int				GetLightCount() const { return (int)m_lights.size(); }
	int				GetRenderableCount() const { (int)m_renderables.size(); }
	int				GetCameraCount() const { return (int)m_renderables.size(); }
	TextureCube*	GetSkybox() const { return m_skybox; }


private:
	//-----Private Methods-----

	RenderScene(const RenderScene& copy) = delete;

	bool			DoesRenderableExist(Renderable* renderable) const;
	bool			DoesLightExist(Light* light) const;
	bool			DoesCameraExist(Camera* camera) const;


private:
	//-----Private Data-----

	std::string					m_name;
	std::vector<Renderable*>	m_renderables;
	std::vector<Light*>			m_lights;
	std::vector<Camera*>		m_cameras;

	Rgba						m_ambience = Rgba::WHITE;
	TextureCube*				m_skybox = nullptr;

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
