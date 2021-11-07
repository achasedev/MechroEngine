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
void RigidBody::SetInertiaTensor_Capsule(float h, float r)
{
	if (m_iMass == 0.f)
	{
		ConsoleWarningf("Attempting to set the inertia tensor of an immovable object to that of a capsule. Ignoring...");
		return;
	}

	float vcyl = h * (r * r) * PI; // Cylinder
	float vhs = (2.f / 3.f) * (r * r * r) * PI; // Hemisphere
	float volume = vcyl + 2.f * vhs; // 2 half spheres, on top and bottom of cylinder

	float mass = (1.f / m_iMass);
	float density = (mass / volume);

	float mcyl = vcyl * density; // Cylinder mass
	float mhs = vhs * density; // Hemisphere mass

	Matrix3 inertiaTensor = Matrix3::IDENTITY;
	inertiaTensor.Ix = mcyl * ((1.f / 12.f) * (h * h) + 0.25f * (r * r)) + 2.f * mhs * ((0.4f * r * r) + (0.5f * h * h) + 0.375f * h * r);
	inertiaTensor.Jy = mcyl * (0.5f * r * r) + 2.f * mhs * (0.4f * r * r);
	inertiaTensor.Kz = inertiaTensor.Ix;

	m_inverseInertiaTensorLocal = inertiaTensor.GetInverse();
}


//-------------------------------------------------------------------------------------------------
void RigidBody::SetInertiaTensor_Cylinder(float h, float r)
{
	if (m_iMass == 0.f)
	{
		ConsoleWarningf("Attempting to set the inertia tensor of an immovable object to that of a cylinder. Ignoring...");
		return;
	}

	float mass = (1.f / m_iMass);

	Matrix3 inertiaTensor = Matrix3::IDENTITY;
	inertiaTensor.Ix = (1.f / 12.f) * mass * (3 * (r * r) + (h * h));
	inertiaTensor.Jy = 0.5f * mass * r * r;
	inertiaTensor.Kz = inertiaTensor.Ix;

	m_inverseInertiaTensorLocal = inertiaTensor.GetInverse();
}


//-------------------------------------------------------------------------------------------------
void RigidBody::SetInertiaTensor_Box(const Vector3& extents)
{
	if (m_iMass == 0.f)
	{
		ConsoleWarningf("Attempting to set the inertia tensor of an immovable object to that of a box. Ignoring...");
		return;
	}

	float mass = (1.f / m_iMass);
	float w = 2.f * extents.x;
	float h = 2.f * extents.y;
	float l = 2.f * extents.z;

	Matrix3 inertiaTensor = Matrix3::IDENTITY;
	inertiaTensor.Ix = (1.f / 12.f) * mass * (h * h + l * l);
	inertiaTensor.Jy = (1.f / 12.f) * mass * (w * w + l * l);
	inertiaTensor.Kz = (1.f / 12.f) * mass * (w * w + h * h);

	SetInverseInertiaTensor(inertiaTensor.GetInverse());
}


//-------------------------------------------------------------------------------------------------
void RigidBody::SetInertiaTensor_Sphere(float radius)
{
	if (m_iMass == 0.f)
	{
		ConsoleWarningf("Attempting to set the inertia tensor of an immovable object to that of a sphere. Ignoring...");
		return;
	}

	float mass = (1.f / m_iMass);
	float moment = (2.f / 5.f) * mass * (radius * radius);

	Matrix3 inertiaTensor = Matrix3::IDENTITY;
	inertiaTensor.Ix = moment;
	inertiaTensor.Jy = moment;
	inertiaTensor.Kz = moment;

	SetInverseInertiaTensor(inertiaTensor.GetInverse());
}


//-------------------------------------------------------------------------------------------------
void RigidBody::SetAngularVelocityDegreesWs(const Vector3& angularVelocityDegreesWs)
{
	SetAngularVelocityRadiansWs(DegreesToRadians(angularVelocityDegreesWs));
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
void RigidBody::SetRotationLocked(bool lockRotation)
{
	m_rotationLocked = lockRotation;
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
void RigidBody::Integrate(float deltaSeconds, const Vector3& gravityAcc)
{
	if (!m_isAwake)
		return;

	// Corrections after last frame's integrate (as well as any rotations applied during the game frame)
	// will have changed the world moment of inertia - ensure that's up-to-date
	CalculateDerivedData();

	// Calculate/apply linear acceleration
	Vector3 acceleration = m_accelerationWs + gravityAcc;
	acceleration += (m_forceAccumWs * m_iMass);
	m_velocityWs += acceleration * deltaSeconds;

	// Impose linear damping
	m_velocityWs *= Pow(m_linearDamping, deltaSeconds);

	// Clamp to speed limits
	Vector3 lateralVelocity = Vector3(m_velocityWs.x, 0.f, m_velocityWs.z);
	if (lateralVelocity.GetLengthSquared() > m_maxLateralSpeed * m_maxLateralSpeed)
	{
		lateralVelocity.Normalize();
		lateralVelocity *= m_maxLateralSpeed;
		m_velocityWs.x = lateralVelocity.x;
		m_velocityWs.z = lateralVelocity.z;
	}

	if (Abs(m_velocityWs.y) > m_maxVerticalSpeed)
	{
		m_velocityWs.y = (m_velocityWs.y > 0.f ? m_maxVerticalSpeed : -m_maxVerticalSpeed);
	}

	transform->position += m_velocityWs * deltaSeconds;

	// Calculate/apply angular acceleration
	if (!m_rotationLocked)
	{
		Vector3 angularAcceleration = m_inverseInertiaTensorWorld * m_torqueAccumWs;
		m_angularVelocityRadiansWs += angularAcceleration * deltaSeconds;
		m_angularVelocityRadiansWs *= Pow(m_angularDamping, deltaSeconds);

		Quaternion deltaRotation = Quaternion::CreateFromEulerAnglesRadians(m_angularVelocityRadiansWs * deltaSeconds);
		transform->Rotate(deltaRotation, RELATIVE_TO_WORLD); // Forces/torques are world space, so velocity/angular velocity is ws....so this is a rotation about the world axes
	}
	else
	{
		// Clear this to prevent accumulation
		m_angularVelocityRadiansWs = Vector3::ZERO;
	}

	// Remember what our acceleration was last frame
	m_lastFrameAccelerationWs = acceleration;

	CalculateDerivedData();
	ClearForces();

	// Update the kinetic energy store, and possibly put the body to sleep.
	if (m_canSleep) 
	{
		float currentMotion = DotProduct(m_velocityWs, m_velocityWs) + DotProduct(m_angularVelocityRadiansWs, m_angularVelocityRadiansWs);

		// Do a recency weighted average - this way quickly moving objects that suddenly stop won't sleep immediately
		float bias = Pow(0.1f, deltaSeconds);
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
	// Update the world inverse inertia tensor if we use it
	if (!IsRotationLocked())
	{
		Matrix3 toWorldRotation = transform->GetLocalToWorldMatrix().GetMatrix3Part();
		Matrix3 toLocalRotation = toWorldRotation;
		toLocalRotation.Transpose();
		ASSERT_OR_DIE(AreMostlyEqual(toLocalRotation, toWorldRotation.GetInverse()), "Transpose and inverse are not equal"); // Transpose should be the inverse, but to be safe check

		m_inverseInertiaTensorWorld = toWorldRotation * m_inverseInertiaTensorLocal * toLocalRotation;
	}
}


//-------------------------------------------------------------------------------------------------
void RigidBody::ClearForces()
{
	m_forceAccumWs = Vector3::ZERO;
	m_torqueAccumWs = Vector3::ZERO;
}
