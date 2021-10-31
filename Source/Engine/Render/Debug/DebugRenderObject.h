///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: February 12th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/Entity.h"
#include "Engine/Core/Rgba.h"
#include "Engine/Math/Transform.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Render/Renderable.h"
#include "Engine/Render/Shader/Shader.h"
#include "Engine/Time/FrameTimer.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#define INVALID_DEBUG_RENDER_OBJECT_HANDLE -1

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
typedef int DebugRenderObjectHandle;
enum DebugRenderMode
{
	DEBUG_RENDER_MODE_IGNORE_DEPTH,
	DEBUG_RENDER_MODE_USE_DEPTH,
	DEBUG_RENDER_MODE_HIDDEN,
	DEBUG_RENDER_MODE_XRAY
};

struct DebugRenderOptions
{
	Rgba				m_startColor = Rgba::WHITE;
	Rgba				m_endColor = Rgba::WHITE;
	float				m_lifetime = FLT_MAX;
	const Transform*	m_parentTransform = nullptr;
	FillMode			m_fillMode = FILL_MODE_SOLID;
	CullMode			m_cullMode = CULL_MODE_BACK;
	DebugRenderMode		m_debugRenderMode = DEBUG_RENDER_MODE_USE_DEPTH;
};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class DebugRenderObject
{
public:
	friend class DebugRenderSystem;

	//-----Public Methods-----

	DebugRenderObject(const DebugRenderOptions& options);
	~DebugRenderObject();

	void					Render();

	bool					IsFinished() const;			
	DebugRenderObjectHandle	GetHandle() const { return m_handle; }

	void					SetOptions(const DebugRenderOptions& options);
	void					SetLifetime(float lifetime, bool resetTimer = true);
	void					SetStartColor(const Rgba& color);
	void					SetEndColor(const Rgba& color);
	void					SetColors(const Rgba& startColor, const Rgba& endColor);
	void					SetParentTransform(const Transform* parentTransform);
	void					SetFillMode(FillMode fillMode);
	void					SetCullMode(CullMode cullMode);
	void					SetDebugRenderMode(DebugRenderMode mode);
	void					SetMesh(Mesh* mesh, bool ownsMesh);


public:
	//-----Public Data-----

	Transform				m_transform;


private:
	//-----Private Methods-----

	Vector4					CalculateTint() const;


private:
	//-----Private Data-----

	FrameTimer				m_timer;
	DebugRenderOptions		m_options;
	DebugRenderObjectHandle	m_handle = INVALID_DEBUG_RENDER_OBJECT_HANDLE;
	DebugRenderSystem*		m_system = nullptr;

	Material*				m_material = nullptr;
	Mesh*					m_mesh = nullptr;
	bool					m_ownsMesh = false;

	static constexpr float	DEFAULT_XRAY_SCALE = 0.25f;

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
