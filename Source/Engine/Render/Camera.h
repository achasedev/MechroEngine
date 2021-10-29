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
#include "Engine/Math/Frustrum.h"
#include "Engine/Math/Matrix4.h"
#include "Engine/Math/Transform.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class DepthStencilView;
class RenderTargetView;
class Texture2D;
class ConstantBuffer;

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
	void					SetProjectionOrthographic(float orthoHeight, float aspect, float nearZ = -1.0f, float farZ = 1.0f);
	void					SetProjectionOrthographic(const Vector2& bottomLeft, const Vector2& topRight, float nearZ = -1.0f, float farZ = 1.0f);
	void					SetProjectionPerspective(float fovDegrees, float aspect, float nearZ, float farZ);
	void					UpdateUBO();
	void					ClearColorTarget(const Rgba& clearColor);
	void					ClearDepthTarget(float depth = 1.0f);

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
	void					SetDrawOrder(int drawOrder) { m_drawOrder = drawOrder; }

	Texture2D*				GetRenderTarget() const;
	Texture2D*				GetDepthTarget() const;
	ConstantBuffer*			GetUniformBuffer() const { return m_cameraUBO; }

	RenderTargetView*		GetRenderTargetView();
	DepthStencilView*		GetDepthStencilTargetView();

	Matrix4					GetCameraMatrix();
	Matrix4					GetViewMatrix();
	Matrix4					GetProjectionMatrix() const;
	AABB2					GetOrthoBounds() const;
	int						GetDrawOrder() const { return m_drawOrder; }
	float					GetNearClip() const { return m_nearClipZ; }
	float					GetFarClip() const { return m_farClipZ; }
	float					GetFOV() const { return m_fovDegrees; }

	Vector3					GetPosition() const;
	Vector3					GetRotationAsEulerAnglesDegrees() const;
	Vector3					GetRotationAsEulerAnglesRadians() const;

	Vector3					GetForwardVector();
	Vector3					GetRightVector();
	Vector3					GetUpVector();
	Frustrum				GetFrustrum();

	bool					Event_WindowResize(NamedProperties& args);


public:
	//-----Public Data-----
	Transform				transform;


private:
	//-----Private Data-----

	// Model/View Data
	Matrix4					m_viewMatrix;
	
	// Projection
	Matrix4					m_projectionMatrix;
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
	ConstantBuffer*			m_cameraUBO = nullptr;
	int						m_drawOrder = 0;

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
