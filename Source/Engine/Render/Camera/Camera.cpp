///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: December 16th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Render/Camera/Camera.h"
#include "Engine/Render/Texture/ColorTargetView.h"
#include "Engine/Render/Core/RenderContext.h"
#include "Engine/Render/Buffer/UniformBuffer.h"
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Framework/Window.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
struct CameraBufferData
{
	Matrix44 m_projectionMatrix;
};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
Camera::Camera()
{
	m_colorTarget = new ColorTargetView();

	// Set the target to the backbuffer (for now)
	RenderContext* renderContext = RenderContext::GetInstance();
	SetColorTarget(renderContext->GetBackBufferColorTarget(), false);
}


//-------------------------------------------------------------------------------------------------
Camera::~Camera()
{
	SAFE_DELETE_POINTER(m_cameraUBO);

	if (m_ownsColorTarget)
	{
		SAFE_DELETE_POINTER(m_colorTarget);
	}
}


//-------------------------------------------------------------------------------------------------
void Camera::SetColorTarget(ColorTargetView* colorTarget, bool ownsTarget)
{
	if (m_colorTarget != nullptr && m_ownsColorTarget)
	{
		SAFE_DELETE_POINTER(m_colorTarget);
	}

	m_colorTarget = colorTarget;
	m_ownsColorTarget = ownsTarget;
}


//-------------------------------------------------------------------------------------------------
void Camera::SetOrthoProjection(float orthoHeight)
{
	float aspect = Window::GetInstance()->GetClientAspect();
	m_orthoBounds.mins = Vector2::ZERO;
	m_orthoBounds.maxs = Vector2(orthoHeight * aspect, orthoHeight);
	m_nearClipZ = -1.0f;
	m_farClipZ = 1.0f;
	m_projectionMatrix = Matrix44::MakeOrtho(m_orthoBounds.mins, m_orthoBounds.maxs, m_nearClipZ, m_farClipZ);
}


//-------------------------------------------------------------------------------------------------
void Camera::UpdateUBO()
{
	// Lazy instantiation
	if (m_cameraUBO == nullptr)
	{
		m_cameraUBO = new UniformBuffer();
	}

	CameraBufferData cameraData;
	cameraData.m_projectionMatrix = m_projectionMatrix;

	m_cameraUBO->CopyToGPU(&cameraData, sizeof(cameraData));
}
