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
#include "Engine/Render/Texture/Texture2D.h"
#include "Engine/Render/Texture/Texture2DArray.h"
#include "Engine/Render/Texture/TextureCube.h"
#include "Engine/Resource/ResourceSystem.h"
#include "Engine/Render/View/DepthStencilView.h"
#include "Engine/Render/View/ShaderResourceView.h"

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
ForwardRenderer::ForwardRenderer()
{
	m_shadowMaps = new Texture2DArray();
	m_shadowMaps->Create(MAX_NUMBER_OF_LIGHTS, SHADOW_TEXTURE_SIZE, SHADOW_TEXTURE_SIZE, TEXTURE_FORMAT_R24G8_TYPELESS);

	m_pointLightMap = new TextureCube();
	m_pointLightMap->CreateWithNoData(SHADOW_TEXTURE_SIZE, SHADOW_TEXTURE_SIZE, TEXTURE_FORMAT_R24G8_TYPELESS, TEXTURE_USAGE_SHADER_RESOURCE_BIT, GPU_MEMORY_USAGE_GPU);

	m_clearDepthTexture = new Texture2D();
	m_clearDepthTexture->CreateWithNoData(SHADOW_TEXTURE_SIZE, SHADOW_TEXTURE_SIZE, TEXTURE_FORMAT_R24G8_TYPELESS, TEXTURE_USAGE_SHADER_RESOURCE_BIT | TEXTURE_USAGE_DEPTH_STENCIL_BIT, GPU_MEMORY_USAGE_GPU);

	// Code reuse!
	Camera camera;
	camera.SetDepthStencilView(m_clearDepthTexture->CreateOrGetDepthStencilView());
	camera.ClearDepthTarget(1.0f);
}


//-------------------------------------------------------------------------------------------------
// Destructor
ForwardRenderer::~ForwardRenderer()
{
	SAFE_DELETE(m_clearDepthTexture);
	SAFE_DELETE(m_pointLightMap);
	SAFE_DELETE(m_shadowMaps);
}


//-------------------------------------------------------------------------------------------------
// Renders the given RenderScene
void ForwardRenderer::Render(RenderScene* scene)
{
	scene->SortCameras();
	ConstructDrawCalls(scene);

	// Ensure depth stencils are not bound before they're used as a render target
	g_renderContext->BindShaderResourceView(SRV_SLOT_SHADOWMAP, nullptr);
	g_renderContext->BindShaderResourceView(SRV_SLOT_POINT_LIGHT_SHADOWMAP, nullptr);

	int numCameras = (int)scene->m_cameras.size();
	for (int index = 0; index < numCameras; ++index)
	{
		// Render shadow textures
		CreateShadowTexturesForCamera(scene, scene->m_cameras[index]);

		// Draw draw draw
		PerformRenderPass(scene, scene->m_cameras[index]);
	}
}


//-------------------------------------------------------------------------------------------------
// Sets up the camera to render a shadowmap for a cone light
static void InitializeCameraForConeLight(Camera* shadowCamera, Light* light, Camera* gameCamera)
{
	LightData lightData = light->GetLightData();
	Vector3 reference = AreMostlyEqual(lightData.m_lightDirection, Vector3::Y_AXIS) ? Vector3::X_AXIS : Vector3::Y_AXIS;
	Matrix4 cameraMatrix = Matrix4::MakeLookAt(lightData.m_position, lightData.m_position + lightData.m_lightDirection, reference);

	shadowCamera->SetCameraMatrix(cameraMatrix);
	shadowCamera->SetProjectionPerspective(2.f * ACosDegrees(lightData.m_dotOuterAngle), light->GetShadowDepthStencilView(0)->GetAspect(), 0.1f, 100.f);
	shadowCamera->SetDepthStencilView(light->GetShadowDepthStencilView(0));

	lightData.m_shadowModel = shadowCamera->GetCameraMatrix();
	lightData.m_shadowView = shadowCamera->GetViewMatrix();
	lightData.m_shadowProjection = shadowCamera->GetProjectionMatrix();

	light->SetLightData(lightData);
}


