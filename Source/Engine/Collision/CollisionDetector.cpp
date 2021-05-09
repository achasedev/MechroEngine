///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: May 8th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Collision/CollisionDetector.h"
#include "Engine/Collision/CollisionPrimitive.h"
#include "Engine/Collision/Contact.h"
#include "Engine/Core/EngineCommon.h"
#include "Engine/Core/Entity.h"
#include "Engine/Math/MathUtils.h"

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
int CollisionDetector::GenerateContacts(const CollisionSphere& a, const CollisionSphere& b, CollisionData& collisionData)
{
	if (collisionData.numContacts >= MAX_CONTACT_COUNT)
		return 0;

	Sphere3D aSphere = a.GetWorldData();
	Sphere3D bSphere = b.GetWorldData();

	Vector3 bToA = aSphere.center - bSphere.center;
	float distanceSquared = bToA.GetLengthSquared();

	if (distanceSquared >= (aSphere.radius + bSphere.radius) * (aSphere.radius + bSphere.radius))
	{
		return 0.f;
	}

	float distance = bToA.Normalize();

	Contact* contact = &collisionData.contacts[collisionData.numContacts];

	contact->position = bSphere.center + 0.5f * distance * bToA;		// Contact position is the midpoint between the centers
	contact->normal = bToA;												// Orientation is set up s.t adding the normal to A would resolve the collision, -normal to B
	contact->penetration = (aSphere.radius + bSphere.radius) - distance;	// Pen is the overlap

	if (a.HasRigidBody() && b.HasRigidBody())
	{
		// TODO: Figure out restitution and friction
		contact->bodyA = a.GetRigidBody();
		contact->bodyB = b.GetRigidBody();	
		contact->restitution = collisionData.restitution;
		contact->friction = collisionData.friction;
	}

	collisionData.numContacts++;
	return 1;
}


//-------------------------------------------------------------------------------------------------
int CollisionDetector::GenerateContacts(const CollisionSphere& sphere, const CollisionHalfSpace& halfSpace, CollisionData& collisionData)
{
	if (collisionData.numContacts >= MAX_CONTACT_COUNT)
		return 0;

	Sphere3D sphereWs = sphere.GetWorldData();
	Plane3 planeWs = halfSpace.GetWorldData();

	float distance = planeWs.GetDistanceFromPlane(sphereWs.center) - sphereWs.radius;
	
	// Sphere too far in front of plane; its radius isn't enough to intersect the plane
	if (distance >= 0)
		return 0;

	Contact* contact = &collisionData.contacts[collisionData.numContacts];

	contact->normal = planeWs.GetNormal();
	contact->penetration = -distance;
	contact->position = planeWs.GetProjectedPointOntoPlane(sphereWs.center);

	if (sphere.HasRigidBody())
	{
		contact->bodyA = sphere.GetRigidBody();
		contact->bodyB = nullptr;
		contact->friction = collisionData.friction;
		contact->restitution = collisionData.restitution;
	}

	collisionData.numContacts++;
	return 1;
}


//-------------------------------------------------------------------------------------------------
int CollisionDetector::GenerateContacts(const CollisionBox& box, const CollisionHalfSpace& halfSpace, CollisionData& collisionData)
{
	if (collisionData.numContacts >= MAX_CONTACT_COUNT)
		return 0;

	OBB3 boxWs = box.GetWorldData();
	Plane3 planeWs = halfSpace.GetWorldData();

	Vector3 boxVertsWs[8];
	boxWs.GetPoints(boxVertsWs);

	int numContactsAdded = 0;
	for (int i = 0; i < 8; ++i)
	{
		float distance = planeWs.GetDistanceFromPlane(boxVertsWs[i]);

		if (distance < 0.f)
		{
			Contact* contact = &collisionData.contacts[collisionData.numContacts];

			contact->position = 0.5f * (boxVertsWs[i] + planeWs.GetProjectedPointOntoPlane(boxVertsWs[i])); // Position is half way between the box vertex and the plane
			contact->normal = planeWs.normal;
			contact->penetration = distance;

			if (box.HasRigidBody())
			{
				contact->bodyA = box.GetRigidBody();
				contact->bodyB = nullptr;
				contact->friction = collisionData.friction;
				contact->restitution = collisionData.restitution;
			}

			collisionData.numContacts++;
			numContactsAdded++;

			if (collisionData.numContacts == MAX_CONTACT_COUNT)
			{
				break;
			}
		}
	}

	return numContactsAdded;
}


//-------------------------------------------------------------------------------------------------
int CollisionDetector::GenerateContacts(const CollisionBox& box, const CollisionSphere& sphere, CollisionData& collisionData)
{
	
}
