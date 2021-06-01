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

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class BoxCollider;
class CapsuleCollider;
class Contact;
class HalfSpaceCollider;
class PlaneCollider;
class SphereCollider;

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

	int GenerateContacts(const SphereCollider& a,			const SphereCollider& b,			Contact* out_contacts, int limit);
	int GenerateContacts(const SphereCollider& sphere,		const HalfSpaceCollider& halfSpace, Contact* out_contacts, int limit);
	int GenerateContacts(const SphereCollider& sphere,		const PlaneCollider& plane,			Contact* out_contacts, int limit);
	int GenerateContacts(const SphereCollider& sphere,		const CapsuleCollider& capsule,		Contact* out_contacts, int limit);
	int GenerateContacts(const CapsuleCollider& capsule,	const HalfSpaceCollider& halfSpace, Contact* out_contacts, int limit);
	int GenerateContacts(const CapsuleCollider& capsule,	const PlaneCollider& plane,			Contact* out_contacts, int limit);
	int GenerateContacts(const CapsuleCollider& a,			const CapsuleCollider& b,			Contact* out_contacts, int limit);
	int GenerateContacts(const BoxCollider& box,			const HalfSpaceCollider& halfSpace, Contact* out_contacts, int limit);
	int GenerateContacts(const BoxCollider& box,			const PlaneCollider& plane,			Contact* out_contacts, int limit);
	int GenerateContacts(const BoxCollider& box,			const SphereCollider& sphere,		Contact* out_contacts, int limit);
	int GenerateContacts(const BoxCollider& a,				const BoxCollider& b,				Contact* out_contacts, int limit);
	int GenerateContacts(const BoxCollider& box,			const CapsuleCollider& capsule,		Contact* out_contacts, int limit);


private:
	//-----Private Data-----

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
