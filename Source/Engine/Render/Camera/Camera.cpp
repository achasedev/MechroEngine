///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: December 16th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Event/EventSystem.h"
#include "Engine/Render/Buffer/UniformBuffer.h"
#include "Engine/Render/Camera/Camera.h"
#include "Engine/Render/Core/RenderContext.h"
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Framework/Window.h"
#include "Engine/Render/Texture/Texture2D.h"
#include "Engine/Render/View/DepthStencilTargetView.h"
#include "Engine/Render/View/RenderTargetView.h"
#include "Engine/Utility/NamedProperties.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
struct CameraUBOLayout
{
	Matrix44	m_cameraMatrix;
	Matrix44	m_viewMatrix;
	Matrix44	m_projectionMatrix;
	float		m_viewportTopLeftX;
	float		m_viewportTopLeftY;
	float		m_viewportWidth;
	float		m_viewportHeight;
};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Quick Matrix invert for look-at matrices
static Matrix44 InvertLookAtMatrix(const Matrix44& lookAt)
{
	Matrix44 rotation = lookAt;

	rotation.Tx = 0.f;
	rotation.Ty = 0.f;
	rotation.Tz = 0.f;

	rotation.Transpose();

	Matrix44 translation = Matrix44::MakeTranslation(Vector3(-lookAt.Tx, -lookAt.Ty, -lookAt.Tz));
	rotation.Append(translation);

	return rotation;
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
Camera::Camera()
{
	m_transform.position = Vector3::ZERO;
	m_transform.rotation = Quaternion::IDENTITY;
	m_transform.scale = Vector3::ONES;

	// Default to backbuffer for render target
	SetRenderTarget(g_renderContext->GetDefaultRenderTarget(), false);

	g_eventSystem->SubscribeEventCallbackObjectMethod("window-resize", &Camera::Event_WindowResize, *this);
}


//-------------------------------------------------------------------------------------------------
Camera::~Camera()
{
	g_eventSystem->SubscribeEventCallbackObjectMethod("window-resize", &Camera::Event_WindowResize, *this);

	SAFE_DELETE(m_cameraUBO);

	if (m_ownsRenderTarget)
	{
		SAFE_DELETE(m_renderTarget);
	}

	if (m_ownsDepthTarget)
	{
		SAFE_DELETE(m_depthTarget);
	}
}


//-------------------------------------------------------------------------------------------------
void Camera::SetRenderTarget(Texture2D* renderTarget, bool ownsTarget)
{
	if (m_ownsRenderTarget)
	{
		SAFE_DELETE(m_renderTarget);
	}

	m_renderTarget = renderTarget;
	m_ownsRenderTarget = ownsTarget;
}


//-------------------------------------------------------------------------------------------------
void Camera::SetDepthTarget(Texture2D* depthTarget, bool ownsTarget)
{
	if (m_ownsDepthTarget)
	{
		SAFE_DELETE(m_depthTarget);
	}

	m_depthTarget = depthTarget;
	m_ownsDepthTarget = ownsTarget;
}


//-------------------------------------------------------------------------------------------------
void Camera::SetPosition(const Vector3& position)
{
	m_transform.position = position;
}


//-------------------------------------------------------------------------------------------------
void Camera::Translate(const Vector3& translation)
{
	m_transform.Translate(translation, RELATIVE_TO_SELF);
	m_viewMatrix = InvertLookAtMatrix(m_transform.GetLocalToWorldMatrix());
}


//-------------------------------------------------------------------------------------------------
void Camera::SetRotation(const Vector3& rotation)
{
	m_transform.rotation = Quaternion::FromEuler(rotation);
	m_viewMatrix = InvertLookAtMatrix(m_transform.GetLocalToWorldMatrix());
}


//-------------------------------------------------------------------------------------------------
void Camera::SetProjection(CameraProjection projectionType, const Matrix44& projection)
{
	m_currentProjection = projectionType;
	m_projectionMatrix = projection;
}


//-------------------------------------------------------------------------------------------------
void Camera::SetProjectionOrthographic(float orthoHeight, float aspect)
{
	m_orthoBounds.mins = Vector2::ZERO;
	m_orthoBounds.maxs = Vector2(orthoHeight * aspect, orthoHeight);
	m_nearClipZ = -1.0f;
	m_farClipZ = 1.0f;
	m_projectionMatrix = Matrix44::MakeOrtho(m_orthoBounds.mins, m_orthoBounds.maxs, m_nearClipZ, m_farClipZ);
	m_currentProjection = CAMERA_PROJECTION_ORTHOGRAPHIC;
}


//-------------------------------------------------------------------------------------------------
void Camera::SetProjectionPerspective(float fovDegrees, float nearZ, float farZ)
{
	m_fovDegrees = fovDegrees;
	m_nearClipZ = nearZ;
	m_farClipZ = farZ;

	float aspect = g_window->GetClientAspect();
	m_projectionMatrix = Matrix44::MakePerspective(fovDegrees, aspect, nearZ, farZ);
	m_currentProjection = CAMERA_PROJECTION_PERSPECTIVE;
}


//-------------------------------------------------------------------------------------------------
void Camera::UpdateUBO()
{
	// Lazy instantiation
	if (m_cameraUBO == nullptr)
	{
		m_cameraUBO = new UniformBuffer();
	}

	CameraUBOLayout cameraData;
	cameraData.m_cameraMatrix = m_transform.GetLocalToWorldMatrix();
	cameraData.m_viewMatrix = InvertLookAtMatrix(cameraData.m_cameraMatrix);
	cameraData.m_projectionMatrix = m_projectionMatrix;
	cameraData.m_viewportTopLeftX = 0.f;
	cameraData.m_viewportTopLeftY = 0.f;
	cameraData.m_viewportWidth = (float)m_renderTarget->GetWidth();
	cameraData.m_viewportHeight = (float)m_renderTarget->GetHeight();

	m_cameraUBO->CopyToGPU(&cameraData, sizeof(cameraData));
}


//-------------------------------------------------------------------------------------------------
void Camera::Rotate(const Vector3& deltaEulerAnglesDegrees)
{
	m_transform.Rotate(deltaEulerAnglesDegrees);
	m_viewMatrix = InvertLookAtMatrix(m_transform.GetLocalToWorldMatrix());
}


//-------------------------------------------------------------------------------------------------
void Camera::LookAt(const Vector3& position, const Vector3& target, const Vector3& up /*= Vector3::Y_AXIS*/)
{
	Matrix44 cameraMatrix = Matrix44::MakeLookAt(position, target, up);

	m_transform.position = position;
	m_transform.rotation = Quaternion::FromMatrix(cameraMatrix);

	m_transform.SetLocalMatrix(cameraMatrix);
	m_viewMatrix = InvertLookAtMatrix(cameraMatrix);
}


//-------------------------------------------------------------------------------------------------
void Camera::SetCameraMatrix(const Matrix44& cameraMatrix)
{
	m_transform.SetLocalMatrix(cameraMatrix);
	m_viewMatrix = InvertLookAtMatrix(cameraMatrix);
}


//-------------------------------------------------------------------------------------------------
void Camera::SetViewMatrix(const Matrix44& viewMatrix)
{
	m_viewMatrix = viewMatrix;
	m_transform.SetLocalMatrix(InvertLookAtMatrix(viewMatrix));
}


Texture2D* Camera::GetRenderTarget() const
{
	return m_renderTarget;
}


//-------------------------------------------------------------------------------------------------
Texture2D* Camera::GetDepthTarget() const
{
	return m_depthTarget;
}


//-------------------------------------------------------------------------------------------------
RenderTargetView* Camera::GetRenderTargetView()
{
	return m_renderTarget->CreateOrGetColorTargetView();
}


//-------------------------------------------------------------------------------------------------
DepthStencilTargetView* Camera::GetDepthStencilTargetView()
{
	if (m_depthTarget != nullptr)
	{
		return m_depthTarget->CreateOrGetDepthStencilTargetView();
	}

	return nullptr;
}


//-------------------------------------------------------------------------------------------------
Matrix44 Camera::GetCameraMatrix()
{
	return m_transform.GetLocalToParentMatrix();
}


//-------------------------------------------------------------------------------------------------
Matrix44 Camera::GetViewMatrix()
{
	// Matrix may be out of data - update to be sure
	Matrix44 cameraMatrix = m_transform.GetLocalToParentMatrix();
	m_viewMatrix = InvertLookAtMatrix(cameraMatrix);

	return m_viewMatrix;
}


//-------------------------------------------------------------------------------------------------
Matrix44 Camera::GetProjectionMatrix() const
{
	return m_projectionMatrix;
}


//-------------------------------------------------------------------------------------------------
AABB2 Camera::GetOrthoBounds() const
{
	ASSERT_RECOVERABLE(m_currentProjection == CAMERA_PROJECTION_ORTHOGRAPHIC, "Camera projection not orthographic!");
	return m_orthoBounds;
}


//-------------------------------------------------------------------------------------------------
Vector3 Camera::GetPosition() const
{
	return m_transform.position;
}


//-------------------------------------------------------------------------------------------------
Vector3 Camera::GetRotation() const
{
	return m_transform.rotation.GetAsEulerAngles();
}


//-------------------------------------------------------------------------------------------------
Vector3 Camera::GetForwardVector()
{
	return m_transform.GetLocalToWorldMatrix().GetKVector().xyz();
}


//-------------------------------------------------------------------------------------------------
Vector3 Camera::GetRightVector()
{
	return m_transform.GetLocalToWorldMatrix().GetIVector().xyz();
}


//-------------------------------------------------------------------------------------------------
Vector3 Camera::GetUpVector()
{
	return m_transform.GetLocalToWorldMatrix().GetJVector().xyz();
}


//-------------------------------------------------------------------------------------------------
bool Camera::Event_WindowResize(NamedProperties& args)
{
	if (m_currentProjection == CAMERA_PROJECTION_ORTHOGRAPHIC)
	{
		// Preserve height
		float height = m_orthoBounds.GetHeight();
		float aspect = args.Get("client-aspect", m_orthoBounds.GetAspect());
		SetProjectionOrthographic(height, aspect);
	}

	return false;
}
