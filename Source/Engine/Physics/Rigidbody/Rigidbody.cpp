///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: May 2nd, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Physics/RigidBody/RigidBody.h"

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
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
void RigidBody::AddWorldForce(const Vector3& forceWs)
{
	m_forceAccumWs += forceWs;
	m_isAwake = true;
}


//-------------------------------------------------------------------------------------------------
void RigidBody::AddLocalForce(const Vector3& forceLs)
{
	Vector3 forceWs = transform.TransformDirection(forceLs);
	AddWorldForce(forceWs);
}


//-------------------------------------------------------------------------------------------------
void RigidBody::AddWorldForceAtWorldPoint(const Vector3& forceWs, const Vector3& pointWs)
{
	Vector3 centerToPoint = pointWs - transform.position;
	m_forceAccumWs += forceWs;
	m_torqueAccumWs += CrossProduct(centerToPoint, forceWs);
	m_isAwake = true;
}


//-------------------------------------------------------------------------------------------------
void RigidBody::AddWorldForceAtLocalPoint(const Vector3& forceWs, const Vector3& pointLs)
{
	Vector3 pointWs = transform.TransformPosition(pointLs);
	AddWorldForceAtWorldPoint(forceWs, pointWs);
}


//-------------------------------------------------------------------------------------------------
void RigidBody::AddLocalForceAtLocalPoint(const Vector3& forceLs, const Vector3& pointLs)
{
	Vector3 pointWs = transform.TransformPosition(pointLs);
	Vector3 forceWs = transform.TransformDirection(forceLs);
	AddWorldForceAtWorldPoint(forceWs, pointWs);
}


//-------------------------------------------------------------------------------------------------
void RigidBody::AddLocalForceAtWorldPoint(const Vector3& forceLs, const Vector3& pointWs)
{
	Vector3 forceWs = transform.TransformDirection(forceLs);
	AddWorldForceAtWorldPoint(forceWs, pointWs);
}


//-------------------------------------------------------------------------------------------------
void RigidBody::Integrate(float deltaSeconds)
{
	// Calculate accelerations
	Vector3 acceleration = m_acceleration;
	acceleration += (m_forceAccumWs * m_iMass);

	Vector3 angularAcceleration = m_inverseInertiaTensorWorld * m_torqueAccumWs;

	// Update velocities
	m_velocity += acceleration * deltaSeconds;
	m_angularVelocityRadians += angularAcceleration * deltaSeconds;

	// Impose damping
	m_velocity *= Pow(m_linearDamping, deltaSeconds);
	m_angularVelocityRadians *= Pow(m_angularDamping, deltaSeconds);

	// Update position/rotation
	transform.position += m_velocity * deltaSeconds;

	Quaternion deltaRotation = Quaternion::CreateFromEulerAnglesRadians(m_angularVelocityRadians);
	transform.Rotate(deltaRotation, RELATIVE_TO_WORLD); // Forces/torques are world space, so velocity/angular velocity is ws....so this is a rotation about the world axes

	CalculateDerivedData();
	ClearForces();
}


//-------------------------------------------------------------------------------------------------
void RigidBody::CalculateDerivedData()
{
	// Update the world inverse inertia tensor
	Matrix3 toWorldRotation = transform.GetLocalToWorldMatrix().GetMatrix3Part();
	Matrix3 toLocalRotation = toWorldRotation;
	toLocalRotation.Transpose();
	ASSERT_OR_DIE(AreMostlyEqual(toLocalRotation, toWorldRotation.GetInverse()), "Transpose and inverse are not equal"); // Transpose should be the inverse, but to be safe check

	m_inverseInertiaTensorWorld = toWorldRotation * m_inverseInertiaTensorLocal * toLocalRotation;
}


//-------------------------------------------------------------------------------------------------
void RigidBody::ClearForces()
{
	m_forceAccumWs = Vector3::ZERO;
	m_torqueAccumWs = Vector3::ZERO;
}
