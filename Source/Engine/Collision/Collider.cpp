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
RTTI_TYPE_DEFINE(PlaneCollider);
RTTI_TYPE_DEFINE(BoxCollider);
RTTI_TYPE_DEFINE(CapsuleCollider);
RTTI_TYPE_DEFINE(CylinderCollider);

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------
const DebugRenderOptions Collider::DEFAULT_COLLIDER_RENDER_OPTIONS = DebugRenderOptions(Rgba::GREEN, Rgba::GREEN, FLT_MAX, nullptr, FILL_MODE_WIREFRAME, CULL_MODE_NONE, DEBUG_RENDER_MODE_IGNORE_DEPTH);


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
Collider::Collider(Entity* owningEntity)
	: m_entity(owningEntity)
{
}


//-------------------------------------------------------------------------------------------------
void Collider::HideDebug()
{
	if (m_debugRenderHandle != INVALID_DEBUG_RENDER_OBJECT_HANDLE)
	{
		g_debugRenderSystem->DestroyObject(m_debugRenderHandle);
		m_debugRenderHandle = INVALID_DEBUG_RENDER_OBJECT_HANDLE;
	}
}


//-------------------------------------------------------------------------------------------------
bool Collider::OwnerHasRigidBody() const
{
	return (m_entity->rigidBody != nullptr);
}


//-------------------------------------------------------------------------------------------------
RigidBody* Collider::GetOwnerRigidBody() const
{
	return m_entity->rigidBody;
}


//-------------------------------------------------------------------------------------------------
SphereCollider::SphereCollider(Entity* owningEntity, const Sphere3D& sphereLs)
	: TypedCollider(owningEntity, sphereLs)
{
}


//-------------------------------------------------------------------------------------------------
void SphereCollider::ShowDebug()
{
	if (m_debugRenderHandle == INVALID_DEBUG_RENDER_OBJECT_HANDLE)
	{
		DebugRenderOptions options = DEFAULT_COLLIDER_RENDER_OPTIONS;
		options.m_parentTransform = &m_entity->transform;

		m_debugRenderHandle = DebugDrawSphere(m_dataLs, options);
	}
}


//-------------------------------------------------------------------------------------------------
Sphere3D SphereCollider::GetDataInWorldSpace() const
{
	Vector3 centerWs = m_entity->transform.TransformPosition(m_dataLs.center);
	return Sphere3D(centerWs, m_dataLs.radius * m_entity->transform.scale.x);
}


//-------------------------------------------------------------------------------------------------
HalfSpaceCollider::HalfSpaceCollider(Entity* owningEntity, const Plane3& planeLs)
	: TypedCollider(owningEntity, planeLs)
{
}


//-------------------------------------------------------------------------------------------------
void HalfSpaceCollider::ShowDebug()
{
	if (m_debugRenderHandle == INVALID_DEBUG_RENDER_OBJECT_HANDLE)
	{
		DebugRenderOptions options = DEFAULT_COLLIDER_RENDER_OPTIONS;
		options.m_parentTransform = &m_entity->transform;

		m_debugRenderHandle = DebugDrawPlane(m_dataLs, options);
	}
}


//-------------------------------------------------------------------------------------------------
Plane3 HalfSpaceCollider::GetDataInWorldSpace() const
{
	Vector3 normalWs = m_entity->transform.TransformDirection(m_dataLs.m_normal);
	Vector3 positionLs = m_dataLs.m_normal * m_dataLs.m_distance;
	Vector3 positionWs = m_entity->transform.TransformPosition(positionLs);

	return Plane3(normalWs, positionWs);
}


//-------------------------------------------------------------------------------------------------
BoxCollider::BoxCollider(Entity* owningEntity, const OBB3& boxLs)
	: TypedCollider(owningEntity, boxLs)
{
}


//-------------------------------------------------------------------------------------------------
void BoxCollider::ShowDebug()
{
	if (m_debugRenderHandle == INVALID_DEBUG_RENDER_OBJECT_HANDLE)
	{
		DebugRenderOptions options = DEFAULT_COLLIDER_RENDER_OPTIONS;
		options.m_parentTransform = &m_entity->transform;

		m_debugRenderHandle = DebugDrawBox(m_dataLs, options);
	}
}


