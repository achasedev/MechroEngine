///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: September 17th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Math/Vector2.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#define MAX_CONTACT_POINTS 2

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class RigidBody2D;

//-------------------------------------------------------------------------------------------------
struct Contact2D
{
	Contact2D() {}

	Vector2 m_position = Vector2::ZERO;
	Vector2 m_normal = Vector2::ZERO;

	Vector2 m_r1 = Vector2::ZERO;	// From the center of mass of body 1 to the contact
	Vector2 m_r2 = Vector2::ZERO;	// From the center of mass of body 2 to the contact

	float m_separation = 0.f;
	float m_pN = 0.f;	// accumulated normal impulse
	float m_pT = 0.f;	// accumulated tangent impulse
	float m_pNB = 0.f;	// accumulated normal impulse for position bias
	float m_massNormal = 0.f;
	float m_massTangent = 0.f;
	float m_bias = 0.f;
	//EdgePair m_edgePairs;
};


//-------------------------------------------------------------------------------------------------
struct ArbiterKey2D
{
	ArbiterKey2D(RigidBody2D* b1, RigidBody2D* b2)
	{
		// Sort then in order of memory
		if (b1 < b2)
		{
			m_body1 = b1; 
			m_body2 = b2;
		}
		else
		{
			m_body1 = b2; 
			m_body2 = b1;
		}
	}

	// Compare memory addresses of the bodies
	inline bool operator<(const ArbiterKey2D& other)
	{
		if (m_body1 < other.m_body1)
			return true;

		if (m_body1 == other.m_body1 && m_body2 < other.m_body2)
			return true;

		return false;
	}

	RigidBody2D* m_body1;
	RigidBody2D* m_body2;
};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class Arbiter2D
{
public:
	//-----Public Methods-----

	Arbiter2D(RigidBody2D* body1, RigidBody2D* body2);

	void	Collide();

	// Accessors
	int		GetNumContacts() const { return m_numContacts; }
	float	GetFriction() const { return m_friction; }


private:
	//-----Private Data-----

	void CalculateContactPoints();


private:
	//-----Private Data-----

	RigidBody2D*	m_body1 = nullptr;
	RigidBody2D*	m_body2 = nullptr;

	Contact2D		m_contacts[MAX_CONTACT_POINTS];
	int				m_numContacts = -1;
	float			m_friction = -1.0f; // Combined Frictions

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
