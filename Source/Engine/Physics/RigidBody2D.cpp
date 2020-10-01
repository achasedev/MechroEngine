///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: September 17th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/Polygon2D.h"
#include "Engine/Physics/RigidBody2D.h"

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
RigidBody2D::RigidBody2D(PhysicsScene2D* scene, GameObject* owningObj)
	: m_scene(scene)
	, m_gameObj(owningObj)
	, m_transform(&owningObj->m_transform) // Convenience
	, m_shapeLs(owningObj->GetShape())
{
	ASSERT_RECOVERABLE(m_scene != nullptr, "RigidBody2D's scene is nullptr");
	ASSERT_RECOVERABLE(m_gameObj != nullptr, "RigidBody2D's object is nullptr!");
	ASSERT_RECOVERABLE(m_shapeLs != nullptr, "RigidBody2D's shape is nullptr!");

	ASSERT_RECOVERABLE(m_shapeLs->IsConvex(), "Rigidbody2D shape is not convex!");
	ASSERT_RECOVERABLE(m_shapeLs->IsWindingClockwise(), "Rigidbody2D shape is not clockwise winding!");

	CalculateCenterOfMass(); // Purely positional, assumes uniform mass density
}


//-------------------------------------------------------------------------------------------------
RigidBody2D::~RigidBody2D()
{
}


//-------------------------------------------------------------------------------------------------
void RigidBody2D::CalculateCenterOfMass()
{
	float area = 0.f;
	Vector2 center = Vector2::ZERO;

	uint32 numVertices = m_shapeLs->GetNumVertices();
	for (uint32 currIndex = 0; currIndex < numVertices; ++currIndex)
	{
		uint32 nextIndex = (currIndex == numVertices - 1 ? 0 : currIndex + 1);

		Vector2 a = m_shapeLs->GetVertexAtIndex(currIndex);
		Vector2 b = m_shapeLs->GetVertexAtIndex(nextIndex);

		float currArea = 0.5f * CrossProduct(a, b);
		Vector2 currCenter = 0.33333f * (a + b); // No need to add origin = (0,0) here

		// Update running totals
		center = (center * area + currCenter * currArea) / (area + currArea); // Move center weighted by areas
		area += currArea;
	}

	m_centerOfMassLs = center;
}


//-------------------------------------------------------------------------------------------------
void RigidBody2D::GetWorldShape(Polygon2D& out_polygon) const
{
	out_polygon.Clear();
	Matrix44 toWorldMat = m_transform->GetLocalToWorldMatrix();

	// Convert local space vertices to world space
	for (uint32 vertexIndex = 0; vertexIndex < m_shapeLs->GetNumVertices(); ++vertexIndex)
	{
		Vector3 currVertex3d = Vector3(m_shapeLs->GetVertexAtIndex(vertexIndex), 0.f);
		Vector2 worldVertex2d = toWorldMat.TransformPoint(currVertex3d).xy();

		out_polygon.AddVertex(worldVertex2d);
	}
}


//-------------------------------------------------------------------------------------------------
Vector2 RigidBody2D::GetCenterOfMassWs() const
{
	Matrix44 toWorld = m_transform->GetLocalToWorldMatrix();
	return toWorld.TransformPoint(m_centerOfMassLs).xy();
}


//-------------------------------------------------------------------------------------------------
void RigidBody2D::SetMassProperties(float mass)
{
	if (mass == FLT_MAX)
	{
		// Just set these to sensible defaults
		m_mass = mass;
		m_invMass = 0.f;
		m_inertia = FLT_MAX;
		m_invInertia = 0.f;
		m_density = FLT_MAX;
		return;
	}

	ASSERT_RETURN(mass > 0.f, NO_RETURN_VAL, "Setting 0.f mass!");

	float area = 0.f;
	Vector2 center = Vector2::ZERO;
	float inertia = 0.f;

	// THE IDEA
	// Determine the moment of inertia at the origin while we find the center of mass
	// Then use parallel axis theorem at the end to determine inertia at the center of mass
	uint32 numVertices = m_shapeLs->GetNumVertices();
	for (uint32 currIndex = 0; currIndex < numVertices; ++currIndex)
	{
		uint32 nextIndex = (currIndex == numVertices - 1 ? 0 : currIndex + 1);

		Vector2 a = m_shapeLs->GetVertexAtIndex(currIndex);
		Vector2 b = m_shapeLs->GetVertexAtIndex(nextIndex);

		float currArea = 0.5f * CrossProduct(a, b);
		Vector2 currCenter = 0.33333f * (a + b); // No need to add origin = (0,0) here
		float currInertia = currArea * (DotProduct(a, a) + DotProduct(b, b) + DotProduct(a, b)) * 0.16666666667f; // Divide by 6

		// Update running totals
		center = (center * area + currCenter * currArea) / (area + currArea); // Move center weighted by areas
		area += currArea;
		inertia += currInertia;
	}

	// Factor in mass to the inertia, since intertia was calculated using areas
	float density = mass / area;
	inertia *= density;

	// Parallel axis theorem: I_origin = I_center_of_mass + mass * (distance from center to origin)^2
	inertia -= mass * DotProduct(center, center);

	m_mass = mass;
	m_invMass = (1.0f / mass);

	m_inertia = inertia;
	m_invInertia = (1.0f / inertia);

	m_density = density;
	m_centerOfMassLs = center;
}
