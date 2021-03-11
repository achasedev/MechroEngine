///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 26th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Physics/3D/Physics3D.h"
#include "Engine/Physics/3D/RigidBody3D.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------
const Vector3 PhysicsScene3D::DEFAULT_GRAVITY = Vector3(0.f, -9.8f, 0.f);
const uint32 PhysicsScene3D::NUM_IMPULSE_ITERATIONS = 10U;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
PhysicsScene3D::~PhysicsScene3D()
{
	// Ensure all GameObjects have been removed first before destroying this scene
	// Otherwise there's going to be dangling pointers to deleted RigidBody3Ds
	ASSERT_RECOVERABLE(m_bodies.size() == 0, "PhysicsScene being destroyed before all GameObjects were removed!");
	m_bodies.clear();
	m_arbiters.clear();
}


//-------------------------------------------------------------------------------------------------
RigidBody3D* PhysicsScene3D::AddGameObject(GameObject* gameObject)
{
	ASSERT_RETURN(gameObject->GetRigidBody3D() == nullptr, nullptr, "GameObject already has a RigidBody3D!");

	RigidBody3D* body = new RigidBody3D(this, gameObject);

	m_bodies.push_back(body);
	gameObject->SetRigidBody3D(body);

	return body;
}


//-------------------------------------------------------------------------------------------------
void PhysicsScene3D::RemoveGameObject(GameObject* gameObject)
{
	RigidBody3D* currBody = gameObject->GetRigidBody3D();
	ASSERT_RETURN(currBody != nullptr, NO_RETURN_VAL, "Removing GameObject that isn't in the scene!");

	for (uint32 i = 0; i < m_bodies.size(); ++i)
	{
		if (m_bodies[i] == currBody)
		{
			m_bodies.erase(m_bodies.begin() + i);

			SAFE_DELETE(currBody);
			gameObject->SetRigidBody3D(nullptr);
			return;
		}
	}

	ERROR_RECOVERABLE("GameObject didn't have a RigidBody3D to remove!");
}


//-------------------------------------------------------------------------------------------------
void PhysicsScene3D::FrameStep(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	PerformBroadphase();
	//ApplyForces(deltaSeconds);
	//PerformArbiterPreSteps(deltaSeconds);
	//ApplyImpulseIterations();
	//UpdatePositions(deltaSeconds);
}


//-------------------------------------------------------------------------------------------------
Arbiter3D* PhysicsScene3D::GetArbiter3DForBodies(RigidBody3D* first, RigidBody3D* second)
{
	ArbiterKey3D key(first, second);

	if (m_arbiters.find(key) != m_arbiters.end())
	{
		return &m_arbiters.at(key);
	}

	return nullptr;
}


//-------------------------------------------------------------------------------------------------
void PhysicsScene3D::PerformBroadphase()
{
	// O(n^2) broad-phase
	// TODO: Make this less garbage
	for (uint32 firstBodyIndex = 0; firstBodyIndex < m_bodies.size() - 1; ++firstBodyIndex)
	{
		RigidBody3D* body1 = m_bodies[firstBodyIndex];

		for (uint32 secondBodyIndex = firstBodyIndex + 1; secondBodyIndex < m_bodies.size(); ++secondBodyIndex)
		{
			RigidBody3D* body2 = m_bodies[secondBodyIndex];

			// If both bodies are static don't bother with physics (though maybe this should be a warning of sorts)
			//if (body1->GetInverseMass() == 0.0f && body2->GetInverseMass() == 0.0f)
			//	continue;

			Arbiter3D newArb = Arbiter3D(body1, body2);
			newArb.DetectCollision();

			ArbiterKey3D key(body1, body2);

			if (newArb.GetNumContacts() > 0)
			{
				// Check if an arbiter already exists for this collision, and if so update
				//Arb3DIter iter = m_arbiters.find(key);
				//if (iter == m_arbiters.end())
				//{
					m_arbiters[key] = newArb;
				//}
				//else
				//{
				//	iter->second.Update(newArb.GetContacts(), newArb.GetNumContacts());
				//}
			}
			else
			{
				m_arbiters.erase(key);
			}
		}
	}
}


