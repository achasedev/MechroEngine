///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: February 20th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
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
DebugRenderHandle DebugDrawTransform(const Transform& transform, float lifetime /*= FLT_MAX*/, const Transform* parentTransform /*= nullptr*/)
{
	DebugRenderOptions options;
	options.m_lifetime = lifetime;
	options.m_color = Rgba::WHITE;
	options.m_parentTransform = parentTransform;

	DebugRenderTransform* debugTransform = new DebugRenderTransform(transform, options);
	return g_debugRenderSystem->AddObject(debugTransform);
}


//-------------------------------------------------------------------------------------------------
DebugRenderHandle DebugDrawLine3D(const Vector3& start, const Vector3& end, const Rgba& color /*= Rgba::RED*/, float lifetime /*= FLT_MAX*/, const Transform* parentTransform /*= nullptr*/)
{
	DebugRenderOptions options;
	options.m_lifetime = lifetime;
	options.m_color = color;
	options.m_parentTransform = parentTransform;

	DebugRenderLine3D* line = new DebugRenderLine3D(start, end, options);
	return g_debugRenderSystem->AddObject(line);
}


//-------------------------------------------------------------------------------------------------
DebugRenderHandle DebugDrawPoint3D(const Vector3& position, const Rgba& color /*= Rgba::RED*/, float lifetime /*= FLT_MAX*/, const Transform* parentTransform /*= nullptr*/)
{
	DebugRenderOptions options;
	options.m_lifetime = lifetime;
	options.m_color = color;
	options.m_parentTransform = parentTransform;

	DebugRenderPoint3D* point = new DebugRenderPoint3D(position, options);
	return g_debugRenderSystem->AddObject(point);
}


//-------------------------------------------------------------------------------------------------
DebugRenderHandle DebugDrawCube(const Vector3& center, const Vector3& extents, const Rgba& color /*= Rgba::WHITE*/, float lifetime /*= FLT_MAX*/, const Transform* parentTransform /*= nullptr*/)
{
	DebugRenderOptions options;
	options.m_lifetime = lifetime;
	options.m_color = color;
	options.m_parentTransform = parentTransform;

	DebugRenderCube* debugCube = new DebugRenderCube(center, extents, options);
	return g_debugRenderSystem->AddObject(debugCube);
}


//-------------------------------------------------------------------------------------------------
DebugRenderHandle DebugDrawSphere(const Vector3& center, float radius, const Rgba& color /*= Rgba::WHITE*/, float lifetime /*= FLT_MAX*/, const Transform* parentTransform /*= nullptr*/)
{
	DebugRenderOptions options;
	options.m_lifetime = lifetime;
	options.m_color = color;
	options.m_parentTransform = parentTransform;

	DebugRenderSphere* debugSphere = new DebugRenderSphere(center, radius, options);
	return g_debugRenderSystem->AddObject(debugSphere);
}


//-------------------------------------------------------------------------------------------------
DebugRenderHandle DebugDrawCapsule(const Vector3& start, const Vector3& end, float radius, const Rgba& color /*= Rgba::WHITE*/, float lifetime /*= FLT_MAX*/, const Transform* parentTransform /*= nullptr*/)
{
	DebugRenderOptions options;
	options.m_lifetime = lifetime;
	options.m_color = color;
	options.m_parentTransform = parentTransform;

	DebugRenderCapsule* debugCapsule = new DebugRenderCapsule(start, end, radius, options);
	return g_debugRenderSystem->AddObject(debugCapsule);
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

	if (m_worldAxesTask != INVALID_DEBUG_RENDER_OBJECT_HANDLE)
	{
		DebugRenderTransform* axes = GetObjectAs<DebugRenderTransform>(m_worldAxesTask);
		axes->m_transform.position = m_camera->GetPosition() + m_camera->GetForwardVector();
	}

	// Draw all objects
	int numObjects = (int)m_objects.size();
	for (int objIndex = 0; objIndex < numObjects; ++objIndex)
	{
		m_objects[objIndex]->PreRender();
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
DebugRenderTask* DebugRenderSystem::GetObject(const DebugRenderHandle& handle)
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

	if (m_worldAxesTask != INVALID_DEBUG_RENDER_OBJECT_HANDLE)
	{
		DestroyObject(m_worldAxesTask);
		m_worldAxesTask = INVALID_DEBUG_RENDER_OBJECT_HANDLE;

		return false;
	}

	Transform toDraw;
	toDraw.position += m_camera->GetPosition() + m_camera->GetForwardVector();
	toDraw.scale = Vector3(0.25f);

	DebugRenderOptions options;
	options.m_color = Rgba::WHITE;

	DebugRenderTransform* debugTransform = new DebugRenderTransform(toDraw, options);
	m_worldAxesTask = AddObject(debugTransform);

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
DebugRenderHandle DebugRenderSystem::AddObject(DebugRenderTask* object)
{
	const DebugRenderHandle handle = m_nextHandle++;
	object->m_handle = handle;
	m_objects.push_back(object);

	return handle;
}


//-------------------------------------------------------------------------------------------------
void DebugRenderSystem::DestroyObject(DebugRenderHandle handle)
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
