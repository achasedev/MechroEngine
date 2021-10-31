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
#include "Engine/Render/Camera.h"
#include "Engine/Render/Debug/DebugRenderSystem.h"
#include "Engine/Render/Mesh/MeshBuilder.h"
#include "Engine/Render/RenderContext.h"
#include "Engine/Resource/ResourceSystem.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------
DebugRenderSystem*	g_debugRenderSystem = nullptr;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Draws a box
DebugRenderObjectHandle DebugDrawBox(const Vector3& center, const Vector3& extents, const Quaternion& rotation, const DebugRenderOptions& options /*= DebugRenderOptions()*/)
{
	DebugRenderObject* obj = new DebugRenderObject(options);

	obj->m_transform.position = center;
	obj->m_transform.scale = 2.f * extents;
	obj->m_transform.rotation = rotation;

	Mesh* mesh = g_resourceSystem->CreateOrGetMesh("unit_cube");
	obj->AddMesh(mesh, Matrix4::IDENTITY, false);

	return g_debugRenderSystem->AddObject(obj);
}


//-------------------------------------------------------------------------------------------------
// Draws a point
DebugRenderObjectHandle DebugDrawPoint(const Vector3& position, float diameter, const DebugRenderOptions& options /*= DebugRenderOptions()*/)
{
	DebugRenderObject* obj = new DebugRenderObject(options);

	obj->m_transform.position = position;
	obj->m_transform.scale = 0.5f * Vector3(diameter); // Point mesh is already 2 in diameter

	Mesh* mesh = g_resourceSystem->CreateOrGetMesh("point");
	obj->AddMesh(mesh, Matrix4::IDENTITY, false);

	return g_debugRenderSystem->AddObject(obj);
}


//-------------------------------------------------------------------------------------------------
// Draws a line in 3D
DebugRenderObjectHandle DebugDrawLine(const Vector3& start, const Vector3& end, const DebugRenderOptions& options /*= DebugRenderOptions()*/)
{
	DebugRenderObject* obj = new DebugRenderObject(options);

	// Keep transform identity, bake positions into the mesh
	MeshBuilder mb;
	mb.BeginBuilding(TOPOLOGY_LINE_LIST, false);
	mb.PushLine3D(start, end, Rgba::WHITE);
	mb.FinishBuilding();

	Mesh* mesh = mb.CreateMesh<Vertex3D_PCU>();
	obj->AddMesh(mesh, Matrix4::IDENTITY, true);

	return g_debugRenderSystem->AddObject(obj);
}


//-------------------------------------------------------------------------------------------------
// Draws 3 axes to represent a transform; Red is X, Green is Y, Blue is Z
DebugRenderObjectHandle DebugDrawTransform(const Transform& transform, const DebugRenderOptions& options /*= DebugRenderOptions()*/)
{
	DebugRenderObject* obj = new DebugRenderObject(options);

	if (transform.GetParentTransform() != nullptr && options.m_parentTransform != nullptr)
	{
		ConsoleLogErrorf("Attempted to debug draw a transform with two parents specified, defaulting to the one on the given transform (not in options).");
	}

	obj->m_transform = transform;

	Mesh* mesh = g_resourceSystem->CreateOrGetMesh("transform");
	obj->AddMesh(mesh, Matrix4::IDENTITY, false);

	return g_debugRenderSystem->AddObject(obj);
}


//-------------------------------------------------------------------------------------------------
// Draws a sphere
DebugRenderObjectHandle DebugDrawSphere(const Vector3& center, float radius, const DebugRenderOptions& options /*= DebugRenderOptions()*/)
{
	DebugRenderObject* obj = new DebugRenderObject(options);

	obj->m_transform.position = center;
	obj->m_transform.scale = Vector3(radius);

	Mesh* mesh = g_resourceSystem->CreateOrGetMesh("unit_sphere");
	obj->AddMesh(mesh, Matrix4::IDENTITY, false);

	return g_debugRenderSystem->AddObject(obj);
}


