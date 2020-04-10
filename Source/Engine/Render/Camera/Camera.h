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
#include "Engine/Math/Matrix44.h"
#include "Engine/Math/Transform.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class RenderTargetView;
class DepthStencilTargetView;
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

	void					SetColorTargetView(RenderTargetView* colorTargetView, bool ownsColorTargetView);
	void					SetDepthStencilTargetView(DepthStencilTargetView* depthTargetView, bool ownsTarget);
	void					SetProjectionOrtho(float orthoHeight);
	void					SetProjectionPerspective(float fovDegrees, float nearZ, float farZ);
	void					UpdateUBO();

	void					SetPosition(const Vector3& position);
	void					Translate(const Vector3& translation);
	void					SetRotation(const Vector3& rotation);
	void					Rotate(const Vector3& rotationEulerAngles);

	void					SetFOV(float fovDegrees) { m_fovDegrees = fovDegrees; }
	void					LookAt(const Vector3& position, const Vector3& target, const Vector3& up = Vector3::Y_AXIS);
	void					SetCameraMatrix(const Matrix44& cameraMatrix);
	void					SetViewMatrix(const Matrix44& viewMatrix);

	RenderTargetView*		GetColorTargetView() const { return m_colorTargetView; }
	DepthStencilTargetView* GetDepthStencilTargetView() const { return m_depthTargetView; }
	UniformBuffer*			GetUniformBuffer() const { return m_cameraUBO; }

	Matrix44				GetCameraMatrix();
	Matrix44				GetViewMatrix();
	Matrix44				GetProjectionMatrix() const;

	Vector3					GetPosition() const;
	Vector3					GetRotation() const;

	Vector3					GetForwardVector();
	Vector3					GetRightVector();
	Vector3					GetUpVector();

	bool					Event_WindowResize(NamedProperties& args);


private:
	//-----Private Data-----

	// Model/View Data
	Transform				m_transform;
	Matrix44				m_viewMatrix;
	
	// Projection
	Matrix44				m_projectionMatrix;
	AABB2					m_orthoBounds;
	float					m_fovDegrees = 90.f;
	float					m_nearClipZ = 0.f;
	float					m_farClipZ = 1.f;
	CameraProjection		m_currentProjection = CAMERA_PROJECTION_NONE;

	// Render Target
	RenderTargetView*		m_colorTargetView = nullptr;
	DepthStencilTargetView* m_depthTargetView = nullptr;
	bool					m_ownsColorTargetView = false;
	bool					m_ownsDepthTargetView = false;

	// Misc
	UniformBuffer*			m_cameraUBO = nullptr;

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