//-------------------------------------------------------------------------------------------------
void PhysicsScene3D::ApplyForces(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	//uint32 numBodies = (uint32)m_bodies.size();

	//for (uint32 bodyIndex = 0; bodyIndex < numBodies; ++bodyIndex)
	//{
	//	RigidBody2D* body = m_bodies[bodyIndex];

	//	if (body->m_invMass == 0.f)
	//	{
	//		continue;
	//	}

	//	// Force = Mass * Acceleration :)
	//	Vector2 gravity = (body->IsAffectedByGravity() ? m_gravity : Vector2::ZERO);
	//	body->m_velocityWs += (body->m_invMass * body->m_forceWs + gravity) * deltaSeconds;
	//	body->m_angularVelocityDegrees += RadiansToDegrees((body->m_invInertia * body->m_torque) * deltaSeconds);

	//	// Zero out forces, they're per-frame
	//	body->m_forceWs = Vector2::ZERO;
	//	body->m_torque = 0.f;
	//}
}


//-------------------------------------------------------------------------------------------------
void PhysicsScene3D::PerformArbiterPreSteps(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	//ArbIter iter = m_arbiters.begin();

	//for (iter; iter != m_arbiters.end(); iter++)
	//{
	//	iter->second.PreStep(deltaSeconds);
	//}
}


//-------------------------------------------------------------------------------------------------
void PhysicsScene3D::ApplyImpulseIterations()
{
	/*for (uint32 i = 0; i < NUM_IMPULSE_ITERATIONS; ++i)
	{
		ArbIter iter = m_arbiters.begin();

		for (iter; iter != m_arbiters.end(); iter++)
		{
			iter->second.ApplyImpulse();
		}
	}*/
}


//-------------------------------------------------------------------------------------------------
void PhysicsScene3D::UpdatePositions(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	//uint32 numBodies = (uint32)m_bodies.size();

	//for (uint32 bodyIndex = 0; bodyIndex < numBodies; ++bodyIndex)
	//{
	//	RigidBody2D* body = m_bodies[bodyIndex];

	//	// Get the deltas
	//	Matrix44	worldToParent = body->m_transform->GetWorldToParentMatrix();
	//	Vector3		deltaPositionWs = Vector3(body->m_velocityWs * deltaSeconds, 0.f);
	//	Vector3		deltaPositionPs = worldToParent.TransformVector(deltaPositionWs).xyz();
	//	float		deltaRotation = body->m_angularVelocityDegrees * deltaSeconds;

	//	// Rotating about the center of mass will also update position if the center of mass and position aren't the same point!
	//	// Ps == Parent space, i.e. this is relative to the parent, and if parent is nullptr then it's just world
	//	Vector2		positionPs = body->m_transform->position.xy;
	//	Vector2		centerOfMassPs = worldToParent.TransformPoint(Vector3(body->GetCenterOfMassWs(), 0.f)).xy();
	//	Vector2		centerOfMassToPositionPs = positionPs - centerOfMassPs;

	//	// Using the angle sum rule
	//	Vector2		offsetPs = Vector2(CosDegrees(deltaRotation) * centerOfMassToPositionPs.x - SinDegrees(deltaRotation) * centerOfMassToPositionPs.y, SinDegrees(deltaRotation) * centerOfMassToPositionPs.x + CosDegrees(deltaRotation) * centerOfMassToPositionPs.y);

	//	// Determine new position after rotating
	//	body->m_transform->position = Vector3(centerOfMassPs + offsetPs, 0.f);

	//	// Apply the delta position from linear movement
	//	body->m_transform->position += deltaPositionPs;

	//	// Set rotation
	//	body->m_transform->Rotate(0.f, 0.f, deltaRotation);
	//}
}
