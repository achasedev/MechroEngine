///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 24th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Math/Vector3.h"

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
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class Particle
{
public:
	//-----Public Methods-----

	Particle() {}
	Particle(const Vector3& position, const Vector3& velocity, float inverseMass = 1.f, float damping = 0.999f, const Vector3& acceleration = Vector3::ZERO);

	void	Integrate(float deltaSeconds);
	void	ClearNetForce() { m_netForce = Vector3::ZERO; }

	void	SetAcceleration(const Vector3& acceleration) { m_acceleration = acceleration; }
	void	SetPosition(const Vector3& position) { m_position = position; }
	void	SetVelocity(const Vector3& velocity) { m_velocity = velocity; }
	void	AddForce(const Vector3& force) { m_netForce += force; }
	void	SetDamping(float damping) { m_damping = damping; }
	void	SetMass(float mass);
	void	SetInverseMass(float iMass);

	Vector3 GetPosition() const { return m_position; }
	Vector3 GetVelocity() const { return m_velocity; }
	Vector3 GetAcceleration() const { return m_acceleration; }
	float	GetDamping() const { return m_damping; }
	float	GetMass() const { return (m_iMass > 0.f ? (1.f / m_iMass) : FLT_MAX); }
	float	GetInverseMass() const { return m_iMass; }


private:
	//-----Private Data-----

	Vector3 m_position		= Vector3::ZERO;
	Vector3 m_velocity		= Vector3::ZERO;
	Vector3 m_acceleration	= Vector3(0.f, -10.f, 0.f); // Default for gravity
	Vector3 m_netForce		= Vector3::ZERO;
	float	m_damping		= 0.999f; // Reduce energy in the system
	float	m_iMass			= 1.f;

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
