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
#include "Engine/Math/MathUtils.h"
#include "Engine/Render/DrawCall.h"
#include "Engine/Render/Renderable.h"
#include "Engine/Render/RenderContext.h"
#include "Engine/Render/Debug/DebugRenderSystem.h"
#include "Engine/Render/Debug/DebugRenderObject.h"
#include "Engine/Resource/ResourceSystem.h"
#include "Engine/Render/Material/Material.h"
#include "Engine/Render/Texture/Texture2D.h"

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
// Constructor - sets up common parameters shared by all object types
DebugRenderObject::DebugRenderObject(const DebugRenderOptions& options)
	: m_options(options)
	, m_system(g_debugRenderSystem)
{
	m_transform.SetParentTransform(options.m_parentTransform);
	m_timer.SetInterval(options.m_lifetime);

	// Set up the material to use the default system shader
	Shader* shader = m_system->GetDefaultShader();

	m_material = new Material();
	m_material->SetShader(shader);

	// Make sure we use the right textures on the material - code reuse!
	SetFillMode(m_options.m_fillMode);
}


//-------------------------------------------------------------------------------------------------
// Destructor - deletes the materials (and the mesh in select cases)
DebugRenderObject::~DebugRenderObject()
{
	SAFE_DELETE(m_material);

	if (m_ownsMesh)
	{
		SAFE_DELETE(m_mesh);
	}
}


//-------------------------------------------------------------------------------------------------
// Draws the object
void DebugRenderObject::Render()
{
	// Update the color tint
	Vector4 tint = CalculateTint();
	m_material->SetProperty(SID("TINT"), tint);

	// Get the matrix
	Matrix4 model = m_transform.GetModelMatrix();
	
	// Set the shader to use the settings that match our options
	Shader* shader = m_material->GetShader();
	shader->SetCullMode(m_options.m_cullMode);
	shader->SetFillMode(m_options.m_fillMode);
	
	switch (m_options.m_debugRenderMode)
	{
	case DEBUG_RENDER_MODE_HIDDEN:
		shader->SetDepthMode(DEPTH_MODE_GREATER_THAN_OR_EQUAL, false);
		break;
	case DEBUG_RENDER_MODE_IGNORE_DEPTH:
		shader->SetDepthMode(DEPTH_MODE_IGNORE_DEPTH, false);
		break;
	case DEBUG_RENDER_MODE_USE_DEPTH:
		shader->SetDepthMode(DEPTH_MODE_LESS_THAN, true);
		break;
	case DEBUG_RENDER_MODE_XRAY:
		shader->SetDepthMode(DEPTH_MODE_LESS_THAN, true); // This is first pass
		break;
	default:
		break;
	}

	// Assemble the draw call
	DrawCall dc;
	dc.SetModelMatrix(model);
	dc.SetMesh(m_mesh);
	dc.SetMaterial(m_material);

	// Draw!
	g_renderContext->Draw(dc);

	// Second draw for xray
	if (m_options.m_debugRenderMode == DEBUG_RENDER_MODE_XRAY)
	{
		m_material->SetProperty(SID("TINT"), Vector4(DEFAULT_XRAY_SCALE * tint.xyz(), tint.w)); // Don't scale alpha
		shader->SetDepthMode(DEPTH_MODE_GREATER_THAN, false);

		g_renderContext->Draw(dc);
	}

}


//-------------------------------------------------------------------------------------------------
bool DebugRenderObject::IsFinished() const
{
	return m_timer.HasIntervalElapsed();
}


//-------------------------------------------------------------------------------------------------
// Sets all the options to the ones specified in the block
void DebugRenderObject::SetOptions(const DebugRenderOptions& options)
{
	SetDebugRenderMode(options.m_debugRenderMode); // Set this first, as switching to/from xray will change behavior below
	SetFillMode(options.m_fillMode);
	SetCullMode(options.m_cullMode);
	SetParentTransform(options.m_parentTransform);
	SetLifetime(options.m_lifetime);
	SetColors(options.m_startColor, options.m_endColor);
}