//-------------------------------------------------------------------------------------------------
// Draws a capsule
DebugRenderObjectHandle DebugDrawCapsule(const Capsule3D& capsule, const DebugRenderOptions& options /*= DebugRenderOptions()*/)
{
	DebugRenderObject* obj = new DebugRenderObject(options);

	Vector3 pos = 0.5f * (capsule.start + capsule.end);
	Vector3 up = (capsule.end - capsule.start);
	float startEndDistance = up.SafeNormalize(Vector3::Y_AXIS);

	Vector3 reference = (!AreMostlyEqual(up, Vector3::Z_AXIS) ? Vector3::Z_AXIS : Vector3::X_AXIS);
	Vector3 right = CrossProduct(up, reference).GetNormalized();
	Vector3 forward = CrossProduct(right, up);

	obj->m_transform.SetLocalMatrix(Matrix4(right, up, forward, pos));

	Mesh* topMesh = g_resourceSystem->CreateOrGetMesh("capsule_top");
	Mesh* middleMesh = g_resourceSystem->CreateOrGetMesh("capsule_middle");
	Mesh* bottomMesh = g_resourceSystem->CreateOrGetMesh("capsule_bottom");

	Matrix4 topMat = Matrix4::MakeModelMatrix(Vector3(0.f, 0.5f * startEndDistance, 0.f), Quaternion::IDENTITY, Vector3(capsule.radius));
	Matrix4 middleMat = Matrix4::MakeScale(Vector3(capsule.radius, startEndDistance, capsule.radius));
	Matrix4 bottomMat = Matrix4::MakeModelMatrix(Vector3(0.f, -0.5f * startEndDistance, 0.f), Quaternion::IDENTITY, Vector3(capsule.radius));

	obj->AddMesh(topMesh, topMat, false);
	obj->AddMesh(middleMesh, middleMat, false);
	obj->AddMesh(bottomMesh, bottomMat, false);

	return g_debugRenderSystem->AddObject(obj);
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
void DebugRenderSystem::Initialize()
{
	ASSERT_OR_DIE(g_debugRenderSystem == nullptr, "DebugRenderSystem is being initialized twice!");
	g_debugRenderSystem = new DebugRenderSystem();
}


//-------------------------------------------------------------------------------------------------
void DebugRenderSystem::Shutdown()
{
	ASSERT_OR_DIE(g_debugRenderSystem != nullptr, "DebugRenderSystem not initialized!");
	SAFE_DELETE(g_debugRenderSystem);
}


//-------------------------------------------------------------------------------------------------
void DebugRenderSystem::SetCamera(Camera* camera)
{
	m_camera = camera;
}


//-------------------------------------------------------------------------------------------------
void DebugRenderSystem::Render()
{
	g_renderContext->BeginCamera(m_camera);

	// Update axes to stay in front of the camera
	if (m_worldAxesObject != INVALID_DEBUG_RENDER_OBJECT_HANDLE)
	{
		DebugRenderObject* axes = GetObject(m_worldAxesObject);
		axes->m_transform.position = m_camera->GetPosition() + m_camera->GetForwardVector();
	}

	// Draw all objects
	int numObjects = (int)m_objects.size();
	for (int objIndex = 0; objIndex < numObjects; ++objIndex)
	{
		m_objects[objIndex]->Render();
	}

	g_renderContext->EndCamera();

	// Clean up any finished objects
	for (int objIndex = numObjects - 1; objIndex >= 0; --objIndex)
	{
		if (m_objects[objIndex]->IsFinished())
		{
			SAFE_DELETE(m_objects[objIndex]);
			m_objects.erase(m_objects.begin() + objIndex);
		}
	}
}


//-------------------------------------------------------------------------------------------------
DebugRenderObject* DebugRenderSystem::GetObject(const DebugRenderObjectHandle& handle)
{
	int numObjects = (int)m_objects.size();
	for (int objIndex = 0; objIndex < numObjects; ++objIndex)
	{
		if (m_objects[objIndex]->GetHandle() == handle)
		{
			return m_objects[objIndex];
		}
	}

	return nullptr;
}


//-------------------------------------------------------------------------------------------------
bool DebugRenderSystem::ToggleWorldAxesDraw()
{
	ASSERT_RETURN(m_camera != nullptr, false, "No camera set!");

	if (m_worldAxesObject != INVALID_DEBUG_RENDER_OBJECT_HANDLE)
	{
		DestroyObject(m_worldAxesObject);
		m_worldAxesObject = INVALID_DEBUG_RENDER_OBJECT_HANDLE;

		return false;
	}

	Transform transform;
	transform.position += m_camera->GetPosition() + m_camera->GetForwardVector();
	transform.scale = Vector3(0.25f);

	DebugRenderOptions options;
	options.m_debugRenderMode = DEBUG_RENDER_MODE_IGNORE_DEPTH;

	m_worldAxesObject = DebugDrawTransform(transform, options);

	return true;
}


//-------------------------------------------------------------------------------------------------
DebugRenderSystem::~DebugRenderSystem()
{
	int numObjects = (int)m_objects.size();
	for (int objIndex = 0; objIndex < numObjects; ++objIndex)
	{
		SAFE_DELETE(m_objects[objIndex]);
	}

	m_objects.clear();
}


//-------------------------------------------------------------------------------------------------
DebugRenderSystem::DebugRenderSystem()
{
	m_shader = g_resourceSystem->CreateOrGetShader("Data/Shader/debug_render_object.shader");
	if (m_shader == nullptr)
	{
		ConsoleLogErrorf("Default shader for DebugRenderSystem couldn't be loaded!");
	}

	m_texture = g_resourceSystem->CreateOrGetTexture2D("Data/Image/debug.png");
	if (m_texture == nullptr)
	{
		ConsoleLogErrorf("Default texture for DebugRenderSystem couldn't be loaded!");
	}
}


//-------------------------------------------------------------------------------------------------
DebugRenderObjectHandle DebugRenderSystem::AddObject(DebugRenderObject* object)
{
	const DebugRenderObjectHandle handle = m_nextHandle++;
	object->m_handle = handle;
	m_objects.push_back(object);

	return handle;
}


//-------------------------------------------------------------------------------------------------
void DebugRenderSystem::DestroyObject(DebugRenderObjectHandle handle)
{
	int numObjects = (int)m_objects.size();
	for (int objIndex = 0; objIndex < numObjects; ++objIndex)
	{
		if (m_objects[objIndex]->GetHandle() == handle)
		{
			delete m_objects[objIndex];
			m_objects.erase(m_objects.begin() + objIndex);
		}
	}
}
