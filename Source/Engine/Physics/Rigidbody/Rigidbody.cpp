///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: May 2nd, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/DevConsole.h"
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
	Vector3 forceWs = transform->TransformDirection(forceLs);
	AddWorldForce(forceWs);
}


//-------------------------------------------------------------------------------------------------
void RigidBody::AddWorldForceAtWorldPoint(const Vector3& forceWs, const Vector3& pointWs)
{
	Vector3 centerToPoint = pointWs - transform->position;
	m_forceAccumWs += forceWs;
	m_torqueAccumWs += CrossProduct(centerToPoint, forceWs);
	m_isAwake = true;
}


//-------------------------------------------------------------------------------------------------
void RigidBody::AddWorldForceAtLocalPoint(const Vector3& forceWs, const Vector3& pointLs)
{
	Vector3 pointWs = transform->TransformPosition(pointLs);
	AddWorldForceAtWorldPoint(forceWs, pointWs);
}


//-------------------------------------------------------------------------------------------------
void RigidBody::AddLocalForceAtLocalPoint(const Vector3& forceLs, const Vector3& pointLs)
{
	Vector3 pointWs = transform->TransformPosition(pointLs);
	Vector3 forceWs = transform->TransformDirection(forceLs);
	AddWorldForceAtWorldPoint(forceWs, pointWs);
}


//-------------------------------------------------------------------------------------------------
void RigidBody::AddLocalForceAtWorldPoint(const Vector3& forceLs, const Vector3& pointWs)
{
	Vector3 forceWs = transform->TransformDirection(forceLs);
	AddWorldForceAtWorldPoint(forceWs, pointWs);
}


//-------------------------------------------------------------------------------------------------
void RigidBody::SetIsAwake(bool isAwake)
{
	m_isAwake = isAwake;

	if (m_isAwake)
	{
		m_motion += 2.f * SLEEP_EPSILON; // Add motion now to prevent it from immediately falling asleep
	}
	else
	{
		// Clear so when we wake up again we start from no movement
		m_velocityWs = Vector3::ZERO;
		m_angularVelocityRadiansWs = Vector3::ZERO;
	}
}


//-------------------------------------------------------------------------------------------------
void RigidBody::SetCanSleep(bool canSleep)
{
	m_canSleep = canSleep;

	// Wake me up if I'm asleep and am not allowed to be
	if (!m_canSleep && !m_isAwake)
	{
		SetIsAwake(true);
	}
}


//-------------------------------------------------------------------------------------------------
void RigidBody::GetWorldInverseInertiaTensor(Matrix3& out_inverseInertiaTensor) const
{
	out_inverseInertiaTensor = m_inverseInertiaTensorWorld;
}


//-------------------------------------------------------------------------------------------------
RigidBody::RigidBody(Transform* transform)
	: transform(transform)
{
}


//-------------------------------------------------------------------------------------------------
void RigidBody::Integrate(float deltaSeconds)
{
	if (!m_isAwake)
		return;

	// Corrections after last frame's integrate (as well as any rotations applied during the game frame)
	// will have changed the world moment of inertia - ensure that's up-to-date
	CalculateDerivedData();

	// Calculate accelerations
	Vector3 acceleration = m_accelerationWs;
	acceleration += (m_forceAccumWs * m_iMass);

	Vector3 angularAcceleration = m_inverseInertiaTensorWorld * m_torqueAccumWs;

	// Update velocities
	m_velocityWs += acceleration * deltaSeconds;
	m_angularVelocityRadiansWs += angularAcceleration * deltaSeconds;

	// Impose damping
	m_velocityWs *= Pow(m_linearDamping, deltaSeconds);
	m_angularVelocityRadiansWs *= Pow(m_angularDamping, deltaSeconds);

	// Update position/rotation
	transform->position += m_velocityWs * deltaSeconds;

	Quaternion deltaRotation = Quaternion::CreateFromEulerAnglesRadians(m_angularVelocityRadiansWs * deltaSeconds);
	transform->Rotate(deltaRotation, RELATIVE_TO_WORLD); // Forces/torques are world space, so velocity/angular velocity is ws....so this is a rotation about the world axes

	// Remember what our acceleration was last frame
	m_lastFrameAccelerationWs = acceleration;

	CalculateDerivedData();
	ClearForces();

	// Update the kinetic energy store, and possibly put the body to sleep.
	if (m_canSleep) {
		float currentMotion = DotProduct(m_velocityWs, m_velocityWs) + DotProduct(m_angularVelocityRadiansWs, m_angularVelocityRadiansWs);

		// Do a recency weighted average - this way quickly moving objects that suddenly stop won't sleep immediately
		float bias = Pow(0.2f, deltaSeconds);
		m_motion = bias * m_motion + (1.f - bias) * currentMotion;

		if (m_motion < SLEEP_EPSILON)
		{
			SetIsAwake(false);
		}
		else if (m_motion > 10.f * SLEEP_EPSILON)
		{
			// Keep motion from growing too much
			// Since we're using RWA, a sudden burst of speed will make this skyrocket and take a while to come back down if it suddenly stops
			m_motion = 10.f * SLEEP_EPSILON;
		}
	}
}


//-------------------------------------------------------------------------------------------------
void RigidBody::CalculateDerivedData()
{
	// Update the world inverse inertia tensor
	Matrix3 toWorldRotation = transform->GetLocalToWorldMatrix().GetMatrix3Part();
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
