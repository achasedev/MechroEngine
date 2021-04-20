///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 18th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Collision/3D/Collider3d.h"
#include "Engine/Collision/3D/ContactManifold3d.h"
#include "Engine/Collision/3D/CollisionUtils3d.h"
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Framework/Rgba.h"
#include "Engine/Physics/3D/PhysicsSystem3D.h"

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
ContactManifold3d::ContactManifold3d(Collider3d* a, Collider3d* b)
{
	// Establish an ordering on manifolds when searching for them
	if (a < b)
	{
		m_colA = a;
		m_colB = b;
	}
	else
	{
		m_colA = b;
		m_colB = a;
	}
}

#include "Engine/Math/MathUtils.h"
//-------------------------------------------------------------------------------------------------
void ContactManifold3d::Collide()
{
	if (m_colA->IsOfType<SphereCollider3d>())
	{
		SphereCollider3d* colA = m_colA->GetAsType<SphereCollider3d>();

		if (m_colB->IsOfType<SphereCollider3d>())
		{
			SphereCollider3d* colB = m_colB->GetAsType<SphereCollider3d>();
			m_broadphaseResult = CollisionUtils3d::Collide(colA, colB);
		}
		else if (m_colB->IsOfType<CapsuleCollider3d>())
		{
			CapsuleCollider3d* colB = m_colB->GetAsType<CapsuleCollider3d>();
			m_broadphaseResult = CollisionUtils3d::Collide(colA, colB);
		}
		else if (m_colB->IsOfType<PolytopeCollider3d>())
		{
			PolytopeCollider3d* colB = m_colB->GetAsType<PolytopeCollider3d>();
			m_broadphaseResult = CollisionUtils3d::Collide(colA, colB);
		}
	}
	else if (m_colA->IsOfType<CapsuleCollider3d>())
	{
		CapsuleCollider3d* colA = m_colA->GetAsType<CapsuleCollider3d>();

		if (m_colB->IsOfType<SphereCollider3d>())
		{
			SphereCollider3d* colB = m_colB->GetAsType<SphereCollider3d>();
			m_broadphaseResult = CollisionUtils3d::Collide(colA, colB);
		}
		else if (m_colB->IsOfType<CapsuleCollider3d>())
		{
			CapsuleCollider3d* colB = m_colB->GetAsType<CapsuleCollider3d>();
			m_broadphaseResult = CollisionUtils3d::Collide(colA, colB);
		}
		else if (m_colB->IsOfType<PolytopeCollider3d>())
		{
			PolytopeCollider3d* colB = m_colB->GetAsType<PolytopeCollider3d>();
			m_broadphaseResult = CollisionUtils3d::Collide(colA, colB);
		}
	}
	else if (m_colA->IsOfType<PolytopeCollider3d>())
	{
		PolytopeCollider3d* colA = m_colA->GetAsType<PolytopeCollider3d>();

		if (m_colB->IsOfType<SphereCollider3d>())
		{
			SphereCollider3d* colB = m_colB->GetAsType<SphereCollider3d>();
			m_broadphaseResult = CollisionUtils3d::Collide(colB, colA);
		}
		else if (m_colB->IsOfType<CapsuleCollider3d>())
		{
			CapsuleCollider3d* colB = m_colB->GetAsType<CapsuleCollider3d>();
			m_broadphaseResult = CollisionUtils3d::Collide(colB, colA);
		}
		else if (m_colB->IsOfType<PolytopeCollider3d>())
		{
			PolytopeCollider3d* colB = m_colB->GetAsType<PolytopeCollider3d>();
			m_broadphaseResult = CollisionUtils3d::Collide(colA, colB);
		}
	}
}


//-------------------------------------------------------------------------------------------------
void ContactManifold3d::GenerateContacts()
{
	m_numContacts = 0;

	if (m_colA->IsOfType<SphereCollider3d>())
	{
		SphereCollider3d* colA = m_colA->GetAsType<SphereCollider3d>();

		if (m_colB->IsOfType<SphereCollider3d>())
		{
			SphereCollider3d* colB = m_colB->GetAsType<SphereCollider3d>();
			m_numContacts = CollisionUtils3d::CalculateContacts(colA, colB, m_broadphaseResult, m_contacts);
		}
		else if (m_colB->IsOfType<CapsuleCollider3d>())
		{
			CapsuleCollider3d* colB = m_colB->GetAsType<CapsuleCollider3d>();
			m_numContacts = CollisionUtils3d::CalculateContacts(colA, colB, m_broadphaseResult, m_contacts);
		}
		else if (m_colB->IsOfType<PolytopeCollider3d>())
		{
			PolytopeCollider3d* colB = m_colB->GetAsType<PolytopeCollider3d>();
			m_numContacts = CollisionUtils3d::CalculateContacts(colA, colB, m_broadphaseResult, m_contacts);
		}
	}
	else if (m_colA->IsOfType<CapsuleCollider3d>())
	{
		CapsuleCollider3d* colA = m_colA->GetAsType<CapsuleCollider3d>();

		if (m_colB->IsOfType<SphereCollider3d>())
		{
			SphereCollider3d* colB = m_colB->GetAsType<SphereCollider3d>();
			m_numContacts = CollisionUtils3d::CalculateContacts(colA, colB, m_broadphaseResult, m_contacts);
		}
		else if (m_colB->IsOfType<CapsuleCollider3d>())
		{
			CapsuleCollider3d* colB = m_colB->GetAsType<CapsuleCollider3d>();
			m_numContacts = CollisionUtils3d::CalculateContacts(colA, colB, m_broadphaseResult, m_contacts);
		}
		else if (m_colB->IsOfType<PolytopeCollider3d>())
		{
			PolytopeCollider3d* colB = m_colB->GetAsType<PolytopeCollider3d>();
			m_numContacts = CollisionUtils3d::CalculateContacts(colA, colB, m_broadphaseResult, m_contacts);
		}
	}
	else if (m_colA->IsOfType<PolytopeCollider3d>())
	{
		PolytopeCollider3d* colA = m_colA->GetAsType<PolytopeCollider3d>();

		if (m_colB->IsOfType<SphereCollider3d>())
		{
			SphereCollider3d* colB = m_colB->GetAsType<SphereCollider3d>();
			m_numContacts = CollisionUtils3d::CalculateContacts(colA, colB, m_broadphaseResult, m_contacts);
		}
		else if (m_colB->IsOfType<CapsuleCollider3d>())
		{
			CapsuleCollider3d* colB = m_colB->GetAsType<CapsuleCollider3d>();
			m_numContacts = CollisionUtils3d::CalculateContacts(colA, colB, m_broadphaseResult, m_contacts);
		}
		else if (m_colB->IsOfType<PolytopeCollider3d>())
		{
			PolytopeCollider3d* colB = m_colB->GetAsType<PolytopeCollider3d>();
			m_numContacts = CollisionUtils3d::CalculateContacts(colA, colB, m_broadphaseResult, m_contacts);
		}
	}
}


