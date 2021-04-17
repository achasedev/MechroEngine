///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 25th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Collision/3D/Collider3d.h"
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/Polygon3d.h"
#include "Engine/Physics/3D/RigidBody3D.h"
#include "Engine/Render/Core/RenderContext.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

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
void RigidBody3D::CalculateCenterOfMass()
{
	float volume = 0.f;
	Vector3 center = Vector3::ZERO;

	const Polygon3d* localShape = GetLocalShape();
	int numFaces = localShape->GetNumFaces();
	for (int faceIndex = 0; faceIndex < numFaces; ++faceIndex)
	{
		// Iterate across all the vertices of the face
		const PolygonFace3d* face = localShape->GetFace(faceIndex);
		int numVerticesInFace = (int)face->m_indices.size();

		Vector3 origin = Vector3::ZERO;
		Vector3 a = localShape->GetVertex(face->m_indices[0])->m_position;

		for (int faceVertexIndex = 1; faceVertexIndex < numVerticesInFace - 1; ++faceVertexIndex)
		{
			Vector3 b = localShape->GetVertexPosition(face->m_indices[faceVertexIndex]);
			Vector3 c = localShape->GetVertexPosition(face->m_indices[faceVertexIndex + 1]);

			float currVolume = CalculateVolumeOfTetrahedron(a, b, c, origin);
			Vector3 currCenter = 0.25f * (a + b + c + origin);

			// Update running totals
			center = (center * volume + currCenter * currVolume) / (volume + currVolume); // Move center weighted by volumes
			volume += currVolume;
		}
	}

	m_centerOfMassLs = center;
}


//-------------------------------------------------------------------------------------------------
const Polygon3d* RigidBody3D::GetLocalShape() const
{
	if (m_collider != nullptr)
	{
		return m_collider->GetLocalShape();
	}

	// TODO: Return our own shape
	return nullptr;
}


//-------------------------------------------------------------------------------------------------
Vector3 RigidBody3D::GetCenterOfMassWs() const
{
	Matrix44 toWorld = m_transform->GetLocalToWorldMatrix();
	return toWorld.TransformPoint(m_centerOfMassLs).xyz();
}


//-------------------------------------------------------------------------------------------------
void RigidBody3D::DebugRender(Material* material, const Rgba& color)
{
	// Debug render in world space
	g_renderContext->DrawWirePolygon3D(*m_collider->GetWorldShape(), material, color);
	g_renderContext->DrawPoint3D(GetCenterOfMassWs(), 0.25f, material, Rgba::YELLOW);
}


//-------------------------------------------------------------------------------------------------
void RigidBody3D::SetMassProperties(float mass)
{
	if (mass == FLT_MAX)
	{
		// Just set these to sensible defaults
		m_mass = mass;
		m_invMass = 0.f;
		m_inertia = Vector3(FLT_MAX);
		m_invInertia = Vector3::ZERO;
		m_density = FLT_MAX;
		return;
	}

	ASSERT_RETURN(mass > 0.f, NO_RETURN_VAL, "Setting 0.f mass!");

	float volume = 0.f;
	Vector3 inertia = Vector3::ZERO;
	Vector3 center = Vector3::ZERO;

	const Polygon3d* localShape = GetLocalShape();
	int numFaces = localShape->GetNumFaces();
	for (int faceIndex = 0; faceIndex < numFaces; ++faceIndex)
	{
		// Iterate across all the vertices of the face
		const PolygonFace3d* face = localShape->GetFace(faceIndex);
		int numVerticesInFace = (int)face->m_indices.size();

		Vector3 origin = Vector3::ZERO;
		Vector3 a = localShape->GetVertex(face->m_indices[0])->m_position;

		for (int faceVertexIndex = 1; faceVertexIndex < numVerticesInFace - 1; ++faceVertexIndex)
		{
			Vector3 b = localShape->GetVertexPosition(face->m_indices[faceVertexIndex]);
			Vector3 c = localShape->GetVertexPosition(face->m_indices[faceVertexIndex + 1]);

			float currVolume = CalculateVolumeOfTetrahedron(a, b, c, origin);
			Vector3 currCenter = 0.25f * (a + b + c + origin);

			Vector3 currInertia;
			currInertia.x = Vector3(0.f, currCenter.y, currCenter.z).GetLengthSquared();
			currInertia.y = Vector3(currCenter.x, 0.f, currCenter.z).GetLengthSquared();
			currInertia.z = Vector3(currCenter.x, currCenter.y, 0.f).GetLengthSquared();

			currInertia *= currVolume;

			// Update running totals
			center = (center * volume + currCenter * currVolume) / (volume + currVolume); // Move center weighted by volumes
			volume += currVolume;
			inertia += currInertia;
		}
	}

	m_centerOfMassLs = center;

	// Factor in mass to the inertia, since intertia was calculated using areas
	float density = mass / volume;
	inertia *= density; // I = Mr^2, so multiply by mass, and divide by volume since this is a weighted sum of all partial volumes

	// Parallel axis theorem: I_origin = I_center_of_mass + mass * (distance from center to origin)^2
	inertia.x -= mass * Vector3(0.f, center.y, center.z).GetLengthSquared();
	inertia.y -= mass * Vector3(center.x, 0.f, center.z).GetLengthSquared();
	inertia.z -= mass * Vector3(center.x, center.y, 0.f).GetLengthSquared();

	m_mass = mass;
	m_invMass = (1.0f / mass);

	m_inertia = inertia;
	m_invInertia = Vector3(1.0f / inertia.x, 1.0f / inertia.y, 1.0f * inertia.z);

	m_density = density;
	m_centerOfMassLs = center;
}
