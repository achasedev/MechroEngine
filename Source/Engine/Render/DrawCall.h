///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 15th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Math/Matrix4.h"
#include "Engine/Core/EngineCommon.h"
#include "Engine/Render/Light.h"
#include "Engine/Render/Shader/Shader.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Mesh;
class Material;
class Renderable;
class Texture2DArray;
class TextureCube;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class DrawCall
{
public:
	//-----Public Methods-----

	DrawCall();

	void				SetFromRenderable(const Renderable& renderable, uint32 drawCallIndex);
	void				SetAmbience(const Rgba& ambience) { m_ambience = ambience; }
	void				SetNumLightsInUse(int numLights) { m_numLightsInUse = numLights; }
	void				SetLight(int lightIndex, Light* light);
	void				SetMaterial(Material* material) { m_material = material; }
	void				SetMesh(Mesh* mesh) { m_mesh = mesh; }
	void				SetModelMatrix(const Matrix4& model) { m_modelMatrix = model; }
	void				SetShadowMaps(Texture2DArray* shadowMaps, TextureCube* pointLightShadowMaps);
	Mesh*				GetMesh() const { return m_mesh; }
	Material*			GetMaterial() const { return m_material; }
	Matrix4				GetModelMatrix() const { return m_modelMatrix; }
	int					GetSortOrder() const;
	const Light*		GetLight(int index) const { return m_lights[index]; }
	int					GetNumLights() const { return m_numLightsInUse; }
	Rgba				GetAmbience() const { return m_ambience; }
	Texture2DArray*		GetShadowMaps() const { return m_shadowMaps; }
	TextureCube*		GetPointLightShadowMaps() const { return m_pointLightShadowMaps; }


private:
	//-----Private Data-----

	Mesh*				m_mesh = nullptr;
	Material*			m_material = nullptr;
	Matrix4				m_modelMatrix;

	// For sorting i	n the ForwardRenderer
	int					m_renderLayer = 0;
	RenderQueue			m_renderQueue = RENDER_QUEUE_OPAQUE;

	// Lights
	Rgba				m_ambience = Rgba::WHITE;
	int					m_numLightsInUse = 0;
	Light*				m_lights[MAX_NUMBER_OF_LIGHTS];
	Texture2DArray*		m_shadowMaps = nullptr; // Set by the ForwardRenderer
	TextureCube*		m_pointLightShadowMaps = nullptr; // Set by the ForwardRenderer

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
