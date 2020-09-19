///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: September 17th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
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
void RigidBody2D::SetMass(float mass)
{

	if (mass < FLT_MAX)
	{
		ASSERT_RETURN(mass > 0.f, NO_RETURN_VAL, "Setting a RigidBody2D mass to 0.f!");
		CalculateMassProperties(mass);
	}
	else
	{
		// Just set these to sensible defaults
		m_mass = mass;
		m_invMass = 0.f;
		m_inertia = FLT_MAX;
		m_invInertia = 0.f;
		m_density = FLT_MAX;
	}
}


//-------------------------------------------------------------------------------------------------
void RigidBody2D::CalculateMassProperties(float mass)
{
	float area = 0.f;
	Vector2 center = Vector2::ZERO;
	float inertia = 0.f;

	// THE IDEA
	// Determine the moment of inertia at the origin while we find the center of mass
	// Then use parallel axis theorem at the end to determine interia at the center of mass
	uint32 numVertices = m_shape->GetNumVertices();
	for (uint32 currIndex = 0; currIndex < numVertices; ++currIndex)
	{
		uint32 nextIndex = (currIndex == numVertices - 1 ? 0 : currIndex + 1);

		Vector2 a = m_shape->GetVertexAtIndex(currIndex);
		Vector2 b = m_shape->GetVertexAtIndex(nextIndex);

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
}