//-------------------------------------------------------------------------------------------------
// Sets up the camera to render a shadowmap for a point light in a single direction
static void InitializeCameraForPointLight(Camera* shadowCamera, Light* light, Camera* gameCamera, int pointLightDirectionIndex)
{
	LightData lightData = light->GetLightData();

	Matrix4 cameraModel;
	switch (pointLightDirectionIndex)
	{
	case 0: cameraModel = Matrix4(Vector3::MINUS_Z_AXIS, Vector3::Y_AXIS, Vector3::X_AXIS, lightData.m_position); break;
	case 1: cameraModel = Matrix4(Vector3::Z_AXIS, Vector3::Y_AXIS, Vector3::MINUS_X_AXIS, lightData.m_position); break;
	case 2: cameraModel = Matrix4(Vector3::X_AXIS, Vector3::MINUS_Z_AXIS, Vector3::Y_AXIS, lightData.m_position); break;
	case 3: cameraModel = Matrix4(Vector3::X_AXIS, Vector3::Z_AXIS, Vector3::MINUS_Y_AXIS, lightData.m_position); break;
	case 4: cameraModel = Matrix4(Vector3::X_AXIS, Vector3::Y_AXIS, Vector3::Z_AXIS, lightData.m_position); break;
	case 5: cameraModel = Matrix4(Vector3::MINUS_X_AXIS, Vector3::Y_AXIS, Vector3::MINUS_Z_AXIS, lightData.m_position); break;
	default:
		break;
	}

	shadowCamera->SetCameraMatrix(cameraModel);
	shadowCamera->SetProjectionPerspective(90.f, 1.f, 0.1f, 100.f);
	shadowCamera->SetDepthStencilView(light->GetShadowDepthStencilView(pointLightDirectionIndex));

	if (pointLightDirectionIndex == 4)
	{
		lightData.m_shadowModel = shadowCamera->GetCameraMatrix();
		lightData.m_shadowView = shadowCamera->GetViewMatrix();
		lightData.m_shadowProjection = shadowCamera->GetProjectionMatrix();
	}

	light->SetLightData(lightData);
}


//-------------------------------------------------------------------------------------------------
// Sets up the camera to render a shadowmap for a directional light
static void InitializeCameraForDirectionalLight(Camera* shadowCamera, Light* light, Camera* gameCamera)
{
	Frustrum frustrum = gameCamera->GetFrustrum();
	LightData lightData = light->GetLightData();
	Vector3 reference = AreMostlyEqual(Abs(DotProduct(lightData.m_lightDirection, Vector3::Y_AXIS)), 1.0f) ? Vector3::X_AXIS : Vector3::Y_AXIS;
	Matrix4 lightModel = Matrix4::MakeLookAt(lightData.m_position, lightData.m_position + lightData.m_lightDirection, reference);
	Matrix4 invLightModel = Matrix4::GetInverse(lightModel);

	// Get frustrum in light space
	Vector3 frustrumPointsLs[8];
	for (int i = 0; i < 8; ++i)
	{
		frustrumPointsLs[i] = invLightModel.TransformPosition(frustrum.GetPoint(i));
	}

	// Find the AABB bounds to encapsulate the frustrum, in light space
	Vector3 minsLs;
	minsLs.x = Min(frustrumPointsLs[0].x, frustrumPointsLs[1].x, frustrumPointsLs[2].x, frustrumPointsLs[3].x, frustrumPointsLs[4].x, frustrumPointsLs[5].x, frustrumPointsLs[6].x, frustrumPointsLs[7].x);
	minsLs.y = Min(frustrumPointsLs[0].y, frustrumPointsLs[1].y, frustrumPointsLs[2].y, frustrumPointsLs[3].y, frustrumPointsLs[4].y, frustrumPointsLs[5].y, frustrumPointsLs[6].y, frustrumPointsLs[7].y);
	minsLs.z = Min(frustrumPointsLs[0].z, frustrumPointsLs[1].z, frustrumPointsLs[2].z, frustrumPointsLs[3].z, frustrumPointsLs[4].z, frustrumPointsLs[5].z, frustrumPointsLs[6].z, frustrumPointsLs[7].z);

	Vector3 maxsLs;
	maxsLs.x = Max(frustrumPointsLs[0].x, frustrumPointsLs[1].x, frustrumPointsLs[2].x, frustrumPointsLs[3].x, frustrumPointsLs[4].x, frustrumPointsLs[5].x, frustrumPointsLs[6].x, frustrumPointsLs[7].x);
	maxsLs.y = Max(frustrumPointsLs[0].y, frustrumPointsLs[1].y, frustrumPointsLs[2].y, frustrumPointsLs[3].y, frustrumPointsLs[4].y, frustrumPointsLs[5].y, frustrumPointsLs[6].y, frustrumPointsLs[7].y);
	maxsLs.z = Max(frustrumPointsLs[0].z, frustrumPointsLs[1].z, frustrumPointsLs[2].z, frustrumPointsLs[3].z, frustrumPointsLs[4].z, frustrumPointsLs[5].z, frustrumPointsLs[6].z, frustrumPointsLs[7].z);

	// Place the camera at the back of the AABB, in light space
	Vector3 shadowCameraPosLs = Vector3(0.5f * (minsLs.x + maxsLs.x), 0.5f * (minsLs.y + maxsLs.y), minsLs.z);
	Vector3 shadowCameraPosWs = lightModel.TransformPosition(shadowCameraPosLs);

	// Determine orthobounds to represent the AABB at this location
	Vector2 orthoBottomLeft = Vector2(minsLs.x - shadowCameraPosLs.x, minsLs.y - shadowCameraPosLs.y);
	Vector2 orthoTopRight = Vector2(maxsLs.x - shadowCameraPosLs.x, maxsLs.y - shadowCameraPosLs.y);

	// Make the projection
	Matrix4 orthoProj = Matrix4::MakeOrtho(orthoBottomLeft, orthoTopRight, 0.f, maxsLs.z - minsLs.z);

	// Find the model to place the camera at this location in world space
	Matrix4 cameraModel = Matrix4::MakeLookAt(shadowCameraPosWs, shadowCameraPosWs + lightData.m_lightDirection, reference);

	shadowCamera->SetCameraMatrix(cameraModel);
	shadowCamera->SetProjection(CAMERA_PROJECTION_ORTHOGRAPHIC, orthoProj);
	shadowCamera->SetDepthStencilView(light->GetShadowDepthStencilView(0));

	lightData.m_shadowModel = shadowCamera->GetCameraMatrix();
	lightData.m_shadowView = shadowCamera->GetViewMatrix();
	lightData.m_shadowProjection = shadowCamera->GetProjectionMatrix();

	light->SetLightData(lightData);
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
			Camera shadowCamera;

			if (light->IsConeLight())
			{
				InitializeCameraForConeLight(&shadowCamera, light, camera);
				PerformShadowDepthPass(&shadowCamera);

			}
			else if (light->IsDirectionalLight())
			{
				InitializeCameraForDirectionalLight(&shadowCamera, light, camera);
				PerformShadowDepthPass(&shadowCamera);
			}
			else if (light->IsPointLight())
			{
				// For point lights, render 6 directions to make a shadow cube
				for (int i = 0; i < 6; ++i)
				{
					InitializeCameraForPointLight(&shadowCamera, light, camera, i);
					PerformShadowDepthPass(&shadowCamera);
				}
			}
			else
			{
				ERROR_AND_DIE("Light doesn't match any classification!");
			}
		}
	}
}


