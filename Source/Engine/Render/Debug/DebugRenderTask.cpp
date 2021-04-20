///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: February 20th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Physics/3D/RigidBody3D.h"
#include "Engine/Render/Core/RenderContext.h"
#include "Engine/Render/Debug/DebugRenderTask.h"

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
	m_timer.SetInterval(options.m_lifetime);
}


//-------------------------------------------------------------------------------------------------
bool DebugRenderTask::IsFinished() const
{
	return m_timer.HasIntervalElapsed();
}


//-------------------------------------------------------------------------------------------------
DebugRenderTransform::DebugRenderTransform(const Transform& transform, bool freezeTransform, const DebugRenderOptions& options)
	: DebugRenderTask(options)
{
	if (freezeTransform)
	{
		// Just copy it, so it doesn't move with the owner
		m_transform = transform;
	}
	else
	{
		// Otherwise keep this transform identity and parent it to this transform, so it will move
		m_transform.SetParentTransform(&transform);
	}
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
	g_renderContext->DrawLine3D(m_start, m_end, m_options.m_color);
}


//-------------------------------------------------------------------------------------------------
DebugRenderRigidBody3D::DebugRenderRigidBody3D(const RigidBody3D* rigidBody, const DebugRenderOptions& options)
	: DebugRenderTask(options)
	, m_rigidBody(rigidBody)
{
}


//-------------------------------------------------------------------------------------------------
void DebugRenderRigidBody3D::Render() const
{
	const Polygon3d* bodyShape = m_rigidBody->GetWorldShape();
	g_renderContext->DrawWirePolygon3D(*bodyShape, m_options.m_color);
	g_renderContext->DrawPoint3D(m_rigidBody->GetCenterOfMassWs(), 0.25f, m_options.m_color);
}
