///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: December 15th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Math/AABB2.h"
#include "Engine/Math/Matrix4.h"
#include "Engine/Math/Transform.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class DepthStencilTargetView;
class RenderTargetView;
class Texture2D;
class UniformBuffer;

enum CameraProjection
{
	CAMERA_PROJECTION_NONE,
	CAMERA_PROJECTION_ORTHOGRAPHIC,
	CAMERA_PROJECTION_PERSPECTIVE
};
///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class Camera
{
public:
	//-----Public Methods-----

	Camera();
	~Camera();

	void					SetRenderTarget(Texture2D* renderTarget, bool ownsTarget);
	void					SetDepthTarget(Texture2D* depthTarget, bool ownsTarget);
	void					SetProjection(CameraProjection projectionType, const Matrix4& projection);
	void					SetProjectionOrthographic(float orthoHeight, float aspect);
	void					SetProjectionPerspective(float fovDegrees, float nearZ, float farZ);
	void					UpdateUBO();

	void					SetPosition(const Vector3& position);
	void					Translate(const Vector3& translation);
	void					SetRotationEulerAnglesDegrees(const Vector3& eulerAnglesDegrees);
	void					SetRotationEulerAnglesRadians(const Vector3& eulerAnglesRadians);
	void					RotateEulerAnglesDegrees(const Vector3& deltaEulerAnglesDegrees);
	void					RotateEulerAnglesRadians(const Vector3& deltaEulerAnglesRadians);

	void					SetFOV(float fovDegrees) { m_fovDegrees = fovDegrees; }
	void					LookAt(const Vector3& position, const Vector3& target, const Vector3& up = Vector3::Y_AXIS);
	void					SetCameraMatrix(const Matrix4& cameraMatrix);
	void					SetViewMatrix(const Matrix4& viewMatrix);

	Texture2D*				GetRenderTarget() const;
	Texture2D*				GetDepthTarget() const;
	UniformBuffer*			GetUniformBuffer() const { return m_cameraUBO; }

	RenderTargetView*		GetRenderTargetView();
	DepthStencilTargetView*	GetDepthStencilTargetView();

	Matrix4					GetCameraMatrix();
	Matrix4					GetViewMatrix();
	Matrix4					GetProjectionMatrix() const;
	AABB2					GetOrthoBounds() const;
	
	Vector3					GetPosition() const;
	Vector3					GetRotationAsEulerAnglesDegrees() const;
	Vector3					GetRotationAsEulerAnglesRadians() const;

	Vector3					GetForwardVector();
	Vector3					GetRightVector();
	Vector3					GetUpVector();

	bool					Event_WindowResize(NamedProperties& args);


private:
	//-----Private Data-----

	// Model/View Data
	Transform				m_transform;
	Matrix4				m_viewMatrix;
	
	// Projection
	Matrix4				m_projectionMatrix;
	AABB2					m_orthoBounds;
	float					m_fovDegrees = 90.f;
	float					m_nearClipZ = 0.f;
	float					m_farClipZ = 1.f;
	CameraProjection		m_currentProjection = CAMERA_PROJECTION_NONE;

	// Render Target
	Texture2D*				m_renderTarget = nullptr;
	Texture2D*				m_depthTarget = nullptr;
	bool					m_ownsRenderTarget = false;
	bool					m_ownsDepthTarget = false;

	// Misc
	UniformBuffer*			m_cameraUBO = nullptr;

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
