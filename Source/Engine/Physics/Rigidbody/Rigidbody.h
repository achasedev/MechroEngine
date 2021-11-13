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
	void Integrate(float deltaSeconds, const Vector3& gravityAcc);
	
	void AddWorldForce(const Vector3& forceWs);
	void AddLocalForce(const Vector3& forceLs);
	void AddWorldForceAtWorldPoint(const Vector3& forceWs, const Vector3& pointWs);
	void AddWorldForceAtLocalPoint(const Vector3& forceWs, const Vector3& pointLs);
	void AddLocalForceAtLocalPoint(const Vector3& forceLs, const Vector3& pointLs);
	void AddLocalForceAtWorldPoint(const Vector3& forceLs, const Vector3& pointWs);
	void AddWorldVelocity(const Vector3& velocity) { m_velocityWs += velocity; }
	void AddWorldAngularVelocityRadians(const Vector3& angularVelocityRadians) { m_angularVelocityRadiansWs += angularVelocityRadians; }

	void SetInertiaTensor_Capsule(float cylinderHeight, float radius);
	void SetInertiaTensor_Cylinder(float height, float radius);
	void SetInertiaTensor_Box(const Vector3& extents);
	void SetInertiaTensor_Sphere(float radius);
	void SetInverseInertiaTensor(const Matrix3& inverseInertiaTensor, const Vector3& centerOfMassLs);

	void SetVelocityWs(const Vector3& velocityWs) { m_velocityWs = velocityWs; }
	void SetAngularVelocityRadiansWs(const Vector3& angularVelocityRadiansWs) { m_angularVelocityRadiansWs = angularVelocityRadiansWs; }
	void SetAngularVelocityDegreesWs(const Vector3& angularVelocityDegreesWs);
	void SetAcceleration(const Vector3& acceleration) { m_accelerationWs = acceleration; }
	void SetInverseMass(float iMass);
	void SetLinearDamping(float linearDamping) { m_linearDamping = linearDamping; }
	void SetAngularDamping(float angularDamping) { m_angularDamping = angularDamping; }
	void SetIsAwake(bool isAwake);
	void SetCanSleep(bool canSleep);
	void SetAffectedByGravity(bool affectedByGravity) { m_affectedByGravity = affectedByGravity; }
	void SetGravityScale(float scale) { m_gravityScale = scale; }
	void SetRotationLocked(bool lockRotation);
	void SetMaxLateralSpeed(float maxLateralSpeed) { m_maxLateralSpeed = maxLateralSpeed; }
	void SetMaxVerticalSpeed(float maxVerticalSpeed) { m_maxVerticalSpeed = maxVerticalSpeed; }

	Vector3	GetCenterOfMassWs() const;
	Vector3 GetLastFrameAcceleration() const { return m_lastFrameAccelerationWs; }
	float	GetInverseMass() const { return m_iMass; }
	void	GetWorldInverseInertiaTensor(Matrix3& out_inverseInertiaTensor) const;
	Vector3 GetVelocityWs() const { return m_velocityWs; }
	Vector3	GetAngularVelocityRadiansWs() const { return m_angularVelocityRadiansWs; }
	float	GetGravityScale() const { return m_gravityScale; }
	bool	IsAwake() const { return m_isAwake; }
	bool	CanSleep() const { return m_canSleep; }
	bool	IsAffectedByGravity() const { return m_affectedByGravity; }
	bool	IsRotationLocked() const { return m_rotationLocked; }
	bool	IsStatic() const { return m_iMass <= 0.f; }


public:
	//-----Public Data-----

	Transform* transform = nullptr;


private:
	//-----Private Methods-----

	void ClearForces();


private:
	//-----Private Static Data-----

	static constexpr float SLEEP_EPSILON = 0.1f;


private:
	//-----Private Data-----

	Vector3		m_centerOfMassLs = Vector3::ZERO;
	Vector3		m_velocityWs = Vector3::ZERO;
	Vector3		m_accelerationWs = Vector3::ZERO;
	Vector3		m_lastFrameAccelerationWs = Vector3::ZERO;
	Vector3		m_angularVelocityRadiansWs = Vector3::ZERO;
	Vector3		m_forceAccumWs = Vector3::ZERO;
	Vector3		m_torqueAccumWs = Vector3::ZERO;
	float		m_iMass = 1.f;
	float		m_linearDamping = 0.9f;
	float		m_angularDamping = 0.9f;
	Matrix3		m_inverseInertiaTensorLocal;
	Matrix3		m_inverseInertiaTensorWorld;
	bool		m_isAwake = true;
	bool		m_canSleep = true;
	float		m_motion = 2.f * SLEEP_EPSILON;

	bool		m_affectedByGravity = true;
	float		m_gravityScale = 1.0f;
	bool		m_rotationLocked = false;
	float		m_maxLateralSpeed = 1000.f;
	float		m_maxVerticalSpeed = 1000.f;
};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
