///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: October 19th, 2021
/// Description: Class to control a forward rendering path
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Render/DrawCall.h"
#include "Engine/Render/RenderScene.h"
#include <vector>

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Camera;
class Renderable;
class Texture2D;
class Texture2DArray;
class TextureCubeArray;
class Vector3;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class ForwardRenderer
{
public:
	//-----Public Methods-----

	ForwardRenderer();
	~ForwardRenderer();

	void Render(RenderScene* scene);


private:
	//-----Private Methods-----

	void CreateShadowTexturesForCamera(RenderScene* scene, Camera* camera);
	void PerformShadowDepthPass(Camera* camera);
	void PerformRenderPass(RenderScene* scene, Camera* camera);
	void ConstructDrawCalls(RenderScene* scene);
	void ConstructDrawCallsForRenderable(const Renderable& renderable, const RenderOptions& options, RenderScene* scene);
	void SortDrawCalls();
	void ComputeLightsForDrawCall(DrawCall& drawCall, RenderScene* scene, const Vector3& position);
	void PopulateShadowMapArray(const DrawCall& dc);

private:
	//-----Private Data-----

	std::vector<std::pair<DrawCall, RenderOptions>>	m_drawCalls;
	Texture2DArray*									m_coneDirShadowMaps = nullptr;
	TextureCubeArray*								m_pointShadowMaps = nullptr;

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
