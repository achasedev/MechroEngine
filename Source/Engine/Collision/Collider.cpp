///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: May 9th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Collision/Collider.h"
#include "Engine/Core/EngineCommon.h"
#include "Engine/Core/Entity.h"
#include "Engine/Core/Rgba.h"
#include "Engine/Render/Debug/DebugRenderSystem.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
RTTI_TYPE_DEFINE(Collider);
RTTI_TYPE_DEFINE(SphereCollider);
RTTI_TYPE_DEFINE(HalfSpaceCollider);
RTTI_TYPE_DEFINE(BoxCollider);
RTTI_TYPE_DEFINE(CapsuleCollider);

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
Collider::Collider(Entity* owningEntity)
	: entity(owningEntity)
{
}


//-------------------------------------------------------------------------------------------------
bool Collider::OwnerHasRigidBody() const
{
	return (entity->rigidBody != nullptr);
}


//-------------------------------------------------------------------------------------------------
RigidBody* Collider::GetOwnerRigidBody() const
{
	return entity->rigidBody;
}


//-------------------------------------------------------------------------------------------------
SphereCollider::SphereCollider(Entity* owningEntity, const Sphere3D& sphereLs)
	: TypedCollider(owningEntity, sphereLs)
{
}


//-------------------------------------------------------------------------------------------------
void SphereCollider::DebugRender(const Rgba& color) const
{
	DebugDrawSphere(m_dataLs.center, m_dataLs.radius, color, 0.f, &entity->transform);
}


//-------------------------------------------------------------------------------------------------
Sphere3D SphereCollider::GetDataInWorldSpace() const
{
	Vector3 centerWs = entity->transform.TransformPosition(m_dataLs.center);
	return Sphere3D(centerWs, m_dataLs.radius);
}


//-------------------------------------------------------------------------------------------------
HalfSpaceCollider::HalfSpaceCollider(Entity* owningEntity, const Plane3& planeLs)
	: TypedCollider(owningEntity, planeLs)
{
}


//-------------------------------------------------------------------------------------------------
void HalfSpaceCollider::DebugRender(const Rgba& color) const
{
	// TODO:
}


//-------------------------------------------------------------------------------------------------
Plane3 HalfSpaceCollider::GetDataInWorldSpace() const
{
	Vector3 normalWs = entity->transform.TransformDirection(m_dataLs.m_normal);
	Vector3 positionLs = m_dataLs.m_normal * m_dataLs.m_distance;
	Vector3 positionWs = entity->transform.TransformPosition(positionLs);

	return Plane3(normalWs, positionWs);
}


//-------------------------------------------------------------------------------------------------
BoxCollider::BoxCollider(Entity* owningEntity, const OBB3& boxLs)
	: TypedCollider(owningEntity, boxLs)
{
}


//-------------------------------------------------------------------------------------------------
void BoxCollider::DebugRender(const Rgba& color) const
{
	DebugDrawCube(m_dataLs.center, m_dataLs.extents, color, 0.f, &entity->transform);
}


//-------------------------------------------------------------------------------------------------
OBB3 BoxCollider::GetDataInWorldSpace() const
{
	Vector3 centerWs = entity->transform.TransformPosition(m_dataLs.center);
	Quaternion rotationWs = entity->transform.rotation * m_dataLs.rotation;

	return OBB3(centerWs, m_dataLs.extents, rotationWs);
}


//-------------------------------------------------------------------------------------------------
CapsuleCollider::CapsuleCollider(Entity* owningEntity, const Capsule3D& capsuleLs)
	: TypedCollider(owningEntity, capsuleLs)
{
}


//-------------------------------------------------------------------------------------------------
void CapsuleCollider::DebugRender(const Rgba& color) const
{
	DebugDrawSphere(m_dataLs.start, m_dataLs.radius, color, 0.f, &entity->transform);
	DebugDrawSphere(m_dataLs.end, m_dataLs.radius, color, 0.f, &entity->transform);
}


//-------------------------------------------------------------------------------------------------
Capsule3D CapsuleCollider::GetDataInWorldSpace() const
{
	Vector3 startWs = entity->transform.TransformPosition(m_dataLs.start);
	Vector3 endWs = entity->transform.TransformPosition(m_dataLs.end);

	return Capsule3D(startWs, endWs, m_dataLs.radius);
}
