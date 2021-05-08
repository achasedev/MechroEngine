///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: May 3rd, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/Physics/RigidBody/RigidBody.h"
#include "Engine/Physics/RigidBody/RigidBodySpring.h"

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
RigidBodySpring::RigidBodySpring(const Vector3& connectionPointLs, RigidBody* otherBody, const Vector3& otherConnectionPointLs, float springConstant, float restLength)
	: m_connectionPointLs(connectionPointLs)
	, m_otherBody(otherBody)
	, m_otherConnectionPointLs(otherConnectionPointLs)
	, m_springConstant(springConstant)
	, m_restLength(restLength)
{
}


//-------------------------------------------------------------------------------------------------
void RigidBodySpring::GenerateAndAddForce(RigidBody* body, float deltaSeconds) const
{
	UNUSED(deltaSeconds);

	// Get the force direction, pointing from other to this body
	Vector3 connectionPointWs = body->transform->TransformPosition(m_connectionPointLs);
	Vector3 forceDir = connectionPointWs - m_otherBody->transform->position;
	float springLength = forceDir.SafeNormalize(forceDir);

	if (springLength > 0.f)
	{
		// Determine magnitude based on length and resting length
		float magnitude = (springLength - m_restLength) * m_springConstant;

		// Apply the force
		body->AddWorldForceAtLocalPoint(forceDir * -magnitude, m_connectionPointLs);
	}
}
