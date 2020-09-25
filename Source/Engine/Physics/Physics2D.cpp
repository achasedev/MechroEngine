///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: September 9th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Physics/Physics2D.h"
#include "Engine/Physics/RigidBody2D.h"

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
PhysicsScene2D::~PhysicsScene2D()
{
	// Ensure all GameObjects have been removed first before destroying this scene
	// Otherwise there's going to be dangling pointers to deleted RigidBody2Ds
	ASSERT_RECOVERABLE(m_bodies.size() == 0, "PhysicsScene being destroyed before all GameObjects were removed!");

	ArbIter iter = m_arbiters.begin();

	for (iter; iter != m_arbiters.end(); iter++)
	{
		SAFE_DELETE_POINTER(iter->second);
	}

	m_arbiters.clear();
}


//-------------------------------------------------------------------------------------------------
RigidBody2D* PhysicsScene2D::AddGameObject(GameObject* gameObject)
{
	ASSERT_RETURN(gameObject->GetRigidBody2D() == nullptr, nullptr, "GameObject already has a RigidBody2D!");

	RigidBody2D* body = new RigidBody2D(this, gameObject);
	body->m_shape = gameObject->GetShape();
	body->SetMassProperties(1.0f);

	m_bodies.push_back(body);
	gameObject->SetRigidBody2D(body);

	return body;
}


//-------------------------------------------------------------------------------------------------
void PhysicsScene2D::RemoveGameObject(GameObject* gameObject)
{
	RigidBody2D* currBody = gameObject->GetRigidBody2D();
	ASSERT_RETURN(currBody != nullptr, NO_RETURN_VAL, "Removing GameObject that isn't in the scene!");

	for (uint32 i = 0; i < m_bodies.size(); ++i)
	{
		if (m_bodies[i] == currBody)
		{
			m_bodies.erase(m_bodies.begin() + i);

			SAFE_DELETE_POINTER(currBody);
			gameObject->SetRigidBody2D(nullptr);
			return;
		}
	}

	ERROR_RECOVERABLE("GameObject didn't have a RigidBody2D to remove!");
}


//-------------------------------------------------------------------------------------------------
void PhysicsScene2D::FrameStep()
{
	PerformBroadphase();
	//ApplyForces();
	//PerformArbiterPreSteps();
	//ApplyImpulseIterations();
	//UpdatePositions();
}


//-------------------------------------------------------------------------------------------------
void PhysicsScene2D::PerformBroadphase()
{
	// O(n^2) broad-phase
	// TODO: Make this less garbage
	for (uint32 firstBodyIndex = 0; firstBodyIndex < m_bodies.size(); ++firstBodyIndex)
	{
		RigidBody2D* body1 = m_bodies[firstBodyIndex];

		for (uint32 secondBodyIndex = firstBodyIndex + 1; secondBodyIndex < m_bodies.size(); ++secondBodyIndex)
		{
			RigidBody2D* body2 = m_bodies[secondBodyIndex];

			// If both bodies are static don't bother with physics (though maybe this should be an assert)
			if (body1->GetInverseMass() == 0.0f && body2->GetInverseMass() == 0.0f)
				continue;

			Arbiter2D* newArb = new Arbiter2D(body1, body2);
			newArb->DetectCollision();

			ArbiterKey2D key(body1, body2);

			if (newArb->GetNumContacts() > 0)
			{
				// TODO: Need to preserve info or something or something
				SAFE_DELETE_POINTER(m_arbiters[key]);
				m_arbiters[key] = newArb;
			}
			else
			{
				m_arbiters.erase(key);
				SAFE_DELETE_POINTER(newArb);
			}
		}
	}
}
