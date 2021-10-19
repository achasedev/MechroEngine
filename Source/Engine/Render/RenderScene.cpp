///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: October 18th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/DevConsole.h"
#include "Engine/Core/EngineCommon.h"
#include "Engine/Render/Camera.h"
#include "Engine/Render/Renderable.h"
#include "Engine/Render/RenderScene.h"

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
// Constructor
RenderScene::RenderScene(const std::string& name)
	: m_name(name)
{
}


//-------------------------------------------------------------------------------------------------
// Adds the renderable to the scene
void RenderScene::AddRenderable(Renderable* renderable)
{
	ASSERT_RETURN(renderable != nullptr, NO_RETURN_VAL, "Renderable was nullptr!");
	ASSERT_RETURN(!DoesRenderableExist(renderable), NO_RETURN_VAL, "Renderable already exists in the scene!");
	ASSERT_RETURN(renderable->IsReadyForDrawing(), NO_RETURN_VAL, "Renderable isn't complete!");

	m_renderables.push_back(renderable);
}


//-------------------------------------------------------------------------------------------------
// Adds the light to the scene
void RenderScene::AddLight(Light* light)
{
	ASSERT_RETURN(!DoesLightExist(light), NO_RETURN_VAL, "Light already exists in the scene!");
	m_lights.push_back(light);
}


//-------------------------------------------------------------------------------------------------
// Adds the camera to the scene
void RenderScene::AddCamera(Camera* camera)
{
	ASSERT_RETURN(!DoesCameraExist(camera), NO_RETURN_VAL, "Camera already exists in the scene!");
	m_cameras.push_back(camera);
}


//-------------------------------------------------------------------------------------------------
// Removes the renderable from the scene, does not delete it
void RenderScene::RemoveRenderable(Renderable* toRemove)
{
	int numRenderables = (int)m_renderables.size();
	for (int i = 0; i < numRenderables; ++i)
	{
		if (m_renderables[i] == toRemove)
		{
			m_renderables.erase(m_renderables.begin() + i);
			return;
		}
	}

	ConsoleLogWarningf("Tried to remove a renderable but it doesn't exist!");
}


//-------------------------------------------------------------------------------------------------
// Removes the light from the scene, does not delete it
void RenderScene::RemoveLight(Light* toRemove)
{
	int numLights = (int)m_lights.size();
	for (int i = 0; i < numLights; ++i)
	{
		if (m_lights[i] == toRemove)
		{
			m_lights.erase(m_lights.begin() + i);
			return;
		}
	}

	ConsoleLogWarningf("Tried to remove a light but it doesn't exist!");
}


//-------------------------------------------------------------------------------------------------
// Removes a camera from the scene, does not delete it
void RenderScene::RemoveCamera(Camera* toRemove)
{
	int numCameras = (int)m_cameras.size();
	for (int i = 0; i < numCameras; ++i)
	{
		if (m_cameras[i] == toRemove)
		{
			m_cameras.erase(m_cameras.begin() + i);
		}
	}

	ConsoleLogWarningf("Tried to remove a camera but it doesn't exist!");
}


//-------------------------------------------------------------------------------------------------
// Clears the entire scene of all render objects
void RenderScene::Clear()
{
	m_renderables.clear();
	m_lights.clear();
	m_renderables.clear();
}


//-------------------------------------------------------------------------------------------------
// Sorts the cameras in their draw order (bubble sort)
void RenderScene::SortCameras()
{
	int numCameras = GetCameraCount();

	for (int i = 0; i < numCameras - 1; ++i)
	{
		int minIndex = i;
		int minOrder = m_cameras[minIndex]->GetDrawOrder();

		for (int j = i + 1; j < numCameras; ++j)
		{
			if (m_cameras[j]->GetDrawOrder() < minOrder)
			{
				minIndex = j;
				minOrder = m_cameras[minIndex]->GetDrawOrder();
			}
		}

		// A camera after first has an earlier draw order, so replace
		if (minIndex != i)
		{
			Camera* temp = m_cameras[i];
			m_cameras[i] = m_cameras[minIndex];
			m_cameras[minIndex] = temp;
		}
	}
}


//-------------------------------------------------------------------------------------------------
// Returns true if the renderable is currently in the scene
bool RenderScene::DoesRenderableExist(Renderable* renderable) const
{
	int numRenderables = (int)m_renderables.size();
	for (int i = 0; i < numRenderables; ++i)
	{
		if (m_renderables[i] == renderable)
		{
			return true;
		}
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
// Returns true if the light is currently in the scene
bool RenderScene::DoesLightExist(Light* light) const
{
	int numLights = (int)m_lights.size();
	for (int i = 0; i < numLights; ++i)
	{
		if (m_lights[i] == light)
		{
			return true;
		}
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
// Returns true if the camera is currently in the scene
bool RenderScene::DoesCameraExist(Camera* camera) const
{
	int numCameras = (int)m_cameras.size();
	for (int i = 0; i < numCameras; ++i)
	{
		if (m_cameras[i] == camera)
		{
			return true;
		}
	}

	return false;
}