//-------------------------------------------------------------------------------------------------
void ContactManifold3d::UpdateContacts(const ContactPoint3D* contacts, int numContacts)
{
	// Where the final result will be stored temporarily
	ContactPoint3D mergedContacts[MAX_CONTACTS];

	for (int newContactIndex = 0U; newContactIndex < numContacts; ++newContactIndex)
	{
		const ContactPoint3D* newContact = contacts + newContactIndex;

		// Search to see if we already have info for this contact stored from last frames result
		ContactPoint3D* matchingOldContact = nullptr;
		for (int oldContactIndex = 0; oldContactIndex < m_numContacts; ++oldContactIndex)
		{
			ContactPoint3D* oldContact = m_contacts + oldContactIndex;

			if (newContact->m_id == oldContact->m_id)
			{
				matchingOldContact = oldContact;
				break;
			}
		}

		if (matchingOldContact != nullptr)
		{
			// Overwrite our old contact info with the new stuff
			ContactPoint3D* mergeContact = mergedContacts + newContactIndex;
			*mergeContact = *newContact;

			// If we want to reuse the last accumulation state of the contact, copy it now
			if (PhysicsSystem3D::WARM_START_ACCUMULATIONS)
			{
				mergeContact->m_accNormalImpulse = matchingOldContact->m_accNormalImpulse;
				mergeContact->m_accTangentImpulse = matchingOldContact->m_accTangentImpulse;
				mergeContact->m_normalBiasImpulse = matchingOldContact->m_normalBiasImpulse;
			}
			else
			{
				mergeContact->m_accNormalImpulse = 0.f;
				mergeContact->m_accTangentImpulse = 0.f;
				mergeContact->m_normalBiasImpulse = 0.f;
			}
		}
		else
		{
			// Brand new contact, just add it to merged contacts
			mergedContacts[newContactIndex] = contacts[newContactIndex];
		}

	}

	// Done merging, update the arbiter's contact data
	for (int newContactIndex = 0U; newContactIndex < numContacts; ++newContactIndex)
	{
		m_contacts[newContactIndex] = mergedContacts[newContactIndex];
	}

	m_numContacts = numContacts;
}


#include "Engine/Render/Core/RenderContext.h"
//-------------------------------------------------------------------------------------------------
void ContactManifold3d::DebugRender() const
{
	Rgba color = (m_broadphaseResult.m_collisionFound ? Rgba::RED : Rgba::WHITE);
	float aColorScale = (m_colA == m_broadphaseResult.m_refCol ? 1.0f : 0.5f);
	float bColorScale = (m_colB == m_broadphaseResult.m_refCol ? 1.0f : 0.5f);

	m_colA->DebugRender(color * aColorScale);
	m_colB->DebugRender(color * bColorScale);

	if (m_numContacts > 0)
	{
		for (int contactIndex = 0; contactIndex < m_numContacts; ++contactIndex)
		{
			g_renderContext->DrawPoint3D(m_contacts[contactIndex].m_position, 0.10f, Rgba::YELLOW);
			g_renderContext->DrawLine3D(m_contacts[contactIndex].m_position, m_contacts[contactIndex].m_position + 0.25f * m_contacts[contactIndex].m_normal, Rgba::GREEN);
		}
	}
}


//-------------------------------------------------------------------------------------------------
Entity* ContactManifold3d::GetReferenceEntity() const
{
	if (!m_broadphaseResult.m_collisionFound)
		return nullptr;
	
	return m_broadphaseResult.m_refCol->GetOwningEntity();
}


//-------------------------------------------------------------------------------------------------
Entity* ContactManifold3d::GetIncidentEntity() const
{
	if (!m_broadphaseResult.m_collisionFound)
		return nullptr;

	return m_broadphaseResult.m_incCol->GetOwningEntity();
}
