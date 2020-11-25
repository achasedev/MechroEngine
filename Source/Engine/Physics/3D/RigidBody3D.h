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
#include "Engine/Math/Transform.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#define INVALID_RIGIDBODY_ID -1

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class GameObject;
class PhysicsScene2D;
class Polygon2D;
class Transform;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class RigidBody2D
{
	friend class Arbiter2D;
	friend class PhysicsScene2D;

public:
	//-----Public Methods-----

	// Mutators
	void				SetPosition(const Vector2& position) { m_transform->position = Vector3(position, 0.f); }
	void				SetRotationDegrees(float rotationDegrees) { m_transform->SetRotation(Vector3(0.f, 0.f, rotationDegrees)); }
	void				SetVelocity(const Vector2& velocity) { m_velocityWs = velocity; }
	void				SetAngularVelocity(float angularVelocityDegrees) { m_angularVelocityDegrees = angularVelocityDegrees; }
	void				SetMassProperties(float mass);
	void				SetFriction(float friction) { m_friction = friction; }
	void				SetAffectedByGravity(bool affected) { m_affectedByGravity = affected; }

	// Accessors
	GameObject*			GetGameObject() const { return m_gameObj; }
	PhysicsScene2D*		GetScene() const { return m_scene; }
	Vector2				GetCenterOfMassLs() const { return m_centerOfMassLs; }
	Vector2				GetVelocity() const { return m_velocityWs; }
	float				GetAngularVelocity() const { return m_angularVelocityDegrees; }
	float				GetFriction() const { return m_friction; }
	float				GetMass() const { return m_mass; }
	float				GetInverseMass() const { return m_invMass; }
	float				GetInertia() const { return m_inertia; }
	float				GetInverseInertia() const { return m_invInertia; }
	float				GetDensity() const { return m_density; }
	Vector2				GetForce() const { return m_forceWs; }
	float				GetTorque() const { return m_torque; }
	const Polygon2D*	GetLocalShape() const { return m_shapeLs; } // Const because you shouldn't be changing this >.>
	void				GetWorldShape(Polygon2D& out_polygon) const;
	bool				IsAffectedByGravity() const { return m_affectedByGravity; }

	// Producers
	bool				IsStatic() const { return m_invMass == 0.f; }
	Vector2				GetCenterOfMassWs() const;


private:
	//-----Private Methods-----

	// Only PhysicsScene2D can create/destroy these
	RigidBody2D(PhysicsScene2D* owner, GameObject* gameObject);
	~RigidBody2D();

	void				CalculateCenterOfMass();


private:
	//-----Private Data-----
	
	// Misc
	GameObject*			m_gameObj					= nullptr;
	PhysicsScene3D*		m_scene						= nullptr;

	// Positional
	Transform*			m_transform;
	Vector3				m_centerOfMassLs			= Vector3::ZERO;

	// Velocity
	Vector3				m_velocityWs				= Vector3::ZERO;
	Vector3				m_angularVelocityDegrees	= 0.f;

	// Mass + friction
	float				m_friction					= 0.2f;
	float				m_mass						= FLT_MAX;
	float				m_invMass					= 0.f; // For static bodies, invMass == 0
	float				m_inertia					= FLT_MAX;
	float				m_invInertia				= 0.f; // For static bodies, invI == 0
	float				m_density					= FLT_MAX;

	// Forces
	Vector2				m_forceWs					= Vector2::ZERO;
	float				m_torque					= 0.f;
	bool				m_affectedByGravity			= true;

	// Shape
	const Polygon2D*	m_shapeLs					= nullptr;

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
