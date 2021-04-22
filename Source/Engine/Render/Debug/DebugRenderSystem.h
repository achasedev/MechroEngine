///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: February 20th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Render/Buffer/UniformBuffer.h"
#include "Engine/Render/Debug/DebugRenderTask.h"
#include <map>

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Camera;
struct DebugBufferData
{
	Vector4 m_colorTint = Vector4::ONES;
};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class DebugRenderSystem
{
public:
	//-----Public Methods-----

	static void			Initialize();
	static void			Shutdown();

	void				SetCamera(Camera* camera);
	void				Render();

	DebugRenderHandle	AddObject(DebugRenderTask* object);
	DebugRenderTask*	GetObject(const DebugRenderHandle& handle);

	Shader*				GetShader() const;
	void				UpdateUniformBuffer(const DebugBufferData& data);

	template<typename T>
	T*					GetObjectAs(const DebugRenderHandle& handle);


private:
	//-----Private Methods-----

	DebugRenderSystem() {}
	~DebugRenderSystem();
	DebugRenderSystem(const DebugRenderSystem& other) = delete;


private:
	//-----Private Data-----

	Camera*							m_camera = nullptr;
	UniformBuffer					m_uniformBuffer;
	std::vector<DebugRenderTask*>	m_objects;
	DebugRenderHandle				m_nextHandle = 0;

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// TEMPLATES
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
template<typename T>
T* DebugRenderSystem::GetObjectAs(const DebugRenderHandle& handle)
{
	DebugRenderTask* object = GetObject(handle);
	return object->GetAsType<T*>();
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

// Convenience
DebugRenderHandle DebugDrawTransform(const Transform& transform, float lifetime = FLT_MAX, const Transform* parentTransform = nullptr);
DebugRenderHandle DebugDrawLine3D(const Vector3& start, const Vector3& end, const Rgba& color = Rgba::RED, float lifetime = FLT_MAX, const Transform* parentTransform = nullptr);
DebugRenderHandle DebugDrawPoint3D(const Vector3& position, const Rgba& color = Rgba::RED, float lifetime = FLT_MAX, const Transform* parentTransform = nullptr);
DebugRenderHandle DebugDrawRigidBody3D(const RigidBody3D* body, const Rgba& color);
DebugRenderHandle DebugDrawCube(const Vector3& center, const Vector3& extents, const Rgba& color = Rgba::WHITE, float lifetime = FLT_MAX, const Transform* parentTransform = nullptr);
DebugRenderHandle DebugDrawSphere(const Vector3& center, float radius, const Rgba& color = Rgba::WHITE, float lifetime = FLT_MAX, const Transform* parentTransform = nullptr);
