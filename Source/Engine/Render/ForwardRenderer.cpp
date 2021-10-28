///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: October 19th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Render/Camera.h"
#include "Engine/Render/DrawCall.h"
#include "Engine/Render/ForwardRenderer.h"
#include "Engine/Render/Light.h"
#include "Engine/Render/Material/Material.h"
#include "Engine/Render/Renderable.h"
#include "Engine/Render/RenderContext.h"
#include "Engine/Render/RenderScene.h"
#include "Engine/Render/Skybox.h"
#include "Engine/Resource/ResourceSystem.h"

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
// Renders the given RenderScene
void ForwardRenderer::Render(RenderScene* scene)
{
	scene->SortCameras();
	ConstructDrawCalls(scene);

	// Ensure depth stencils are not bound before they're used as a render target
	g_renderContext->BindShaderResourceView(4, nullptr);

	int numCameras = (int)scene->m_cameras.size();
	for (int index = 0; index < numCameras; ++index)
	{
		// Render shadow textures
		CreateShadowTexturesForCamera(scene, scene->m_cameras[index]);

		// Draw draw draw
		RenderSceneForCamera(scene, scene->m_cameras[index], false);
	}
}


//-------------------------------------------------------------------------------------------------
// Renders the depth textures for all lights for the given camera
void ForwardRenderer::CreateShadowTexturesForCamera(RenderScene* scene, Camera* camera)
{
	int numLights = (int)scene->m_lights.size();

	for (int lightIndex = 0; lightIndex < numLights; ++lightIndex)
	{
		Light* light = scene->m_lights[lightIndex];
		if (light->IsShadowCasting())
		{
			Camera shadowCamera = Camera();
			Vector3 cameraPos = camera->GetPosition();

			if (light->GetLightData().m_directionFactor > 0.f)
			{
				LightData data = light->GetLightData();
				shadowCamera.SetCameraMatrix(Matrix4::MakeLookAt(light->GetLightData().m_position - 10.f * (cameraPos - light->GetLightData().m_position).GetNormalized(), cameraPos));
				shadowCamera.SetProjectionPerspective(90.f, 0.1f, 100.0f);
			}
			else
			{
				// Directional
				shadowCamera.SetCameraMatrix(Matrix4::MakeLookAt(light->GetLightData().m_position, cameraPos, Vector3::X_AXIS));
				shadowCamera.SetProjectionOrthographic(Vector2(-100.f), Vector2(100.f), 0.1f, 100.f);
			}

			// Set the view projection to be used for the shadow test
			LightData data = light->GetLightData();
			data.m_shadowVP = shadowCamera.GetProjectionMatrix() * shadowCamera.GetViewMatrix();
			light->SetLightData(data);

			shadowCamera.SetRenderTarget(nullptr, false);
			shadowCamera.SetDepthTarget(light->GetShadowTexture(), false);

			RenderSceneForCamera(scene, &shadowCamera, true);
		}
	}
}


//-------------------------------------------------------------------------------------------------
// Renders the scene using the given camera; also used for shadow camera draws
void ForwardRenderer::RenderSceneForCamera(RenderScene* scene, Camera* camera, bool depthOnlyPass)
{
	g_renderContext->BeginCamera(camera);
	camera->ClearDepthTarget(1.0f);

	// Iterate over all draw calls (already sorted) and draw them
	for (int drawIndex = 0; drawIndex < (int)m_drawCalls.size(); ++drawIndex)
	{
		DrawCall& dc = m_drawCalls[drawIndex];

		if (depthOnlyPass)
		{
			// Use the shadowmap shader
			Shader* shadowShader = g_resourceSystem->CreateOrGetShader("Data/Shader/shadowmap.shader");

			Material shadowMat;
			shadowMat.SetShader(shadowShader);

			// Cache off the existing material, swap to the shadow material
			Material* prevMaterial = dc.GetMaterial();
			dc.SetMaterial(&shadowMat);

			g_renderContext->Draw(dc);

			// Revert it back
			dc.SetMaterial(prevMaterial);
		}
		else
		{
			g_renderContext->Draw(dc);
		}
	}

	if (!depthOnlyPass)
	{
		Skybox* skybox = scene->GetSkybox();

		if (skybox != nullptr)
		{
			skybox->Render();
		}
	}

	g_renderContext->EndCamera();
}


