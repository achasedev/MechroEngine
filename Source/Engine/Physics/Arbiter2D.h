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
#include "Engine/Physics/Collsion2D.h"
#include <map>

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#define MAX_CONTACT_POINTS 2

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Arbiter2D;
class RigidBody2D;

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
	bool operator<(const ArbiterKey2D& other) const
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
typedef std::map<ArbiterKey2D, Arbiter2D>::iterator ArbIter;


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

	Arbiter2D() {}
	Arbiter2D(RigidBody2D* body1, RigidBody2D* body2);

	void				Update(const Contact2D* newContacts, uint32 numNewContacts);
	void				DetectCollision();
	void				PreStep(float deltaSeconds);
	void				ApplyImpulse();

	// Accessors
	uint32				GetNumContacts() const { return m_numContacts; }
	const Contact2D*	GetContacts() const { return m_contacts; }
	float				GetFriction() const { return m_friction; }


private:
	//-----Private Methods-----

	void				CalculateContactPoints(const Polygon2D* poly1, const Polygon2D* poly2, const CollisionSeparation2D& separation);


private:
	//-----Private Static Data-----

	static const float	ALLOWED_PENETRATION;
	static const float	BIAS_FACTOR;
	static const bool	ACCUMULATE_IMPULSES;
	static const bool	WARM_START_ACCUMULATIONS;


private:
	//-----Private Member Data-----

	RigidBody2D*		m_body1 = nullptr;
	RigidBody2D*		m_body2 = nullptr;

	Contact2D			m_contacts[MAX_CONTACT_POINTS];
	uint32				m_numContacts = 0;
	float				m_friction = -1.0f; // Combined Frictions

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
