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
#include "Engine/Math/Vector3.h"
#include "Engine/Time/FrameTimer.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#define INVALID_DEBUG_RENDER_OBJECT_HANDLE -1;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class RigidBody3D;
typedef int DebugRenderHandle;
struct DebugRenderOptions
{
	Rgba				m_color = Rgba::RED;
	float				m_lifetime = FLT_MAX;
	const Transform*	m_parentTransform = nullptr;
};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class DebugRenderTask
{
	friend class DebugRenderSystem;
	RTTI_BASE_CLASS(DebugRenderTask);


public:
	//-----Public Methods-----

	DebugRenderTask(const DebugRenderOptions& options);

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
class DebugRenderTransform : public DebugRenderTask
{
public:
	//-----Public Methods-----

	DebugRenderTransform(const Transform& transform, const DebugRenderOptions& options);
	virtual void Render() const override;


private:
	//-----Private Data-----

};

//-------------------------------------------------------------------------------------------------
class DebugRenderLine3D : public DebugRenderTask
{
public:
	//-----Public Methods-----

	DebugRenderLine3D(const Vector3& start, const Vector3& end, const DebugRenderOptions& options);
	virtual void Render() const override;


private:
	//-----Private Data-----

	Vector3 m_start = Vector3::ZERO;
	Vector3 m_end	= Vector3::ZERO;

};


//-------------------------------------------------------------------------------------------------
class DebugRenderRigidBody3D : public DebugRenderTask
{
public:
	//-----Public Methods-----

	DebugRenderRigidBody3D(const RigidBody3D* rigidBody, const DebugRenderOptions& options);
	virtual void Render() const override;


private:
	//-----Private Data-----

	const RigidBody3D* m_rigidBody = nullptr;

};

//-------------------------------------------------------------------------------------------------
class DebugRenderPoint3D : public DebugRenderTask
{
public:
	//-----Public Methods-----

	DebugRenderPoint3D(const Vector3& position, const DebugRenderOptions& options);
	virtual void Render() const override;


private:
	//-----Private Data-----
	
};


//-------------------------------------------------------------------------------------------------
class DebugRenderCube : public DebugRenderTask
{
public:
	//-----Public Methods-----

	DebugRenderCube(const Vector3& center, const Vector3& extents, const DebugRenderOptions& options);
	virtual void Render() const override;


private:
	//-----Private Data-----

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
