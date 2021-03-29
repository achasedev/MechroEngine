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

#include "Engine/Render/Core/RenderContext.h"
//-------------------------------------------------------------------------------------------------
void ContactManifold3d::DebugRender(Material* material) const
{
	Rgba color = (m_broadphaseResult.m_collisionFound ? Rgba::RED : Rgba::WHITE);

	m_colA->DebugRender(material, color);
	m_colB->DebugRender(material, color * 0.5f);

	Plane3 plane(m_broadphaseResult.m_direction * m_broadphaseResult.m_penetration, m_broadphaseResult.m_position);
	g_renderContext->DrawPlane3(plane, material, Rgba(255, 255, 255, 100));
}
