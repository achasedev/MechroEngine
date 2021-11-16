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
#include "Engine/Math/Capsule3.h"
#include "Engine/Math/Cylinder.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/OBB3.h"
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
// Draws a box
DebugRenderObjectHandle DebugDrawBox(const OBB3& box, const DebugRenderOptions& options /*= DebugRenderOptions()*/)
{
	return DebugDrawBox(box.center, box.extents, box.rotation, options);
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
// Draws a sphere
DebugRenderObjectHandle DebugDrawSphere(const Sphere& sphere, const DebugRenderOptions& options /*= DebugRenderOptions()*/)
{
	return DebugDrawSphere(sphere.m_center, sphere.m_radius, options);
}


//-------------------------------------------------------------------------------------------------
// Draws a capsule
DebugRenderObjectHandle DebugDrawCapsule(const Capsule3& capsule, const DebugRenderOptions& options /*= DebugRenderOptions()*/)
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

	Vector3 endcapScale = Vector3(capsule.radius);
	if (options.m_parentTransform != nullptr)
	{
		// Ugly hack - in the case the size of the capsule are embedded inside the parent transform's scale, we need
		// to make sure the end caps aren't scaled incorrectly in the y. So make up a scale s.t. after the parent scale is applied we end up scaling by xz scale, not y scale
		endcapScale.y *= options.m_parentTransform->scale.x / options.m_parentTransform->scale.y;
	}

	Matrix4 topMat = Matrix4::MakeModelMatrix(Vector3(0.f, 0.5f * startEndDistance, 0.f), Quaternion::IDENTITY, endcapScale);
	Matrix4 middleMat = Matrix4::MakeScale(Vector3(capsule.radius, startEndDistance, capsule.radius));
	Matrix4 bottomMat = Matrix4::MakeModelMatrix(Vector3(0.f, -0.5f * startEndDistance, 0.f), Quaternion::IDENTITY, endcapScale);

	obj->AddMesh(topMesh, topMat, false);
	obj->AddMesh(middleMesh, middleMat, false);
	obj->AddMesh(bottomMesh, bottomMat, false);

	return g_debugRenderSystem->AddObject(obj);
}


//-------------------------------------------------------------------------------------------------
// Draws a cylinder
DebugRenderObjectHandle DebugDrawCylinder(const Cylinder& cylinder, const DebugRenderOptions& options /*= DebugRenderOptions()*/)
{
	return DebugDrawCylinder(cylinder.m_bottom, cylinder.m_top, cylinder.m_radius, options);
}


//-------------------------------------------------------------------------------------------------
// Draws a cylinder
DebugRenderObjectHandle DebugDrawCylinder(const Vector3& bottom, const Vector3& top, float radius, const DebugRenderOptions& options /*= DebugRenderOptions()*/)
{
	DebugRenderObject* obj = new DebugRenderObject(options);

	Vector3 pos = 0.5f * (bottom + top);
	Vector3 up = (top - bottom);
	float height = up.SafeNormalize(Vector3::Y_AXIS);

	Vector3 reference = (!AreMostlyEqual(up, Vector3::Z_AXIS) ? Vector3::Z_AXIS : Vector3::X_AXIS);
	Vector3 right = CrossProduct(up, reference).GetNormalized();
	Vector3 forward = CrossProduct(right, up);

	Matrix4 model = Matrix4(right, up, forward, pos) * Matrix4::MakeScale(Vector3(radius, height, radius));
	obj->m_transform.SetLocalMatrix(model);

	Mesh* mesh = g_resourceSystem->CreateOrGetMesh("cylinder");
	obj->AddMesh(mesh, Matrix4::IDENTITY, false);

	return g_debugRenderSystem->AddObject(obj);
}


//-------------------------------------------------------------------------------------------------
// Draws a frustrum as 12 lines
DebugRenderObjectHandle DebugDrawFrustrum(const Frustrum& frustrum, const DebugRenderOptions& options /*= DebugRenderOptions()*/)
{
	MeshBuilder mb;
	mb.BeginBuilding(TOPOLOGY_LINE_LIST, true);

	for (int i = 0; i < 8; ++i)
	{
		mb.PushVertex(frustrum.GetPoint(i));
	}
	
	// Near clip plane
	mb.PushIndex(0); mb.PushIndex(1);
	mb.PushIndex(1); mb.PushIndex(2);
	mb.PushIndex(2); mb.PushIndex(3);
	mb.PushIndex(3); mb.PushIndex(0);

	// Far clip plane
	mb.PushIndex(4); mb.PushIndex(5);
	mb.PushIndex(5); mb.PushIndex(6);
	mb.PushIndex(6); mb.PushIndex(7);
	mb.PushIndex(7); mb.PushIndex(4);

	// Connecting lines to planes
	mb.PushIndex(0); mb.PushIndex(7);
	mb.PushIndex(1); mb.PushIndex(6);
	mb.PushIndex(2); mb.PushIndex(5);
	mb.PushIndex(3); mb.PushIndex(4);

	mb.FinishBuilding();
	Mesh* mesh = mb.CreateMesh<Vertex3D_PCU>();

	DebugRenderObject* obj = new DebugRenderObject(options);
	obj->AddMesh(mesh, Matrix4::IDENTITY, true);

	return g_debugRenderSystem->AddObject(obj);
}


//-------------------------------------------------------------------------------------------------
// Draws a plane really large
DebugRenderObjectHandle DebugDrawPlane(const Plane3& plane, const DebugRenderOptions& options /*= DebugRenderOptions()*/)
{
	Mesh* mesh = g_resourceSystem->CreateOrGetMesh("plane");

	Vector3 normal = plane.m_normal;
	Vector3 reference = AreMostlyEqual(Abs(DotProduct(normal, Vector3::X_AXIS)), 1.0f) ? Vector3::Z_AXIS : Vector3::X_AXIS;
	Vector3 bitangent = CrossProduct(reference, normal).GetNormalized();
	Vector3 tangent = CrossProduct(normal, bitangent);
	Vector3 position = plane.m_normal * plane.m_distance;

	Matrix4 modelMat = Matrix4(tangent, bitangent, normal, position);
	modelMat = modelMat * Matrix4::MakeScale(Vector3(200.f));

	DebugRenderObject* obj = new DebugRenderObject(options);
	obj->m_transform.SetLocalMatrix(modelMat);
	obj->AddMesh(mesh, Matrix4::IDENTITY, false);

	return g_debugRenderSystem->AddObject(obj);
}


//-------------------------------------------------------------------------------------------------
DebugRenderObjectHandle DebugDrawPolygon(const Polyhedron& poly, const DebugRenderOptions& options /*= DebugRenderOptions()*/)
{
	MeshBuilder mb;
	mb.BeginBuilding(TOPOLOGY_TRIANGLE_LIST, true);
	mb.PushPolygon(poly, Rgba::WHITE);
	mb.FinishBuilding();

	Mesh* mesh = mb.CreateMesh<Vertex3D_PCU>();

	DebugRenderObject* obj = new DebugRenderObject(options);
	obj->AddMesh(mesh, Matrix4::IDENTITY, true);

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
			return;
		}
	}
}