//-------------------------------------------------------------------------------------------------
// Renders the scene for the given shadow camera
void ForwardRenderer::PerformShadowDepthPass(Camera* shadowCamera)
{
	g_renderContext->BeginCamera(shadowCamera);
	shadowCamera->ClearDepthTarget(1.0f);

	// Iterate over all draw calls (already sorted) and draw them
	for (int drawIndex = 0; drawIndex < (int)m_drawCalls.size(); ++drawIndex)
	{
		DrawCall& dc = m_drawCalls[drawIndex];

		// Use the shadowmap shader
		Material* depthMat = g_resourceSystem->CreateOrGetMaterial("Data/Material/depth_only.material");

		// Cache off the existing material, swap to the shadow material
		Material* prevMaterial = dc.GetMaterial();
		dc.SetMaterial(depthMat);

		g_renderContext->Draw(dc);

		// Revert it back
		dc.SetMaterial(prevMaterial);

	}

	g_renderContext->EndCamera();
}


//-------------------------------------------------------------------------------------------------
// Renders the scene using the given camera
void ForwardRenderer::PerformRenderPass(RenderScene* scene, Camera* camera)
{
	g_renderContext->BeginCamera(camera);
	camera->ClearDepthTarget(1.0f);

	// Iterate over all draw calls (already sorted) and draw them
	for (int drawIndex = 0; drawIndex < (int)m_drawCalls.size(); ++drawIndex)
	{
		DrawCall& dc = m_drawCalls[drawIndex];

		PopulateShadowMapArray(dc);
		g_renderContext->Draw(dc);
	}

	// Render skybox last
	Skybox* skybox = scene->GetSkybox();

	if (skybox != nullptr)
	{
		skybox->Render();
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
		dc.SetShadowMaps(m_shadowMaps, m_pointLightMap);

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


//-------------------------------------------------------------------------------------------------
// Copies the lights' shadowmaps into the array used by the shader for shadow depth tests
void ForwardRenderer::PopulateShadowMapArray(const DrawCall& dc)
{
	ID3D11DeviceContext* dxContext = g_renderContext->GetDxContext();
	ID3D11Resource* dxTexArray = m_shadowMaps->GetDxHandle();

	int numLights = dc.GetNumLights();
	for (int i = 0; i < numLights; ++i)
	{
		if (dc.GetLight(i)->IsShadowCasting())
		{
			if (dc.GetLight(i)->IsPointLight())
			{
				for (int mapIndex = 0; mapIndex < 6; ++mapIndex)
				{
					Texture2D* shadowMap = dc.GetLight(i)->GetShadowTexture(mapIndex);
					ID3D11Resource* dxShadowTexture = shadowMap->GetDxHandle();
					ID3D11Resource* dxCubeMap = m_pointLightMap->GetDxHandle();

					dxContext->CopySubresourceRegion(dxCubeMap, mapIndex, 0, 0, 0, dxShadowTexture, 0, nullptr);
				}
			}
			else
			{
				Texture* shadowTexture = dc.GetLight(i)->GetShadowTexture(0);
				ID3D11Resource* dxShadowTexture = shadowTexture->GetDxHandle();

				dxContext->CopySubresourceRegion(dxTexArray, i, 0, 0, 0, dxShadowTexture, 0, nullptr);
			}
		}
	}
}
