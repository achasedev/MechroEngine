///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: February 20th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Render/Buffer/ConstantBuffer.h"
#include "Engine/Render/Debug/DebugRenderObject.h"
#include <map>

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Camera;
class Capsule3D;
class Frustrum;
class OBB3;
class Plane3;
class Sphere3D;
class Shader;
class Texture2D;

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

	static void				Initialize();
	static void				Shutdown();
	void					Render();

	void					SetCamera(Camera* camera);
	DebugRenderObjectHandle	AddObject(DebugRenderObject* object);
	void					DestroyObject(DebugRenderObjectHandle handle);

	DebugRenderObject*		GetObject(const DebugRenderObjectHandle& handle);
	Camera*					GetCamera() const { return m_camera; }
	Shader*					GetDefaultShader() const { return m_shader; }
	Texture2D*				GetDefaultTexture2D() const { return m_texture; }

	bool					ToggleWorldAxesDraw(); // Special case to draw the world transform in front of the camera


private:
	//-----Private Methods-----

	// Singleton
	DebugRenderSystem();
	~DebugRenderSystem();
	DebugRenderSystem(const DebugRenderSystem& other) = delete;


private:
	//-----Private Data-----

	Camera*							m_camera = nullptr;
	ConstantBuffer					m_uniformBuffer;
	std::vector<DebugRenderObject*>	m_objects;

	DebugRenderObjectHandle			m_nextHandle = 0;
	DebugRenderObjectHandle			m_worldAxesObject = INVALID_DEBUG_RENDER_OBJECT_HANDLE;
	Shader*							m_shader = nullptr;
	Texture2D*						m_texture = nullptr;

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// TEMPLATES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

// Convenience
DebugRenderObjectHandle DebugDrawBox(const Vector3& center, const Vector3& extents, const Quaternion& rotation, const DebugRenderOptions& options = DebugRenderOptions());
DebugRenderObjectHandle DebugDrawBox(const OBB3& box, const DebugRenderOptions& options = DebugRenderOptions());
DebugRenderObjectHandle DebugDrawPoint(const Vector3& position, float diameter, const DebugRenderOptions& options = DebugRenderOptions());
DebugRenderObjectHandle DebugDrawLine(const Vector3& start, const Vector3& end, const DebugRenderOptions& options = DebugRenderOptions());
DebugRenderObjectHandle DebugDrawTransform(const Transform& transform, const DebugRenderOptions& options = DebugRenderOptions());
DebugRenderObjectHandle DebugDrawSphere(const Vector3& center, float radius, const DebugRenderOptions& options = DebugRenderOptions());
DebugRenderObjectHandle DebugDrawSphere(const Sphere3D& sphere, const DebugRenderOptions& options = DebugRenderOptions());
DebugRenderObjectHandle DebugDrawCapsule(const Capsule3D& capsule, const DebugRenderOptions& options = DebugRenderOptions());
DebugRenderObjectHandle DebugDrawFrustrum(const Frustrum& frustrum, const DebugRenderOptions& options = DebugRenderOptions());
DebugRenderObjectHandle DebugDrawPlane(const Plane3& plane, const DebugRenderOptions& options = DebugRenderOptions());
