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
#include "Engine/Render/Camera.h"
#include "Engine/Render/Debug/DebugRenderSystem.h"
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
DebugRenderObjectHandle DebugDrawBox(const Vector3& center, const Vector3& extents, const Quaternion& rotation, const DebugRenderOptions& options)
{
	DebugRenderObject* obj = new DebugRenderObject(options);

	obj->m_transform.position = center;
	obj->m_transform.scale = 2.f * extents;
	obj->m_transform.rotation = rotation;

	Mesh* mesh = g_resourceSystem->CreateOrGetMesh("unit_cube");
	obj->SetMesh(mesh, false);

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

	//if (m_worldAxesObject != INVALID_DEBUG_RENDER_OBJECT_HANDLE)
	//{
	//	DebugRenderTransform* axes = GetObjectAs<DebugRenderTransform>(m_worldAxesObject);
	//	axes->m_transform.position = m_camera->GetPosition() + m_camera->GetForwardVector();
	//}

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

	//if (m_worldAxesObject != INVALID_DEBUG_RENDER_OBJECT_HANDLE)
	//{
	//	DestroyObject(m_worldAxesObject);
	//	m_worldAxesObject = INVALID_DEBUG_RENDER_OBJECT_HANDLE;

	//	return false;
	//}

	//Transform toDraw;
	//toDraw.position += m_camera->GetPosition() + m_camera->GetForwardVector();
	//toDraw.scale = Vector3(0.25f);

	//DebugRenderOptions options;
	//options.m_color = Rgba::WHITE;

	//DebugRenderTransform* debugTransform = new DebugRenderTransform(toDraw, options);
	//m_worldAxesObject = AddObject(debugTransform);

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
