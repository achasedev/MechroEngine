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
#include "Engine/Render/Debug/DebugRenderSystem.h"
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
void DebugRenderTask::PreRender() const
{
	// Update the UBO
	DebugBufferData data;
	data.m_colorTint = m_options.m_color.GetAsFloats();
	g_debugRenderSystem->UpdateUniformBuffer(data);
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
	g_renderContext->DrawTransform(m_transform, 1.0f, g_debugRenderSystem->GetShader());
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

	g_renderContext->DrawLine3D(startWs, endWs, Rgba::WHITE, g_debugRenderSystem->GetShader());
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
	g_renderContext->DrawWirePolygon3D(*bodyShape, Rgba::WHITE, g_debugRenderSystem->GetShader());
	g_renderContext->DrawPoint3D(m_rigidBody->GetCenterOfMassWs(), 0.25f, Rgba::WHITE, g_debugRenderSystem->GetShader());
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
	g_renderContext->DrawPoint3D(posWs, 0.25f, Rgba::WHITE, g_debugRenderSystem->GetShader());
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


//-------------------------------------------------------------------------------------------------
DebugRenderSphere::DebugRenderSphere(const Vector3& center, float radius, const DebugRenderOptions& options)
	: DebugRenderTask(options)
{
	m_transform.position = center;
	m_transform.scale = Vector3(radius);
}


//-------------------------------------------------------------------------------------------------
void DebugRenderSphere::Render() const
{
	Mesh* sphereMesh = g_resourceSystem->CreateOrGetMesh("unit_sphere");
	Material* debugMaterial = g_resourceSystem->CreateOrGetMaterial("Data/Material/debug.material");

	Renderable rend;
	rend.AddDraw(sphereMesh, debugMaterial, m_transform.GetLocalToWorldMatrix());

	g_renderContext->DrawRenderable(rend);
}


//-------------------------------------------------------------------------------------------------
DebugRenderText3D::DebugRenderText3D(const char* text, const Vector3& position, const DebugRenderOptions& options)
	: DebugRenderTask(options)
	, m_text(text)
{
}


//-------------------------------------------------------------------------------------------------
void DebugRenderText3D::Render() const
{
	Font* font = g_resourceSystem->CreateOrGetFont("Data/Font/Prototype.ttf");
}