//-------------------------------------------------------------------------------------------------
// Sets the lifetime to the new value, optionally resetting the timer to live the entire new lifetime
void DebugRenderObject::SetLifetime(float lifetime, bool resetTimer /*= true*/)
{
	m_options.m_lifetime = lifetime;

	if (resetTimer)
	{
		float timeElapsed = m_timer.GetElapsedSeconds();
		m_timer.SetInterval(lifetime);
		m_timer.SetElapsedTime(timeElapsed);
	}
	else
	{
		m_timer.SetInterval(lifetime);
	}
}


//-------------------------------------------------------------------------------------------------
void DebugRenderObject::SetStartColor(const Rgba& color)
{
	m_options.m_startColor = color;
}


//-------------------------------------------------------------------------------------------------
void DebugRenderObject::SetEndColor(const Rgba& color)
{
	m_options.m_endColor = color;
}


//-------------------------------------------------------------------------------------------------
void DebugRenderObject::SetColors(const Rgba& startColor, const Rgba& endColor)
{
	SetStartColor(startColor);
	SetEndColor(endColor);
}


//-------------------------------------------------------------------------------------------------
void DebugRenderObject::SetParentTransform(const Transform* parentTransform)
{
	m_options.m_parentTransform = parentTransform;
	m_transform.SetParentTransform(parentTransform);
}


//-------------------------------------------------------------------------------------------------
void DebugRenderObject::SetFillMode(FillMode fillMode)
{
	m_options.m_fillMode = fillMode;

	Texture2D* texture = m_system->GetDefaultTexture2D();

	// If wireframe, bind a white texture so the lines are flat colors
	if (m_options.m_fillMode == FILL_MODE_WIREFRAME)
	{
		texture = g_resourceSystem->CreateOrGetTexture2D("white");
	}

	m_material->SetAlbedoTextureView(texture->CreateOrGetShaderResourceView());
}


//-------------------------------------------------------------------------------------------------
void DebugRenderObject::SetCullMode(CullMode cullMode)
{
	m_options.m_cullMode = cullMode;
}


//-------------------------------------------------------------------------------------------------
// Sets the debug render depth behavior; If switching to/from xray then an additional draw is added/removed
void DebugRenderObject::SetDebugRenderMode(DebugRenderMode mode)
{
	m_options.m_debugRenderMode = mode;
}


//-------------------------------------------------------------------------------------------------
// Sets the mesh on the renderable to draw; Will delete the mesh on destruction if it is flagged as owning it
void DebugRenderObject::SetMesh(Mesh* mesh, bool ownsMesh)
{
	// Clean up any old mesh on the object
	if (m_mesh != nullptr && m_ownsMesh)
	{
		SAFE_DELETE(m_mesh);
	}

	m_mesh = mesh;
	m_ownsMesh = ownsMesh;
}


//-------------------------------------------------------------------------------------------------
// Calculates the tint, which is the interpolation of start and end colors by fraction into lifetime
Vector4 DebugRenderObject::CalculateTint() const
{
	float t = m_timer.GetElapsedTimeNormalized();
	return Interpolate(m_options.m_startColor.GetAsFloats(), m_options.m_endColor.GetAsFloats(), t);
}

