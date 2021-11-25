///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: May 8th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#define NUM_COLLIDER_TYPES (7)

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Collider;
class Contact;
class CollisionDetector;
typedef int(CollisionDetector::*GenerateContactsFunction)(const Collider* a, const Collider* b, Contact* out_contacts, int limit);


class CapsuleCollider;
class SphereCollider;
class HalfSpaceCollider;
class PlaneCollider;
class BoxCollider;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class CollisionDetector
{
public:
	//-----Public Methods-----

	int GenerateContacts(const Collider* a, const Collider* b, Contact* out_contacts, int limit);


private:
	//-----Private Methods-----

	// [0][X]
	int GenerateContacts_HalfSpaceSphere(const Collider* a, const Collider* b, Contact* out_contacts, int limit);
	int GenerateContacts_HalfSpaceCapsule(const Collider* a, const Collider* b, Contact* out_contacts, int limit);
	int GenerateContacts_HalfSpaceBox(const Collider* a, const Collider* b, Contact* out_contacts, int limit);
	int GenerateContacts_HalfSpaceCylinder(const Collider* a, const Collider* b, Contact* out_contacts, int limit);
	int GenerateContacts_HalfSpaceHull(const Collider* a, const Collider* b, Contact* out_contacts, int limit);

	// [1][X]
	int GenerateContacts_PlaneSphere(const Collider* a, const Collider* b, Contact* out_contacts, int limit);
	int GenerateContacts_PlaneCapsule(const Collider* a, const Collider* b, Contact* out_contacts, int limit);
	int GenerateContacts_PlaneBox(const Collider* a, const Collider* b, Contact* out_contacts, int limit);
	int GenerateContacts_PlaneCylinder(const Collider* a, const Collider* b, Contact* out_contacts, int limit);

	// [2][X]
	int GenerateContacts_SphereSphere(const Collider* a, const Collider* b, Contact* out_contacts, int limit);
	int GenerateContacts_SphereCapsule(const Collider* a, const Collider* b, Contact* out_contacts, int limit);
	int GenerateContacts_SphereBox(const Collider* a, const Collider* b, Contact* out_contacts, int limit);
	int GenerateContacts_SphereCylinder(const Collider* a, const Collider* b, Contact* out_contacts, int limit);
	int GenerateContacts_SphereHull(const Collider* a, const Collider* b, Contact* out_contacts, int limit);

	// [3][X]
	int GenerateContacts_CapsuleCapsule(const Collider* a, const Collider* b, Contact* out_contacts, int limit);
	int GenerateContacts_CapsuleBox(const Collider* a, const Collider* b, Contact* out_contacts, int limit);
	int GenerateContacts_CapsuleCylinder(const Collider* a, const Collider* b, Contact* out_contacts, int limit);
	int GenerateContacts_CapsuleHull(const Collider* a, const Collider* b, Contact* out_contacts, int limit);

	// [4][X]
	int GenerateContacts_BoxBox(const Collider* a, const Collider* b, Contact* out_contacts, int limit);

	// [5][X]

	// [6][X]
	int GenerateContacts_HullHull(const Collider* a, const Collider* b, Contact* out_contacts, int limit);


private:
	//-----Private Data-----

	static GenerateContactsFunction s_colliderMatrix[NUM_COLLIDER_TYPES][NUM_COLLIDER_TYPES];

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
