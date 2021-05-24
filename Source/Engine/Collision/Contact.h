///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: May 8th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Math/Matrix3.h"
#include "Engine/Math/Vector3.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Collider;
class RigidBody;
typedef unsigned int ContactFeatureID;
enum ContactRecordType
{
	CONTACT_RECORD_INVALID = -1,
	CONTACT_RECORD_BOX_BOX_EDGE_EDGE,
	CONTACT_RECORD_BOX_BOX_FACE_VERTEX,
};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class ContactFeatureRecord
{
public:
	//-----Public Methods-----

	ContactFeatureRecord() {}
	ContactFeatureRecord(ContactRecordType type, const Collider* firstCollider, const Collider* secondCollider, ContactFeatureID firstID, ContactFeatureID secondID);

	bool operator==(const ContactFeatureRecord& other) const;
	bool operator!=(const ContactFeatureRecord& other) const;

	bool IsValid() const { return m_type != CONTACT_RECORD_INVALID; }
	ContactRecordType GetType() const { return m_type; }
	const Collider* GetFirstCollider() const { return m_firstCollider; }
	const Collider* GetSecondCollider() const { return m_secondCollider; }
	ContactFeatureID GetFirstID() const { return m_firstID; }
	ContactFeatureID GetSecondID() const { return m_secondID; }


private:
	//-----Private Data-----

	ContactRecordType	m_type = CONTACT_RECORD_INVALID;
	const Collider*		m_firstCollider = nullptr;
	const Collider*		m_secondCollider = nullptr;
	ContactFeatureID	m_firstID = 0;
	ContactFeatureID	m_secondID = 0;

};


//-------------------------------------------------------------------------------------------------
class Contact
{
public:
	//-----Public Methods-----

	Contact();

	void MatchAwakeState();

	void CalculateInternals(float deltaSeconds);
	void CalculateBasis();
	void CalculateClosingVelocityInContactSpace(float deltaSeconds);
	void CalculateDesiredVelocityInContactSpace(float deltaSeconds);
	void CheckValuesAreReasonable() const;


public:
	//-----Public Data-----

	Vector3					position = Vector3::ZERO;
	Vector3					normal = Vector3::ZERO;
	float					penetration = 0.f;
	float					restitution = 1.f;
	float					friction = 0.f;
	RigidBody*				bodies[2];
	Matrix3					contactToWorld = Matrix3::IDENTITY;
	Vector3					closingVelocityContactSpace = Vector3::ZERO;
	float					desiredDeltaVelocityAlongNormal = 0.f;
	Vector3					bodyToContact[2];
	ContactFeatureRecord	featureRecord;
	bool					isValid = false;
	int						ageInFrames = 0;

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
