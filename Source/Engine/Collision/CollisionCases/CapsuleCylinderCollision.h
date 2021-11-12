///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: Nov 11th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Math/Capsule3D.h"
#include "Engine/Math/Cylinder3D.h"
#include "Engine/Math/Plane3.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Contact;
class CapsuleCollider;
class CylinderCollider;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class CapsuleCylinderCollision
{
public:
	//-----Public Methods-----

	CapsuleCylinderCollision(const CapsuleCollider* capsuleCollider, const CylinderCollider* cylinderCollider, Contact* out_contacts, int contactLimit);
	
	void Solve();
	int	 GetNumContacts() const { return m_numContacts; }


private:
	//-----Private Methods-----

	void InitializeValues();
	void SolveVertical();
	void SolveHorizontal();
	void SolveEdge();
	void MakeContacts();
	void MakeVerticalContacts();
	void MakeHorizontalContacts();
	void MakeEdgeContacts();


private:
	//-----Private Data-----

	const CapsuleCollider* m_capsuleCollider = nullptr;
	const CylinderCollider* m_cylinderCollider = nullptr;

	Contact*	m_contacts = nullptr;
	int			m_numContacts = 0;
	int			m_contactLimit = 2;

	Capsule3D	m_capsule;
	Cylinder3D	m_cylinder;

	// General
	Vector3		m_capSpineDir = Vector3::ZERO;
	Vector3		m_cylSpineDir = Vector3::ZERO;
	float		m_capHeight = 0.f;
	float		m_cylHeight = 0.f;
	float		m_capRadius = 0.f;
	float		m_cylRadius = 0.f;
	Plane3		m_cylTopPlane;
	Plane3		m_cylBottomPlane;
	Vector3		m_cylClosestSegPt = Vector3::ZERO;
	Vector3		m_capClosestSegPt = Vector3::ZERO;
	float		m_distBetweenSegs = 0.f;

	// Vertical
	float		m_worstVerticalPen = FLT_MAX;
	float		m_verticalPens[2];
	Vector3		m_verticalNormal = Vector3::ZERO;
	Vector3		m_verticalPositions[2];
	int			m_numVerticalContacts = 0;

	// Horizontal
	float		m_worstHorizontalPen = FLT_MAX;

	// Edge
	float		m_worstEdgePen = FLT_MAX;

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------