///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 25th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Framework/Entity.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Transform.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#define INVALID_RIGIDBODY_ID -1

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Entity;
class Material;
class Polygon3d;
class PolytopeCollider3d;
class Rgba;
class Transform;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class RigidBody3D
{
	friend class PhysicsSystem3D;

public:
	//-----Public Methods-----

	void				DebugRender(Material* material, const Rgba& color);

	// Mutators
	void				AddForce(const Vector3& forceWs) { m_forceWs += forceWs; }
	void				AddTorque(const Vector3& torque) { m_torque += torque; }

	void				SetPosition(const Vector3& position) { m_owner->transform.position = position; }
	void				SetRotationDegrees(float rotationDegrees) { m_owner->transform.SetRotation(Vector3(rotationDegrees)); }
	void				SetVelocity(const Vector3& velocity) { m_velocityWs = velocity; }
	void				SetAngularVelocity(float angularVelocityDegrees) { m_angularVelocityDegrees = Vector3(angularVelocityDegrees); }
	void				SetMassProperties(float mass);
	void				SetFriction(float friction) { m_friction = friction; }
	void				SetAffectedByGravity(bool affected) { m_affectedByGravity = affected; }

	// Accessors
	Entity*				GetOwningEntity() const { return m_owner; }
	Vector3				GetCenterOfMassLs() const { return m_centerOfMassLs; }
	Vector3				GetVelocity() const { return m_velocityWs; }
	Vector3				GetAngularVelocity() const { return m_angularVelocityDegrees; }
	float				GetFriction() const { return m_friction; }
	float				GetMass() const { return m_mass; }
	float				GetInverseMass() const { return m_invMass; }
	Vector3				GetInertia() const { return m_inertia; }
	Vector3				GetInverseInertia() const { return m_invInertia; }
	float				GetDensity() const { return m_density; }
	Vector3				GetForce() const { return m_forceWs; }
	Vector3				GetTorque() const { return m_torque; }
	const Polygon3d*	GetLocalShape() const;
	const Polygon3d*	GetWorldShape(Polygon3d& out_polygon) const;
	bool				IsAffectedByGravity() const { return m_affectedByGravity; }

	// Producers
	bool				IsStatic() const { return m_invMass == 0.f; }
	Vector3				GetCenterOfMassWs() const;


private:
	//-----Private Methods-----

	// Only PhysicsSystem3D can create/destroy these
	RigidBody3D() {}
	~RigidBody3D() {}

	void				CalculateCenterOfMass();


private:
	//-----Private Data-----
	
	// Misc
	Entity*				m_owner						= nullptr;
	Transform*			m_transform					= nullptr; // Owner's transform, just for convenience
	// Positional
	Vector3				m_centerOfMassLs			= Vector3::ZERO;

	// Velocity
	Vector3				m_velocityWs				= Vector3::ZERO;
	Vector3				m_angularVelocityDegrees	= Vector3::ZERO;

	// Mass + friction
	float				m_friction					= 0.2f;
	float				m_mass						= FLT_MAX;
	float				m_invMass					= 0.f; // For static bodies, invMass == 0
	Vector3				m_inertia					= Vector3(FLT_MAX);
	Vector3				m_invInertia				= Vector3(0.f); // For static bodies, invI == 0
	float				m_density					= FLT_MAX;

	// Forces
	Vector3				m_forceWs					= Vector3::ZERO;
	Vector3				m_torque					= Vector3::ZERO;
	bool				m_affectedByGravity			= true;

	// Shape - to be replaced with possibly a shape instead
	const PolytopeCollider3d* m_collider			= nullptr;

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
