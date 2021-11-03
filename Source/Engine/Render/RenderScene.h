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
#include "Engine/Core/Entity.h"
#include "Engine/Render/Renderable.h"
#include <map>

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Camera;
class Light;
class Skybox;

struct RenderOptions
{
	bool m_castsShadows = true; // If true, this renderable will be drawn in the depth-only pass for shadows
	bool m_shouldBeRendered = true; // If true, will be drawn in the render pass. Useful for when I want something to cast a shadow but not render (player)
};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class RenderScene
{
	friend class ForwardRenderer;

public:
	//-----Public Methods-----

	RenderScene(const std::string& name);
	~RenderScene() {}

	// List mutators
	void			AddRenderable(EntityID id, const Renderable& renderable, const RenderOptions& options = RenderOptions());
	void			AddLight(Light* light);
	void			AddCamera(Camera* camera);

	void			RemoveRenderable(EntityID entityId);
	void			RemoveLight(Light* toRemove);
	void			RemoveCamera(Camera* toRemove);
	void			Clear();

	void			SortCameras();

	void			SetSkybox(Skybox* skybox) { m_skybox = skybox; }
	void			SetAmbience(const Rgba& ambience) { m_ambience = ambience; }

	Rgba			GetAmbience() const { return m_ambience; }
	int				GetLightCount() const { return (int)m_lights.size(); }
	int				GetRenderableCount() const { (int)m_renderables.size(); }
	int				GetCameraCount() const { return (int)m_cameras.size(); }
	Skybox*			GetSkybox() const { return m_skybox; }
	Renderable*		GetRenderable(EntityID entityId);


private:
	//-----Private Methods-----

	RenderScene(const RenderScene& copy) = delete;

	bool			DoesRenderableExist(EntityID entityId) const;
	bool			DoesLightExist(Light* light) const;
	bool			DoesCameraExist(Camera* camera) const;


private:
	//-----Private Data-----

	std::string													m_name;
	std::map<EntityID, std::pair<Renderable, RenderOptions>>	m_renderables;
	std::vector<Light*>											m_lights;
	std::vector<Camera*>										m_cameras;

	Rgba														m_ambience = Rgba::WHITE;
	Skybox*														m_skybox = nullptr;

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