//-------------------------------------------------------------------------------------------------
OBB3 BoxCollider::GetDataInWorldSpace() const
{
	Vector3 centerWs = m_entity->transform.TransformPosition(m_dataLs.center);
	Quaternion rotationWs = m_entity->transform.rotation * m_dataLs.rotation;
	Vector3 extentsWs = m_dataLs.extents * m_entity->transform.scale;

	return OBB3(centerWs, extentsWs, rotationWs);
}


//-------------------------------------------------------------------------------------------------
CapsuleCollider::CapsuleCollider(Entity* owningEntity, const Capsule3D& capsuleLs)
	: TypedCollider(owningEntity, capsuleLs)
{
}


//-------------------------------------------------------------------------------------------------
void CapsuleCollider::ShowDebug()
{
	if (m_debugRenderHandle == INVALID_DEBUG_RENDER_OBJECT_HANDLE)
	{
		DebugRenderOptions options = DEFAULT_COLLIDER_RENDER_OPTIONS;
		options.m_parentTransform = &m_entity->transform;

		m_debugRenderHandle = DebugDrawCapsule(m_dataLs, options);
	}
}


//-------------------------------------------------------------------------------------------------
Capsule3D CapsuleCollider::GetDataInWorldSpace() const
{
	Vector3 startWs = m_entity->transform.TransformPosition(m_dataLs.start);
	Vector3 endWs = m_entity->transform.TransformPosition(m_dataLs.end);

	return Capsule3D(startWs, endWs, m_dataLs.radius * m_entity->transform.scale.x); // It should be that x == z
}


//-------------------------------------------------------------------------------------------------
PlaneCollider::PlaneCollider(Entity* owningEntity, const Plane3& planeLs)
	: TypedCollider(owningEntity, planeLs)
{
}


//-------------------------------------------------------------------------------------------------
void PlaneCollider::ShowDebug()
{
	if (m_debugRenderHandle == INVALID_DEBUG_RENDER_OBJECT_HANDLE)
	{
		DebugRenderOptions options = DEFAULT_COLLIDER_RENDER_OPTIONS;
		options.m_parentTransform = &m_entity->transform;

		m_debugRenderHandle = DebugDrawPlane(m_dataLs, options);
	}
}


//-------------------------------------------------------------------------------------------------
Plane3 PlaneCollider::GetDataInWorldSpace() const
{
	Vector3 normalWs = m_entity->transform.TransformDirection(m_dataLs.m_normal);
	Vector3 positionLs = m_dataLs.m_normal * m_dataLs.m_distance;
	Vector3 positionWs = m_entity->transform.TransformPosition(positionLs);

	return Plane3(normalWs, positionWs);
}


//-------------------------------------------------------------------------------------------------
CylinderCollider::CylinderCollider(Entity* owningEntity, const Cylinder3D& cylinderLs)
	: TypedCollider(owningEntity, cylinderLs)
{
}


//-------------------------------------------------------------------------------------------------
void CylinderCollider::ShowDebug()
{
	if (m_debugRenderHandle == INVALID_DEBUG_RENDER_OBJECT_HANDLE)
	{
		DebugRenderOptions options = DEFAULT_COLLIDER_RENDER_OPTIONS;
		options.m_parentTransform = &m_entity->transform;

		DebugDrawCylinder(m_dataLs, options);
	}
}


//-------------------------------------------------------------------------------------------------
Cylinder3D CylinderCollider::GetDataInWorldSpace() const
{
	Vector3 bottomWs = m_entity->transform.TransformPosition(m_dataLs.m_bottom);
	Vector3 topWs = m_entity->transform.TransformPosition(m_dataLs.m_top);

	return Cylinder3D(bottomWs, topWs, m_dataLs.m_radius * m_entity->transform.scale.x); // It should be that x == z
}
