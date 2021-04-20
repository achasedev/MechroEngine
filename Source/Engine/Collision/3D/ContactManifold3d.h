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
struct ContactPointID
{
	bool operator==(const ContactPointID& other) const { return m_poly == other.m_poly && m_vertexIndex == other.m_vertexIndex; }
	const Polygon3d*	m_poly = nullptr;
	int					m_vertexIndex = -1;
};


//-------------------------------------------------------------------------------------------------
struct ContactPoint3D
{
	ContactPoint3D() {}

	Vector3 m_position = Vector3::ZERO;
	Vector3 m_normal = Vector3::ZERO;

	Vector3 m_r1 = Vector3::ZERO;	// From the center of mass of body 1 to the contact
	Vector3 m_r2 = Vector3::ZERO;	// From the center of mass of body 2 to the contact

	float m_pen = 0.f;
	float m_massNormal = 0.f;
	float m_massTangent = 0.f;
	float m_massBitangent = 0.f;
	float m_bias = 0.f;

	float m_accNormalImpulse = 0.f;		// accumulated normal impulse
	float m_accTangentImpulse = 0.f;	// accumulated tangent impulse
	float m_accBitangentImpulse = 0.f;	// accumulated bitangent impulse
	float m_normalBiasImpulse = 0.f;	// accumulated normal impulse for position bias

	ContactPointID m_id;
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
	void					GenerateContacts();
	void					UpdateContacts(const ContactPoint3D* contacts, int numContacts);

	void					DebugRender() const;

	bool					HasCollision() const { return m_broadphaseResult.m_collisionFound; }
	int						GetNumContacts() const { return m_numContacts; }
	ContactPoint3D*			GetContacts() { return m_contacts; }
	ContactPoint3D			GetContact(int contactIndex) { return m_contacts[contactIndex]; }
	BroadphaseResult3d		GetBroadphaseResult() const { return m_broadphaseResult; }
	const Collider3d*		GetColliderA() const { return m_colA; }
	const Collider3d*		GetColliderB() const { return m_colB; }
	Entity*					GetEntityA() const { return m_colA->GetOwningEntity(); }
	Entity*					GetEntityB() const { return m_colB->GetOwningEntity(); }
	Entity*					GetReferenceEntity() const;
	Entity*					GetIncidentEntity() const;


public:
	//-----Public Static Data-----

	static constexpr int	MAX_CONTACTS = 8;


private:
	//-----Private Member Data-----

	Collider3d*				m_colA = nullptr;
	Collider3d*				m_colB = nullptr;
	BroadphaseResult3d		m_broadphaseResult;

	ContactPoint3D			m_contacts[MAX_CONTACTS];
	int						m_numContacts = 0;

};

typedef std::map<ManifoldKey3D, ContactManifold3d>::iterator Manifold3dIter;


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
