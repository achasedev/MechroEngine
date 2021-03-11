///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 25th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Math/Vector3.h"
#include "Engine/Physics/3D/Collision3D.h"
#include <map>

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Arbiter3D;
class RigidBody3D;

//-------------------------------------------------------------------------------------------------
struct ArbiterKey3D
{
	ArbiterKey3D(RigidBody3D* b1, RigidBody3D* b2)
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
	bool operator<(const ArbiterKey3D& other) const
	{
		if (m_body1 < other.m_body1)
			return true;

		if (m_body1 == other.m_body1 && m_body2 < other.m_body2)
			return true;

		return false;
	}

	RigidBody3D* m_body1;
	RigidBody3D* m_body2;
};
typedef std::map<ArbiterKey3D, Arbiter3D>::iterator Arb3DIter;


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class Arbiter3D
{
public:
	//-----Public Methods-----

	Arbiter3D() {}
	Arbiter3D(RigidBody3D* body1, RigidBody3D* body2);

	void					Update(const Contact3D* newContacts, uint32 numNewContacts);
	void					DetectCollision();
	void					PreStep(float deltaSeconds);
	void					ApplyImpulse();

	// Accessors
	int						GetNumContacts() const { return m_numContacts; }
	const Contact3D*		GetContacts() const { return m_contacts; }
	float					GetFriction() const { return m_friction; }
	CollisionSeparation3d	GetSeparation() const { return m_separation; }

private:
	//-----Private Methods-----

	void					CalculateContactPoints(const Polygon3D* poly1, const Polygon3D* poly2, const CollisionSeparation3d& separation);


private:
	//-----Private Static Data-----

	static const float		ALLOWED_PENETRATION;
	static const float		BIAS_FACTOR;
	static const bool		ACCUMULATE_IMPULSES;
	static const bool		WARM_START_ACCUMULATIONS;
	static constexpr int	MAX_CONTACTS = 8;


private:
	//-----Private Member Data-----

	RigidBody3D*			m_body1 = nullptr;
	RigidBody3D*			m_body2 = nullptr;
	CollisionSeparation3d	m_separation;

	Contact3D				m_contacts[MAX_CONTACTS];
	int						m_numContacts = 0;
	float					m_friction = -1.0f; // Combined Frictions

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
