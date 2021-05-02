///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: February 20th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/DevConsole.h"
#include "Engine/Core/EngineCommon.h"
#include "Engine/Render/Renderable.h"
#include "Engine/Render/RenderContext.h"
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
	Shader* shader = g_resourceSystem->CreateOrGetShader("Data/Shader/default_opaque.shader");
	g_renderContext->DrawTransform(m_transform, 1.0f, shader);
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
	Matrix4 mat = m_transform.GetLocalToWorldMatrix();
	Vector3 startWs = mat.TransformPosition(m_start);
	Vector3 endWs = mat.TransformPosition(m_end);

	g_renderContext->DrawLine3D(startWs, endWs, Rgba::WHITE, g_debugRenderSystem->GetShader());
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
	m_transform.position = position;
}


//-------------------------------------------------------------------------------------------------
void DebugRenderText3D::Render() const
{
	UNIMPLEMENTED();
	//Font* font = g_resourceSystem->CreateOrGetFont("Data/Font/Prototype.ttf");
}
