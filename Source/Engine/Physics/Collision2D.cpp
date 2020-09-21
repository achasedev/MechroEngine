///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: September 9th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Physics/Collision2D.h"
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
	//RemoveAllBodies();
}


//-------------------------------------------------------------------------------------------------
RigidBodyID PhysicsScene2D::AddBody(const Polygon2D* shape)
{
	RigidBody2D* body = new RigidBody2D();
	body->m_shape = shape;
	body->SetMassProperties(1.0f);
	body->m_id = m_nextRigidbodyID++;

	m_bodies.push_back(body);

	return body->m_id;
}


//-------------------------------------------------------------------------------------------------
RigidBody2D* PhysicsScene2D::GetBody(RigidBodyID bodyID) const
{
	for (uint32 bodyIndex = 0; bodyIndex < m_bodies.size(); ++bodyIndex)
	{
		if (m_bodies[bodyIndex]->m_id == bodyID)
		{
			return m_bodies[bodyIndex];
		}
	}

	ERROR_RECOVERABLE("Couldn't find RigidBody2D!");
	return nullptr;
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
				m_arbiters[key] = newArb;
			}
			else
			{
				m_arbiters.erase(key);
			}
		}
	}
}
