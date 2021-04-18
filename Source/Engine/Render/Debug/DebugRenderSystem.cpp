///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: February 20th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
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

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

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
void DebugRenderSystem::Render()
{
	// Draw all objects
	for (std::map<DebugRenderHandle, DebugRenderObject*>::iterator itr = m_objects.begin(); itr != m_objects.end(); itr++)
	{
		itr->second->Render();
	}

	// Clean up any finished objects
	std::map<DebugRenderHandle, DebugRenderObject*>::iterator currItr, nextItr;
	for (currItr = m_objects.begin(), nextItr = currItr; currItr != m_objects.end(); currItr = nextItr)
	{
		++nextItr;
		if (currItr->second->IsFinished())
		{
			SAFE_DELETE(currItr->second);
			m_objects.erase(currItr);
		}
	}
}


//-------------------------------------------------------------------------------------------------
DebugRenderObject* DebugRenderSystem::GetObject(const DebugRenderHandle& handle)
{
	const bool objectExists = m_objects.find(handle) != m_objects.end();
	if (objectExists)
	{
		return m_objects[handle];
	}

	return nullptr;
}


//-------------------------------------------------------------------------------------------------
void DebugRenderSystem::AddObject(DebugRenderObject* object)
{
	const DebugRenderHandle handle = m_nextHandle++;
	m_objects[handle] = object;
}
