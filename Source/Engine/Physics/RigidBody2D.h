///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: September 17th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Math/Vector2.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Polygon2D;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class RigidBody2D
{
public:
	//-----Public Methods-----

	// Mutators
	void SetPosition(const Vector2& position) { m_position = position; }
	void SetRotationDegrees(float rotationDegrees) { m_rotationDegrees = rotationDegrees; }
	void SetVelocity(const Vector2& velocity) { m_velocity = velocity; }
	void SetAngularVelocity(float angularVelocityDegrees) { m_angularVelocityDegrees = angularVelocityDegrees; }
	void SetFriction(float friction) { m_friction = friction; }
	void SetMass(float mass);

	// Accessors
	Vector2				GetPosition() const { return m_position; }
	float				GetRotationDegrees() const { return m_rotationDegrees; }
	Vector2				GetVelocity() const { return m_velocity; }
	float				GetAngularVelocity() const { return m_angularVelocityDegrees; }
	float				GetFriction() const { return m_friction; }
	float				GetMass() const { return m_mass; }
	float				GetInverseMass() const { return m_invMass; }
	float				GetInertia() const { return m_inertia; }
	float				GetInverseInertia() const { return m_invInertia; }
	float				GetDensity() const { return m_density; }
	Vector2				GetForce() const { return m_force; }
	float				GetTorque() const { return m_torque; }
	const Polygon2D*	GetShape() const { return m_shape; } // Const because you shouldn't be changing this >.>

	// Producers
	bool	IsStatic() const { return m_invMass == 0.f; }


private:
	//-----Private Methods-----

	void CalculateMassProperties(float mass);


private:
	//-----Private Data-----
	
	// Positional
	Vector2		m_position					= Vector2::ZERO;
	float		m_rotationDegrees			= 0.f;
	Vector2		m_centerOfMass				= Vector2::ZERO;

	// Velocity
	Vector2		m_velocity					= Vector2::ZERO;
	float		m_angularVelocityDegrees	= 0.f;

	// Mass + friction
	float		m_friction					= 0.2f;
	float		m_mass						= FLT_MAX;
	float		m_invMass					= 0.f; // For static bodies, invMass == 0
	float		m_inertia					= FLT_MAX;
	float		m_invInertia				= 0.f; // For static bodies, invI == 0
	float		m_density					= FLT_MAX;

	// Forces
	Vector2		m_force						= Vector2::ZERO;
	float		m_torque					= 0.f;

	// Shape
	Polygon2D*	m_shape						= nullptr;

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
