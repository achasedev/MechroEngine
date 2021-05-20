///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: May 2nd, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Math/Matrix3.h"
#include "Engine/Math/Transform.h"

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
class RigidBody
{
	friend class PhysicsScene;

public:
	//-----Public Methods-----

	RigidBody(Transform* transform);

	void CalculateDerivedData();
	void Integrate(float deltaSeconds);
	
	void AddWorldForce(const Vector3& forceWs);
	void AddLocalForce(const Vector3& forceLs);
	void AddWorldForceAtWorldPoint(const Vector3& forceWs, const Vector3& pointWs);
	void AddWorldForceAtLocalPoint(const Vector3& forceWs, const Vector3& pointLs);
	void AddLocalForceAtLocalPoint(const Vector3& forceLs, const Vector3& pointLs);
	void AddLocalForceAtWorldPoint(const Vector3& forceLs, const Vector3& pointWs);
	void AddWorldVelocity(const Vector3& velocity) { m_velocityWs += velocity; }
	void AddWorldAngularVelocityRadians(const Vector3& angularVelocityRadians) { m_angularVelocityRadiansWs += angularVelocityRadians; }

	void SetVelocityWs(const Vector3& velocityWs) { m_velocityWs = velocityWs; }
	void SetAngularVelocityRadiansWs(const Vector3& angularVelocityRadiansWs) { m_angularVelocityRadiansWs = angularVelocityRadiansWs; }
	void SetAcceleration(const Vector3& acceleration) { m_accelerationWs = acceleration; }
	void SetInverseMass(float iMass) { m_iMass = iMass; }
	void SetLinearDamping(float linearDamping) { m_linearDamping = linearDamping; }
	void SetAngularDamping(float angularDamping) { m_angularDamping = angularDamping; }
	void SetLocalInverseInertiaTensor(const Matrix3& inverseInertiaTensor) { m_inverseInertiaTensorLocal = inverseInertiaTensor; }

	float	GetInverseMass() const { return m_iMass; }
	void	GetWorldInverseInertiaTensor(Matrix3& out_inverseInertiaTensor) const;
	Vector3 GetLinearVelocity() const { return m_velocityWs; }
	Vector3	GetAngularVelocityRadians() const { return m_angularVelocityRadiansWs; }
	bool	IsAwake() const { return m_isAwake; }


public:
	//-----Public Data-----

	Transform* transform = nullptr;


private:
	//-----Private Methods-----

	void ClearForces();


private:
	//-----Private Data-----

	Vector3		m_velocityWs = Vector3::ZERO;
	Vector3		m_accelerationWs = Vector3::ZERO;
	Vector3		m_angularVelocityRadiansWs = Vector3::ZERO;
	Vector3		m_forceAccumWs = Vector3::ZERO;
	Vector3		m_torqueAccumWs = Vector3::ZERO;
	bool		m_isAwake = false;
	float		m_iMass = 1.f;
	float		m_linearDamping = 0.999f;
	float		m_angularDamping = 0.999f;
	Matrix3		m_inverseInertiaTensorLocal;
	Matrix3		m_inverseInertiaTensorWorld;

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