//-------------------------------------------------------------------------------------------------
// Constructs all the draw calls for rendering; should only need to be called one per frame(?)
void ForwardRenderer::ConstructDrawCalls(RenderScene* scene)
{
	m_drawCalls.clear();

	std::map<EntityID, Renderable>::const_iterator itr = scene->m_renderables.begin();
	for (itr; itr != scene->m_renderables.end(); itr++)
	{
		const Renderable& currRenderable = itr->second;
		ConstructDrawCallsForRenderable(currRenderable, scene);
	}

	// Sort the draw calls by their shader's layer and queue order
	SortDrawCalls();
}


//-------------------------------------------------------------------------------------------------
// Constructs the draw calls needed for the given renderable
void ForwardRenderer::ConstructDrawCallsForRenderable(const Renderable& renderable, RenderScene* scene)
{
	int drawCount = (int)renderable.GetNumDrawCalls();

	for (int dcIndex = 0; dcIndex < drawCount; ++dcIndex)
	{
		DrawCall dc;

		// Compute which lights contribute the most to this renderable
		Material* material = renderable.GetDraw(dcIndex).m_material;
		if (material->UsesLights())
		{
			Vector3 renderablePosition = renderable.GetModelMatrix().GetTVector().xyz();
			ComputeLightsForDrawCall(dc, scene, renderablePosition);
		}

		dc.SetFromRenderable(renderable, dcIndex);

		// Add the draw call to the list to render
		m_drawCalls.push_back(dc);	
	}
}


//-------------------------------------------------------------------------------------------------
// Sorts the draw calls in the order that they will be executed
void ForwardRenderer::SortDrawCalls()
{
	int numDrawCalls = (int)m_drawCalls.size();

	bool done = false;

	while (!done)
	{
		done = true;
		for (int i = 0; i < numDrawCalls - 1; ++i)
		{
			// Find one pair out of order, swap and continue iterating
			if (m_drawCalls[i].GetSortOrder() > m_drawCalls[i + 1].GetSortOrder())
			{
				DrawCall temp = m_drawCalls[i];
				m_drawCalls[i] = m_drawCalls[i + 1];
				m_drawCalls[i + 1] = temp;

				done = false;
			}
		}
	}
}


//-------------------------------------------------------------------------------------------------
// Determines the 8 most contributing lights for this object and sets the light data on the draw call
void ForwardRenderer::ComputeLightsForDrawCall(DrawCall& drawCall, RenderScene* scene, const Vector3& position)
{
	// Set the ambience
	drawCall.SetAmbience(scene->GetAmbience());

	// Calculate all other lights
	int totalLights = (int)scene->m_lights.size();

	// Calculate all intensities, store in an array parallel to the light array
	std::vector<float> intensities;
	for (int lightIndex = 0; lightIndex < totalLights; ++lightIndex)
	{
		float currIntensity = scene->m_lights[lightIndex]->CalculateIntensityForPosition(position);
		intensities.push_back(currIntensity);
	}

	// Sort the two parallel arrays by decreasing intensity
	for (int i = 0; i < totalLights - 1; ++i)
	{
		float maxValue = intensities[i];
		int maxIndex = i;

		for (int j = i + 1; j < totalLights; ++j)
		{
			if (intensities[j] > intensities[maxIndex])
			{
				maxIndex = j;
				maxValue = intensities[j];
			}
		}

		// If we find a max in the remainder of the list that isn't at the start,
		// swap it to the front
		if (maxIndex != i)
		{
			float tempValue = intensities[i];
			Light* tempLight = scene->m_lights[i];

			intensities[i] = intensities[maxIndex];
			scene->m_lights[i] = scene->m_lights[maxIndex];

			intensities[maxIndex] = tempValue;
			scene->m_lights[maxIndex] = tempLight;
		}
	}

	// Light list is sorted in descending order by intensity for this position
	// Just need to add the correct number of lights into the drawCall
	int numLightsToUse = Min(totalLights, MAX_NUMBER_OF_LIGHTS);

	for (int lightIndex = 0; lightIndex < numLightsToUse; ++lightIndex)
	{
		drawCall.SetLight(lightIndex, scene->m_lights[lightIndex]);
	}

	drawCall.SetNumLightsInUse(numLightsToUse);
}
