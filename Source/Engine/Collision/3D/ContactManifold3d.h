///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 18th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Collision/3D/CollisionUtils3d.h"
#include "Engine/Math/Vector3.h"
#include <map>

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
struct ManifoldKey3D
{
	ManifoldKey3D(Collider3d* a, Collider3d* b)
	{
		// Sort then in order of memory
		if (a < b)
		{
			m_colliderA = a;
			m_colliderB = b;
		}
		else
		{
			m_colliderA = b;
			m_colliderB = a;
		}
	}

	// Compare memory addresses of the bodies
	bool operator<(const ManifoldKey3D& other) const
	{
		if (m_colliderA < other.m_colliderA)
			return true;

		if (m_colliderA == other.m_colliderA && m_colliderB < other.m_colliderB)
			return true;

		return false;
	}

	Collider3d* m_colliderA;
	Collider3d* m_colliderB;
};

//-------------------------------------------------------------------------------------------------
struct ContactPoint3D
{
	ContactPoint3D() {}

	Vector3 m_originalPosition = Vector3::ZERO; // For debugging clipping
	Vector3 m_position = Vector3::ZERO;
	Vector3 m_normal = Vector3::ZERO;
};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class ContactManifold3d
{
public:
	//-----Public Methods-----

	ContactManifold3d() {}
	ContactManifold3d(Collider3d* a, Collider3d* b);

	void					Collide();

	bool					HasCollision() const { return m_broadphaseResult.m_collisionFound; }
	int						GetNumContacts() const { return m_numContacts; }
	const ContactPoint3D*	GetContacts() const { return m_contacts; }
	BroadphaseResult3d		GetBroadphaseResult() const { return m_broadphaseResult; }


private:
	//-----Private Static Data-----

	static constexpr int	MAX_CONTACTS = 8;


private:
	//-----Private Member Data-----

	Collider3d*				m_colA = nullptr;
	Collider3d*				m_colB = nullptr;
	BroadphaseResult3d		m_broadphaseResult;

	ContactPoint3D			m_contacts[MAX_CONTACTS];
	int						m_numContacts = 0;

};

typedef std::map<ManifoldKey3D, ContactManifold3d>::iterator Manifold3DIter;


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
