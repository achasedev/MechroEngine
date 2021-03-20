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

}


//-------------------------------------------------------------------------------------------------
void ContactManifold3d::Collide()
{
	if (m_colliderA->IsOfType<SphereCollider3d>())
	{
		SphereCollider3d* colA = m_colliderA->GetAsType<SphereCollider3d>();

		if (m_colliderB->IsOfType<SphereCollider3d>())
		{
			SphereCollider3d* colB = m_colliderB->GetAsType<SphereCollider3d>();
			m_broadphaseResult = CollisionUtils3d::Collide(colA, colB);
		}
		else if (m_colliderB->IsOfType<BoxCollider3d>())
		{
			BoxCollider3d* colB = m_colliderB->GetAsType<BoxCollider3d>();
			m_broadphaseResult = CollisionUtils3d::Collide(colA, colB);
		}
		else if (m_colliderB->IsOfType<CapsuleCollider3d>())
		{
			CapsuleCollider3d* colB = m_colliderB->GetAsType<CapsuleCollider3d>();
			m_broadphaseResult = CollisionUtils3d::Collide(colA, colB);
		}
		else if (m_colliderB->IsOfType<PolytopeCollider3d>())
		{
			PolytopeCollider3d* colB = m_colliderB->GetAsType<PolytopeCollider3d>();
			m_broadphaseResult = CollisionUtils3d::Collide(colA, colB);
		}
	}
	else if (m_colliderA->IsOfType<BoxCollider3d>())
	{
		BoxCollider3d* colA = m_colliderA->GetAsType<BoxCollider3d>();

		if (m_colliderB->IsOfType<SphereCollider3d>())
		{
			SphereCollider3d* colB = m_colliderB->GetAsType<SphereCollider3d>();
			m_broadphaseResult = CollisionUtils3d::Collide(colA, colB);
		}
		else if (m_colliderB->IsOfType<BoxCollider3d>())
		{
			BoxCollider3d* colB = m_colliderB->GetAsType<BoxCollider3d>();
			m_broadphaseResult = CollisionUtils3d::Collide(colA, colB);
		}
		else if (m_colliderB->IsOfType<CapsuleCollider3d>())
		{
			CapsuleCollider3d* colB = m_colliderB->GetAsType<CapsuleCollider3d>();
			m_broadphaseResult = CollisionUtils3d::Collide(colA, colB);
		}
		else if (m_colliderB->IsOfType<PolytopeCollider3d>())
		{
			PolytopeCollider3d* colB = m_colliderB->GetAsType<PolytopeCollider3d>();
			m_broadphaseResult = CollisionUtils3d::Collide(colA, colB);
		}
	}
	else if (m_colliderA->IsOfType<CapsuleCollider3d>())
	{
		CapsuleCollider3d* colA = m_colliderA->GetAsType<CapsuleCollider3d>();

		if (m_colliderB->IsOfType<SphereCollider3d>())
		{
			SphereCollider3d* colB = m_colliderB->GetAsType<SphereCollider3d>();
			m_broadphaseResult = CollisionUtils3d::Collide(colA, colB);
		}
		else if (m_colliderB->IsOfType<BoxCollider3d>())
		{
			BoxCollider3d* colB = m_colliderB->GetAsType<BoxCollider3d>();
			m_broadphaseResult = CollisionUtils3d::Collide(colB, colA);
		}
		else if (m_colliderB->IsOfType<CapsuleCollider3d>())
		{
			CapsuleCollider3d* colB = m_colliderB->GetAsType<CapsuleCollider3d>();
			m_broadphaseResult = CollisionUtils3d::Collide(colA, colB);
		}
		else if (m_colliderB->IsOfType<PolytopeCollider3d>())
		{
			PolytopeCollider3d* colB = m_colliderB->GetAsType<PolytopeCollider3d>();
			m_broadphaseResult = CollisionUtils3d::Collide(colA, colB);
		}
	}
	else if (m_colliderA->IsOfType<PolytopeCollider3d>())
	{
		PolytopeCollider3d* colA = m_colliderA->GetAsType<PolytopeCollider3d>();

		if (m_colliderB->IsOfType<SphereCollider3d>())
		{
			SphereCollider3d* colB = m_colliderB->GetAsType<SphereCollider3d>();
			m_broadphaseResult = CollisionUtils3d::Collide(colB, colA);
		}
		else if (m_colliderB->IsOfType<BoxCollider3d>())
		{
			BoxCollider3d* colB = m_colliderB->GetAsType<BoxCollider3d>();
			m_broadphaseResult = CollisionUtils3d::Collide(colB, colA);
		}
		else if (m_colliderB->IsOfType<CapsuleCollider3d>())
		{
			CapsuleCollider3d* colB = m_colliderB->GetAsType<CapsuleCollider3d>();
			m_broadphaseResult = CollisionUtils3d::Collide(colB, colA);
		}
		else if (m_colliderB->IsOfType<PolytopeCollider3d>())
		{
			PolytopeCollider3d* colB = m_colliderB->GetAsType<PolytopeCollider3d>();
			m_broadphaseResult = CollisionUtils3d::Collide(colA, colB);
		}
	}
}
