///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: October 18th, 2021
/// Description: Class to represent a single light in a scene
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/Rgba.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Matrix4.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#define MAX_NUMBER_OF_LIGHTS 8 // Max number of lights that can be used when rendering a single renderable; a scene can have more lights than this

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Texture2D;

//-------------------------------------------------------------------------------------------------
// Data to be sent to GPU for a single light
struct LightData
{
	Vector3 m_position = Vector3::ZERO;
	float	m_dotOuterAngle = 0.f;

	Vector3 m_lightDirection = Vector3::ZERO;
	float	m_dotInnerAngle = 0.f;

	Vector3 m_attenuation = Vector3(1.f, 0.f, 0.f);
	float	m_directionFactor = 0.f;

	Vector4 m_color = Vector4(0.f, 0.f, 0.f, 0.f);

	Matrix4 m_shadowVP;

	Vector3 m_padding0 = Vector3::ZERO;
	float	m_castsShadows = 0.f;
};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class Light
{
public:
	//-----Public Methods-----

	~Light();

	// Mutators
	void		SetPosition(const Vector3& position);
	void		SetLightData(const LightData& data);
	void		SetIsShadowCasting(bool castsShadows);

	LightData	GetLightData() const { return m_lightData; }
	bool		IsShadowCasting() const { return m_lightData.m_castsShadows > 0.f; }
	Texture2D*	GetShadowTexture() const { return m_shadowTexture; }
	float		CalculateIntensityForPosition(const Vector3& position) const;

	// Statics
	static Light* CreatePointLight(const Vector3& position, const Rgba& color = Rgba::WHITE, const Vector3& attenuation = Vector3(1.f, 0.f, 0.f));
	static Light* CreateDirectionalLight(const Vector3& position, const Vector3& direction = Vector3::MINUS_Y_AXIS, const Rgba& color = Rgba::WHITE, const Vector3& attenuation = Vector3(1.f, 0.f, 0.f));
	static Light* CreateConeLight(const Vector3& position, const Vector3& direction, float outerAngle, float innerAngle, const Rgba& color = Rgba::WHITE, const Vector3& attenuation = Vector3(1.f, 0.f, 0.f));


private:
	//-----Private Data-----

	LightData	m_lightData;
	Texture2D*	m_shadowTexture = nullptr;

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
