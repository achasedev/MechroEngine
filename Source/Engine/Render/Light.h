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

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Texture2D;

//-------------------------------------------------------------------------------------------------
// Data to be sent to GPU for a single light
struct LightData
{
	Vector3 m_position;
	float m_dotOuterAngle;

	Vector3 m_lightDirection;
	float m_dotInnerAngle;

	Vector3 m_attenuation;
	float m_directionFactor;

	Vector4 m_color;

	Matrix4 m_shadowVP;

	Vector3 m_padding0;
	float m_castsShadows = 0.f;
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
	bool		IsShadowCasting() const { return m_isShadowCasting; }
	Texture2D*	GetShadowTexture() const { return m_shadowTexture; }
	float		CalculateIntensityForPosition(const Vector3& position) const;

	// Statics
	static Light* CreatePointLight(const Vector3& position, const Rgba& color = Rgba::WHITE, const Vector3& attenuation = Vector3(1.f, 0.f, 0.f));
	static Light* CreateDirectionalLight(const Vector3& position, const Vector3& direction = Vector3::MINUS_Y_AXIS, const Rgba& color = Rgba::WHITE, const Vector3& attenuation = Vector3(1.f, 0.f, 0.f));
	static Light* CreateConeLight(const Vector3& position, const Vector3& direction, float outerAngle, float innerAngle, const Rgba& color = Rgba::WHITE, const Vector3& attenuation = Vector3(1.f, 0.f, 0.f));


private:
	//-----Private Data-----

	LightData	m_lightData;
	bool		m_isShadowCasting = false;
	Texture2D*	m_shadowTexture = nullptr;

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
