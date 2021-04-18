///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: February 12th, 2021
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/Entity.h"
#include "Engine/Framework/Rgba.h"
#include "Engine/Math/Transform.h"
#include "Engine/Time/FrameTimer.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#define INVALID_DEBUG_RENDER_OBJECT_HANDLE -1;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
typedef int DebugRenderHandle;
struct DebugRenderOptions
{
	Rgba	m_color = Rgba::RED;
	float	m_lifetime = FLT_MAX;
	float	m_scale = 1.0f;
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
	friend class DebugRenderSystem;
	RTTI_BASE_CLASS(DebugRenderObject);


public:
	//-----Public Methods-----

	DebugRenderObject(const DebugRenderOptions& options);

	virtual void		Render() const = 0;

	bool				IsFinished() const;			
	DebugRenderHandle	GetHandle() const { return m_handle; }

	void				SetOptions(const DebugRenderOptions& options);
	void				SetTimeToLive(float newTtl);
	void				SetColor(const Rgba& color);


protected:
	//-----Protected Data-----

	Transform			m_transform;
	FrameTimer			m_timer;
	DebugRenderOptions	m_options;
	DebugRenderHandle	m_handle = INVALID_DEBUG_RENDER_OBJECT_HANDLE;

};


//-------------------------------------------------------------------------------------------------
class DebugRenderTransform : public DebugRenderObject
{
public:
	//-----Public Methods-----

	DebugRenderTransform(const Transform& transform, bool freezeTransform, const DebugRenderOptions& options);
	virtual void Render() const override;


private:
	//-----Private Data-----

};
///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
