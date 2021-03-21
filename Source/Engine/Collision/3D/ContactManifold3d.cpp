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
		else if (m_colB->IsOfType<BoxCollider3d>())
		{
			BoxCollider3d* colB = m_colB->GetAsType<BoxCollider3d>();
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
	else if (m_colA->IsOfType<BoxCollider3d>())
	{
		BoxCollider3d* colA = m_colA->GetAsType<BoxCollider3d>();

		if (m_colB->IsOfType<SphereCollider3d>())
		{
			SphereCollider3d* colB = m_colB->GetAsType<SphereCollider3d>();
			m_broadphaseResult = CollisionUtils3d::Collide(colA, colB);
		}
		else if (m_colB->IsOfType<BoxCollider3d>())
		{
			BoxCollider3d* colB = m_colB->GetAsType<BoxCollider3d>();
			m_broadphaseResult = CollisionUtils3d::Collide(colA, colB);

			if (m_broadphaseResult.m_collisionFound)
			{
				m_broadphaseResult = CollisionUtils3d::Collide(colA, colB);
			}
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
		else if (m_colB->IsOfType<BoxCollider3d>())
		{
			BoxCollider3d* colB = m_colB->GetAsType<BoxCollider3d>();
			m_broadphaseResult = CollisionUtils3d::Collide(colB, colA);
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
		else if (m_colB->IsOfType<BoxCollider3d>())
		{
			BoxCollider3d* colB = m_colB->GetAsType<BoxCollider3d>();
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
