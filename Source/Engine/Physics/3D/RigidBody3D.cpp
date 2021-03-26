///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 25th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/Polygon3d.h"
#include "Engine/Physics/3D/RigidBody3D.h"

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
RigidBody3D::RigidBody3D(PhysicsScene3D* scene, GameObject* owningObj)
	: m_scene(scene)
	, m_gameObj(owningObj)
	, m_transform(&owningObj->m_transform) // Convenience
	, m_shapeLs(owningObj->GetShape3D())
{
	ASSERT_RECOVERABLE(m_scene != nullptr, "RigidBody3D's scene is nullptr");
	ASSERT_RECOVERABLE(m_gameObj != nullptr, "RigidBody3D's object is nullptr!");
	ASSERT_RECOVERABLE(m_shapeLs != nullptr, "RigidBody3D's shape is nullptr!");

	CalculateCenterOfMass(); // Purely positional, assumes uniform mass density
}


//-------------------------------------------------------------------------------------------------
RigidBody3D::~RigidBody3D()
{
}


//-------------------------------------------------------------------------------------------------
void RigidBody3D::CalculateCenterOfMass()
{
	float volume = 0.f;
	Vector3 center = Vector3::ZERO;

	int numFaces = m_shapeLs->GetNumFaces();
	for (int faceIndex = 0; faceIndex < numFaces; ++faceIndex)
	{
		// Iterate across all the vertices of the face
		const PolygonFace3d* face = m_shapeLs->GetFace(faceIndex);
		int numVerticesInFace = face->GetNumVertices();

		Vector3 origin = Vector3::ZERO;
		Vector3 a = face->GetVertex(0);

		for (int vertexIndex = 1; vertexIndex < numVerticesInFace - 1; ++vertexIndex)
		{
			Vector3 b = face->GetVertex(vertexIndex);
			Vector3 c = face->GetVertex(vertexIndex + 1);

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
void RigidBody3D::GetWorldShape(Polygon3d& out_polygonWs) const
{
	Matrix44 toWorldMat = m_transform->GetLocalToWorldMatrix();
	//m_shapeLs->GetTransformed(toWorldMat, out_polygonWs);
}


//-------------------------------------------------------------------------------------------------
Vector3 RigidBody3D::GetCenterOfMassWs() const
{
	Matrix44 toWorld = m_transform->GetLocalToWorldMatrix();
	return toWorld.TransformPoint(m_centerOfMassLs).xyz();
}


//-------------------------------------------------------------------------------------------------
void RigidBody3D::SetMassProperties(float mass)
{
	UNUSED(mass);
	UNIMPLEMENTED();

	//if (mass == FLT_MAX)
	//{
	//	// Just set these to sensible defaults
	//	m_mass = mass;
	//	m_invMass = 0.f;
	//	m_inertia = FLT_MAX;
	//	m_invInertia = 0.f;
	//	m_density = FLT_MAX;
	//	return;
	//}

	//ASSERT_RETURN(mass > 0.f, NO_RETURN_VAL, "Setting 0.f mass!");

	//float area = 0.f;
	//Vector2 center = Vector2::ZERO;
	//float inertia = 0.f;

	//// THE IDEA
	//// Determine the moment of inertia at the origin while we find the center of mass
	//// Then use parallel axis theorem at the end to determine inertia at the center of mass
	//uint32 numVertices = m_shapeLs->GetNumVertices();
	//for (uint32 currIndex = 0; currIndex < numVertices; ++currIndex)
	//{
	//	uint32 nextIndex = (currIndex == numVertices - 1 ? 0 : currIndex + 1);

	//	Vector3 a = m_shapeLs->GetVertex(currIndex);
	//	Vector3 b = m_shapeLs->GetVertex(nextIndex);

	//	float currArea = 0.5f * CrossProduct(a, b);
	//	Vector2 currCenter = 0.33333f * (a + b); // No need to add origin = (0,0) here
	//	float currInertia = currArea * (DotProduct(a, a) + DotProduct(b, b) + DotProduct(a, b)) * 0.16666666667f; // Divide by 6

	//	// Update running totals
	//	center = (center * area + currCenter * currArea) / (area + currArea); // Move center weighted by areas
	//	area += currArea;
	//	inertia += currInertia;
	//}

	//// Factor in mass to the inertia, since intertia was calculated using areas
	//float density = mass / area;
	//inertia *= density;

	//// Parallel axis theorem: I_origin = I_center_of_mass + mass * (distance from center to origin)^2
	//inertia -= mass * DotProduct(center, center);

	//m_mass = mass;
	//m_invMass = (1.0f / mass);

	//m_inertia = inertia;
	//m_invInertia = (1.0f / inertia);

	//m_density = density;
	//m_centerOfMassLs = center;
}
