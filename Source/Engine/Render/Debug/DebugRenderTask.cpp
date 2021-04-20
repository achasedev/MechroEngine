///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: February 20th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/DevConsole.h"
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Physics/3D/RigidBody3D.h"
#include "Engine/Render/Core/Renderable.h"
#include "Engine/Render/Core/RenderContext.h"
#include "Engine/Render/Debug/DebugRenderTask.h"
#include "Engine/Resource/ResourceSystem.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------
RTTI_TYPE_DEFINE(DebugRenderTask);

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
DebugRenderTask::DebugRenderTask(const DebugRenderOptions& options)
	: m_options(options)
{
	m_transform.SetParentTransform(options.m_parentTransform);
	m_timer.SetInterval(options.m_lifetime);
}


//-------------------------------------------------------------------------------------------------
bool DebugRenderTask::IsFinished() const
{
	return m_timer.HasIntervalElapsed();
}


//-------------------------------------------------------------------------------------------------
DebugRenderTransform::DebugRenderTransform(const Transform& transform, const DebugRenderOptions& options)
	: DebugRenderTask(options)
{
	m_transform = transform;
	m_transform.SetParentTransform(options.m_parentTransform); // Need to re-set this here, since the above line overwrites it...
}


//-------------------------------------------------------------------------------------------------
void DebugRenderTransform::Render() const
{
	g_renderContext->DrawTransform(m_transform, 1.0f);
}


//-------------------------------------------------------------------------------------------------
DebugRenderLine3D::DebugRenderLine3D(const Vector3& start, const Vector3& end, const DebugRenderOptions& options)
	: DebugRenderTask(options)
	, m_start(start)
	, m_end(end)
{
}


//-------------------------------------------------------------------------------------------------
void DebugRenderLine3D::Render() const
{
	Matrix44 mat = m_transform.GetLocalToWorldMatrix();
	Vector3 startWs = mat.TransformPoint(m_start).xyz();
	Vector3 endWs = mat.TransformPoint(m_end).xyz();

	g_renderContext->DrawLine3D(startWs, endWs, m_options.m_color);
}


//-------------------------------------------------------------------------------------------------
DebugRenderRigidBody3D::DebugRenderRigidBody3D(const RigidBody3D* rigidBody, const DebugRenderOptions& options)
	: DebugRenderTask(options)
	, m_rigidBody(rigidBody)
{
	if (m_options.m_parentTransform != nullptr)
	{
		ConsoleErrorf("RigidBodies shouldn't have parent transforms, so the debug draw for one will ignore the parent transform specified.");
		m_options.m_parentTransform = nullptr;
	}
}


//-------------------------------------------------------------------------------------------------
void DebugRenderRigidBody3D::Render() const
{
	const Polygon3d* bodyShape = m_rigidBody->GetWorldShape();
	g_renderContext->DrawWirePolygon3D(*bodyShape, m_options.m_color);
	g_renderContext->DrawPoint3D(m_rigidBody->GetCenterOfMassWs(), 0.25f, m_options.m_color);
}


//-------------------------------------------------------------------------------------------------
DebugRenderPoint3D::DebugRenderPoint3D(const Vector3& position, const DebugRenderOptions& options)
	: DebugRenderTask(options)
{
	m_transform.position = position;
}


//-------------------------------------------------------------------------------------------------
void DebugRenderPoint3D::Render() const
{
	Vector3 posWs = m_transform.GetWorldPosition();
	g_renderContext->DrawPoint3D(posWs, 0.25f, m_options.m_color);
}


//-------------------------------------------------------------------------------------------------
DebugRenderCube::DebugRenderCube(const Vector3& center, const Vector3& extents, const DebugRenderOptions& options)
	: DebugRenderTask(options)
{
	m_transform.position = center;
	m_transform.scale = 2.f * extents; // Base mesh has 0.5 extents, so scale up the extents by 2 to compensate
}


//-------------------------------------------------------------------------------------------------
void DebugRenderCube::Render() const
{
	Mesh* cubeMesh = g_resourceSystem->CreateOrGetMesh("unit_cube");
	Material* debugMaterial = g_resourceSystem->CreateOrGetMaterial("Data/Material/debug.material");

	Renderable rend;
	rend.AddDraw(cubeMesh, debugMaterial, m_transform.GetLocalToWorldMatrix());
	
	g_renderContext->DrawRenderable(rend);
}