//
//
////-------------------------------------------------------------------------------------------------
//DebugRenderTransform::DebugRenderTransform(const Transform& transform, const DebugRenderOptions& options)
//	: DebugRenderTask(options)
//{
//	m_transform = transform;
//	m_transform.SetParentTransform(options.m_parentTransform); // Need to re-set this here, since the above line overwrites it...
//}
//
//
////-------------------------------------------------------------------------------------------------
//void DebugRenderTransform::Render() const
//{
//	Shader* shader = g_resourceSystem->CreateOrGetShader("Data/Shader/default_opaque.shader");
//	g_renderContext->DrawTransform(m_transform, 1.0f, shader);
//}
//
//
////-------------------------------------------------------------------------------------------------
//DebugRenderLine3D::DebugRenderLine3D(const Vector3& start, const Vector3& end, const DebugRenderOptions& options)
//	: DebugRenderTask(options)
//	, m_start(start)
//	, m_end(end)
//{
//}
//
//
////-------------------------------------------------------------------------------------------------
//void DebugRenderLine3D::Render() const
//{
//	UNIMPLEMENTED();
//	Matrix4 mat = m_transform.GetLocalToWorldMatrix();
//	Vector3 startWs = mat.TransformPosition(m_start);
//	Vector3 endWs = mat.TransformPosition(m_end);
//
//	//g_renderContext->DrawLine3D(startWs, endWs, Rgba::WHITE, g_debugRenderSystem->GetShader());
//}
//
//
////-------------------------------------------------------------------------------------------------
//DebugRenderPoint3D::DebugRenderPoint3D(const Vector3& position, const DebugRenderOptions& options)
//	: DebugRenderTask(options)
//{
//	m_transform.position = position;
//}
//
//
////-------------------------------------------------------------------------------------------------
//void DebugRenderPoint3D::Render() const
//{
//	UNIMPLEMENTED();
//	Vector3 posWs = m_transform.GetWorldPosition();
//	g_renderContext->DrawPoint3D(posWs, 0.25f, m_options.m_color);
//}
//
//
////-------------------------------------------------------------------------------------------------
//DebugRenderCube::DebugRenderCube(const Vector3& center, const Vector3& extents, const DebugRenderOptions& options)
//	: DebugRenderTask(options)
//{
//	m_transform.position = center;
//	m_transform.scale = 2.f * extents; // Base mesh has 0.5 extents, so scale up the extents by 2 to compensate
//}
//
//
////-------------------------------------------------------------------------------------------------
//void DebugRenderCube::Render() const
//{
//	Mesh* cubeMesh = g_resourceSystem->CreateOrGetMesh("unit_cube");
//	Material* debugMaterial = g_resourceSystem->CreateOrGetMaterial("Data/Material/debug.material");
//
//	Renderable rend;
//	rend.AddDraw(cubeMesh, debugMaterial, m_transform.GetLocalToWorldMatrix());
//	
//	g_renderContext->DrawRenderable(rend);
//}
//
//
////-------------------------------------------------------------------------------------------------
//DebugRenderSphere::DebugRenderSphere(const Vector3& center, float radius, const DebugRenderOptions& options)
//	: DebugRenderTask(options)
//{
//	m_transform.position = center;
//	m_transform.scale = Vector3(radius);
//}
//
//
////-------------------------------------------------------------------------------------------------
//void DebugRenderSphere::Render() const
//{
//	Mesh* sphereMesh = g_resourceSystem->CreateOrGetMesh("unit_sphere");
//
//	// TODO: Material instancing
//	Material* debugMaterial = g_resourceSystem->CreateOrGetMaterial("Data/Material/debug.material");
//	ShaderResourceView* prevAlbedo = debugMaterial->GetAlbedo();
//	if (m_options.m_fillMode == FILL_MODE_WIREFRAME)
//	{
//		debugMaterial->SetAlbedoTextureView(g_resourceSystem->CreateOrGetTexture2D("white")->CreateOrGetShaderResourceView());
//	}
//
//	// TODO: Shader instancing
//	FillMode prevFillMode = debugMaterial->GetShader()->GetFillMode();
//	debugMaterial->GetShader()->SetFillMode(m_options.m_fillMode);
//
//	Renderable rend;
//	rend.AddDraw(sphereMesh, debugMaterial, m_transform.GetLocalToWorldMatrix());
//
//	g_renderContext->DrawRenderable(rend);
//
//	debugMaterial->GetShader()->SetFillMode(prevFillMode);
//	debugMaterial->SetAlbedoTextureView(prevAlbedo);
//}
//
//
////-------------------------------------------------------------------------------------------------
//DebugRenderText3D::DebugRenderText3D(const char* text, const Vector3& position, const DebugRenderOptions& options)
//	: DebugRenderTask(options)
//	, m_text(text)
//{
//	m_transform.position = position;
//}
//
//
////-------------------------------------------------------------------------------------------------
//void DebugRenderText3D::Render() const
//{
//	UNIMPLEMENTED();
//	//Font* font = g_resourceSystem->CreateOrGetFont("Data/Font/Prototype.ttf");
//}
//
//
////-------------------------------------------------------------------------------------------------
//DebugRenderCapsule::DebugRenderCapsule(const Vector3& start, const Vector3& end, float radius, const DebugRenderOptions& options)
//	: DebugRenderTask(options)
//{
//	m_transform.position = 0.5f * (start + end);
//	float height = (end - start).GetLength();
//	m_transform.scale = Vector3(radius, height, radius);
//}
//
//
////-------------------------------------------------------------------------------------------------
//void DebugRenderCapsule::Render() const
//{
//	Mesh* topMesh = g_resourceSystem->CreateOrGetMesh("capsule_top");
//	Mesh* bottomMesh = g_resourceSystem->CreateOrGetMesh("capsule_bottom");
//	Mesh* middleMesh = g_resourceSystem->CreateOrGetMesh("capsule_middle");
//
//	// TODO: Material instancing
//	Material* debugMaterial = g_resourceSystem->CreateOrGetMaterial("Data/Material/debug.material");
//	ShaderResourceView* prevAlbedo = debugMaterial->GetAlbedo();
//	if (m_options.m_fillMode == FILL_MODE_WIREFRAME)
//	{
//		debugMaterial->SetAlbedoTextureView(g_resourceSystem->CreateOrGetTexture2D("white")->CreateOrGetShaderResourceView());
//	}
//
//	// TODO: Shader instancing
//	FillMode prevFillMode = debugMaterial->GetShader()->GetFillMode();
//	debugMaterial->GetShader()->SetFillMode(m_options.m_fillMode);
//
//	Transform topTransform = m_transform;
//	topTransform.position = m_transform.position + Vector3(0.f, m_transform.scale.y * 0.5f, 0.f);
//	topTransform.scale.y = topTransform.scale.x; // The hemispheres need to be scaled up by the radius in the y direction, which is stored in scale x and z
//
//	Transform bottomTransform = topTransform;
//	bottomTransform.position = m_transform.position - Vector3(0.f, m_transform.scale.y * 0.5f, 0.f);
//
//	Renderable rend;
//	rend.AddDraw(topMesh, debugMaterial, topTransform.GetLocalToWorldMatrix());
//	rend.AddDraw(middleMesh, debugMaterial, m_transform.GetLocalToWorldMatrix());
//	rend.AddDraw(bottomMesh, debugMaterial, bottomTransform.GetLocalToWorldMatrix());
//
//	g_renderContext->DrawRenderable(rend);
//
//	debugMaterial->GetShader()->SetFillMode(prevFillMode);
//	debugMaterial->SetAlbedoTextureView(prevAlbedo);
//}
//
//
////------------------------------------------------------------------------------------------------- 
//DebugRenderOBB3::DebugRenderOBB3(const OBB3& obb, const DebugRenderOptions& options)
//	: DebugRenderTask(options)
//{
//	m_transform.position = obb.center;
//	m_transform.scale = 2.f * obb.GetExtents();
//	m_transform.rotation = obb.rotation;
//}
//
//
////-------------------------------------------------------------------------------------------------
//void DebugRenderOBB3::Render() const
//{
//	Mesh* cubeMesh = g_resourceSystem->CreateOrGetMesh("unit_cube");
//	Material* debugMaterial = g_resourceSystem->CreateOrGetMaterial("Data/Material/debug.material");
//
//	Renderable rend;
//	rend.AddDraw(cubeMesh, debugMaterial, m_transform.GetLocalToWorldMatrix());
//
//	g_renderContext->DrawRenderable(rend);
//}
