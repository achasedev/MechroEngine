///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: October 18th, 2021
/// Description:
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Render/Light.h"
#include "Engine/Render/RenderContext.h"
#include "Engine/Render/Texture/Texture2D.h"
#include "Engine/Render/Texture/TextureCube.h"
#include "Engine/Render/View/DepthStencilView.h"
#include "Engine/Render/View/RenderTargetView.h"

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
Light::Light()
{
	for (int i = 0; i < 6; ++i)
	{
		m_shadowTextures[i] = nullptr;
	}
}


//-------------------------------------------------------------------------------------------------
// Destroys the shadow texture if it was using one
Light::~Light()
{
	for (Texture2D* texture : m_shadowTextures)
	{
		SAFE_DELETE(texture);
	}
}


//-------------------------------------------------------------------------------------------------
// Sets the position of the light to the one given
void Light::SetPosition(const Vector3& position)
{
	m_lightData.m_position = position;
}


//-------------------------------------------------------------------------------------------------
// Sets the entire light data to the data given
void Light::SetLightData(const LightData& data)
{
	m_lightData = data;
}


//-------------------------------------------------------------------------------------------------
// Sets whether this light should generate shadows
void Light::SetIsShadowCasting(bool castsShadows)
{
	if (castsShadows)
	{
		if (m_shadowTextures[0] == nullptr)
		{
			int numToCreate = (IsPointLight() ? 6 : 1);
			for (int i = 0; i < numToCreate; ++i)
			{
				m_shadowTextures[i] = new Texture2D();
				m_shadowTextures[i]->CreateWithNoData(SHADOW_TEXTURE_SIZE, SHADOW_TEXTURE_SIZE, TEXTURE_FORMAT_R24G8_TYPELESS, TEXTURE_USAGE_DEPTH_STENCIL_BIT | TEXTURE_USAGE_SHADER_RESOURCE_BIT, GPU_MEMORY_USAGE_GPU);
			}
		}

		// Indicate we will cast shadows to the shader
		m_lightData.m_castsShadows = 1.0f;
	}
	else
	{
		// Clean up
		for (int i = 0; i < 6; ++i)
		{
			SAFE_DELETE(m_shadowTextures[i]);
		}

		m_lightData.m_castsShadows = 0.f;
	}
}


//-------------------------------------------------------------------------------------------------
// Returns the shadow texture itself
Texture2D* Light::GetShadowTexture(int index) const
{
	return m_shadowTextures[index];
}


//-------------------------------------------------------------------------------------------------
// Returns the view of the target used for writing depth
DepthStencilView* Light::GetShadowDepthStencilView(int index) const
{
	return m_shadowTextures[index]->CreateOrGetDepthStencilView();
}


//-------------------------------------------------------------------------------------------------
// Given a position, calculates this light's intensity perceived at that position (distance and attenuation)
float Light::CalculateIntensityForPosition(const Vector3& position) const
{
	float distance = (position - m_lightData.m_position).GetLength();
	float attenuation = 1.f / (m_lightData.m_attenuation.x + m_lightData.m_attenuation.y * distance + m_lightData.m_attenuation.z * distance * distance);

	return m_lightData.m_color.w * attenuation;
}


//-------------------------------------------------------------------------------------------------
// Constructs and returns a point light (position that shines light in all directions)
Light* Light::CreatePointLight(const Vector3& position, const Rgba& color /*= Rgba::WHITE*/, const Vector3& attenuation /*= Vector3(1.f, 0.f, 0.f)*/)
{
	Light* light = new Light();

	light->m_lightData.m_position = position;
	light->m_lightData.m_color = color.GetAsFloats();
	light->m_lightData.m_attenuation = attenuation;

	// Don't need dots since this isn't a spot light, and directional factor = 1.f indicates this is a point light
	light->m_lightData.m_dotOuterAngle = -1.f;
	light->m_lightData.m_dotInnerAngle = -1.f;
	light->m_lightData.m_directionFactor = 1.f;

	// Light direction is unused, as point lights emit light in all directions
	light->m_lightData.m_lightDirection = Vector3::ZERO;

	return light;
}


//-------------------------------------------------------------------------------------------------
// Constructs and returns a directional light (plane where all rays shine parallel from)
Light* Light::CreateDirectionalLight(const Vector3& position, const Vector3& direction /*= Vector3::MINUS_Y_AXIS*/, const Rgba& color /*= Rgba::WHITE*/, const Vector3& attenuation /*= Vector3(1.f, 0.f, 0.f)*/)
{
	Light* light = new Light();

	light->m_lightData.m_position = position;
	light->m_lightData.m_lightDirection = direction;
	light->m_lightData.m_color = color.GetAsFloats();
	light->m_lightData.m_attenuation = attenuation;

	// Don't need dots since this isn't a spot light, and directional factor = 0.f indicates this is a directional light
	light->m_lightData.m_dotOuterAngle = -2.f;
	light->m_lightData.m_dotInnerAngle = -1.f;
	light->m_lightData.m_directionFactor = 0.f;

	return light;
}


//-------------------------------------------------------------------------------------------------
// Constructs and returns a cone (spot?) light (light shines in a cone pattern from position in direction)
Light* Light::CreateConeLight(const Vector3& position, const Vector3& direction, float outerAngle, float innerAngle, const Rgba& color /*= Rgba::WHITE*/, const Vector3& attenuation /*= Vector3(1.f, 0.f, 0.f)*/)
{
	Light* light = new Light();

	light->m_lightData.m_position = position;
	light->m_lightData.m_lightDirection = direction;
	light->m_lightData.m_color = color.GetAsFloats();
	light->m_lightData.m_attenuation = attenuation;

	// Need dots since this is a spot light, and directional factor = 1.f indicates this is a point light
	light->m_lightData.m_dotOuterAngle = CosDegrees(outerAngle * 0.5f);
	light->m_lightData.m_dotInnerAngle = CosDegrees(innerAngle * 0.5f);
	light->m_lightData.m_directionFactor = 1.0f;

	return light;
}
