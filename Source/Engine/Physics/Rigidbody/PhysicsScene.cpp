///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: May 3rd, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/Physics/RigidBody/RigidBody.h"
#include "Engine/Physics/RigidBody/RigidBodyForceGenerator.h"
#include "Engine/Physics/RigidBody/PhysicsScene.h"

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
PhysicsScene::PhysicsScene(CollisionScene<BoundingVolumeSphere>* collisionScene)
	: m_collisionScene(collisionScene)
{
}


//-------------------------------------------------------------------------------------------------
PhysicsScene::~PhysicsScene()
{
	SafeDeleteVector(m_forceGens);
	SafeDeleteVector(m_bodies);
}


//-------------------------------------------------------------------------------------------------
void PhysicsScene::BeginFrame()
{
	for (RigidBody* body : m_bodies)
	{
		body->CalculateDerivedData();
		body->ClearForces();
	}
}


//-------------------------------------------------------------------------------------------------
void PhysicsScene::DoPhysicsStep(float deltaSeconds)
{
	// Apply all forces
	m_forceRegistry.GenerateAndAddForces(deltaSeconds);

	// Update positions and velocities
	Integrate(deltaSeconds);

	// Check for collisions, then correct
	if (m_collisionScene != nullptr)
	{
		m_collisionScene->DoCollisionStep(deltaSeconds);
	}
}


//-------------------------------------------------------------------------------------------------
void PhysicsScene::AddRigidbody(RigidBody* body)
{
	if (std::find(m_bodies.begin(), m_bodies.end(), body) == m_bodies.end())
	{
		m_bodies.push_back(body);
	}
}


//-------------------------------------------------------------------------------------------------
void PhysicsScene::AddForceGenerator(RigidBodyForceGenerator* forceGen, RigidBody* body)
{
	// Add the generator and body of not already added
	if (std::find(m_forceGens.begin(), m_forceGens.end(), forceGen) == m_forceGens.end())
	{
		m_forceGens.push_back(forceGen);
	}

	if (std::find(m_bodies.begin(), m_bodies.end(), body) == m_bodies.end())
	{
		m_bodies.push_back(body);
	}

	m_forceRegistry.AddRegistration(body, forceGen);
}


//-------------------------------------------------------------------------------------------------
void PhysicsScene::Integrate(float deltaSeconds)
{
	for (RigidBody* body : m_bodies)
	{
		body->Integrate(deltaSeconds);
	}
}
