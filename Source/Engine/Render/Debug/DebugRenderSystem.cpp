///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: February 20th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Render/Core/RenderContext.h"
#include "Engine/Render/Debug/DebugRenderSystem.h"

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
DebugRenderHandle DebugDrawTransform(const Transform& transform, float lifetime /*= FLT_MAX*/, bool freezeTransform /*= false*/)
{
	DebugRenderOptions options;
	options.m_lifetime = lifetime;
	options.m_color = Rgba::WHITE;

	DebugRenderTransform* debugTransform = new DebugRenderTransform(transform, freezeTransform, options);
	return g_debugRenderSystem->AddObject(debugTransform);
}


//-------------------------------------------------------------------------------------------------
DebugRenderHandle DebugDrawCube(
	const Vector3& position, 
	const Vector3& extents, 
	const DebugRenderOptions& options /*= DebugRenderOptions()*/)
{
	UNIMPLEMENTED();
	UNUSED(position);
	UNUSED(extents);
	UNUSED(options);

	return DebugRenderHandle();
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
DebugRenderObject* DebugRenderSystem::GetObject(const DebugRenderHandle& handle)
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
DebugRenderHandle DebugRenderSystem::AddObject(DebugRenderObject* object)
{
	const DebugRenderHandle handle = m_nextHandle++;
	object->m_handle = handle;
	m_objects.push_back(object);

	return handle;
}
