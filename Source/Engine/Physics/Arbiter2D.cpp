///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: September 18th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/Polygon2D.h"
#include "Engine/Physics/Arbiter2D.h"
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
const float Arbiter2D::ALLOWED_PENETRATION = 0.01f;
const float Arbiter2D::BIAS_FACTOR = 0.2f; // Always do position correction for now?
const bool Arbiter2D::ACCUMULATE_IMPULSES = true; // Always for now?

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
Arbiter2D::Arbiter2D(RigidBody2D* body1, RigidBody2D* body2)
{
	// Store them in order
	if (body1 < body2)
	{
		m_body1 = body1;
		m_body2 = body2;
	}
	else
	{
		m_body1 = body2;
		m_body2 = body1;
	}

	// Combined friction
	m_friction = Sqrt(m_body1->GetFriction() * m_body2->GetFriction());
}


//-------------------------------------------------------------------------------------------------
void Arbiter2D::DetectCollision()
{
	Polygon2D poly1, poly2;
	m_body1->GetWorldShape(poly1);
	m_body2->GetWorldShape(poly2);

	// Detect collision
	CollisionSeparation2D separation = CalculateSeparation2D(&poly1, &poly2);
	m_numContacts = 0;

	if (separation.m_collisionFound)
	{
		// Find the contact points of the collision
		// http://www.dyn4j.org/2011/11/contact-points-using-clipping/ for reference
		CalculateContactPoints(&poly1, &poly2, separation);
	}
}


//-------------------------------------------------------------------------------------------------
void Arbiter2D::PreStep(float deltaSeconds)
{
	float invDeltaSeconds = (deltaSeconds > 0.f ? 1.0f / deltaSeconds : 0.f);

	for (uint32 contactIndex = 0; contactIndex < m_numContacts; ++contactIndex)
	{
		Contact2D* contact = m_contacts + contactIndex;

		// Precompute normal mass, tangent mass, and bias
		// Mass normal is used to calculate impulse necessary to prevent penetration
		float r1Normal = DotProduct(contact->m_r1, contact->m_normal);
		float r2Normal = DotProduct(contact->m_r2, contact->m_normal);

		float kNormal = m_body1->m_invMass * m_body2->m_invMass;
		kNormal += m_body1->m_invMass * (DotProduct(contact->m_r1, contact->m_r1) - r1Normal * r1Normal) + m_body2->m_invMass * (DotProduct(contact->m_r2, contact->m_r2) - r2Normal * r2Normal);
		contact->m_massNormal = 1.0f / kNormal;

		// Right-handed perp for tangent
		// Mass tangent is used to calculate impulse to simulate friction
		Vector2 tangent = Vector2(contact->m_normal.y, -1.0f * contact->m_normal.x);
		float r1Tangent = DotProduct(contact->m_r1, tangent);
		float r2Tangent = DotProduct(contact->m_r2, tangent);

		float kTangent = m_body1->m_invMass * m_body2->m_invMass;;
		kTangent += m_body1->m_invMass * (DotProduct(contact->m_r1, contact->m_r1) - r1Tangent * r1Tangent) + m_body2->m_invMass * (DotProduct(contact->m_r2, contact->m_r2) - r2Tangent * r2Tangent);
		contact->m_massTangent = 1.0f / kTangent;

		// To quote Erin Catto, this gives the normal impulse "some extra oomph"
		// Proportional to the penetration, so if two objects are really intersecting -> greater bias -> greater normal force -> larger correction this frame
		// Allowed penetration means this will correct over time, not instantaneously - make it less jittery?
		contact->m_bias = -BIAS_FACTOR * invDeltaSeconds * (contact->m_separation + ALLOWED_PENETRATION); // separation is *always* negative, it's distance below the reference edge

		// Apply old accumulated impulses at the beginning of the step
		// This leads to less iterations and greater stability
		// This is considered "warm starting"
		if (ACCUMULATE_IMPULSES)
		{
			Vector2 impulse = contact->accumulatedNormalImpulse * contact->m_normal + contact->m_accumulatedTangentImpulse * tangent;

			// Q. But friction impulse should be in some way related to relative velocity! Yet there's no velocity here!
			// A. These incrementals were calculated last frame in ApplyImpulse(), so unless velocity instantaneous and largely changed outside
			//    the physics system this warm start should feel "continuous" from where it just left off
			m_body1->m_velocityWs -= m_body1->m_invMass * impulse;
			m_body1->m_angularVelocityDegrees -= m_body1->m_invInertia * CrossProduct(contact->m_r1, impulse);

			m_body2->m_velocityWs += m_body2->m_invMass * impulse;
			m_body2->m_angularVelocityDegrees += m_body2->m_invInertia * CrossProduct(contact->m_r2, impulse);
		}
	}
}


//-------------------------------------------------------------------------------------------------
void Arbiter2D::ApplyImpulse()
{
	for (uint32 contactIndex = 0; contactIndex < m_numContacts; ++contactIndex)
	{
		Contact2D* contact = m_contacts + contactIndex;

		stop - do ArbIter update first
	}
}


//-------------------------------------------------------------------------------------------------
void Arbiter2D::CalculateContactPoints(const Polygon2D* poly1, const Polygon2D* poly2, const CollisionSeparation2D& separation)
{
	// Find the best edges for each polygon (normal is from A)
	CollisionFeatureEdge2D edge1 = GetFeatureEdge2D(poly1, separation.m_dirFromFirst);
	CollisionFeatureEdge2D edge2 = GetFeatureEdge2D(poly2, -1.0f * separation.m_dirFromFirst);
	
	// Determine which is the reference edge and which is the incident edge
	// Reference edge is the one more closely perpendicular to the separation direction
	float dot1 = DotProduct(edge1.m_normal, separation.m_dirFromFirst);
	float dot2 = DotProduct(edge2.m_normal, separation.m_dirFromFirst);

	const CollisionFeatureEdge2D* referenceEdge = nullptr;
	const CollisionFeatureEdge2D* incidentEdge = nullptr;

	if (Abs(dot1) > Abs(dot2))
	{
		// poly1 is our reference
		referenceEdge = &edge1;
		incidentEdge = &edge2;
	}
	else
	{
		// poly2 is our reference
		referenceEdge = &edge2;
		incidentEdge = &edge1;
	}

	Vector2 refEdgeDirection = referenceEdge->m_vertex2 - referenceEdge->m_vertex1;
	refEdgeDirection.Normalize();
	
	// Keep all our results from each clip for debugging purposes
	std::vector<Vector2> clippedPoints1;

	// Clip the incident edge to the start of the reference edge
	// First determine the min value the dot would need to be in order to be inside the clipping edge
	float startDot = DotProduct(refEdgeDirection, referenceEdge->m_vertex1);
	ClipIncidentEdgeToReferenceEdge(incidentEdge->m_vertex1, incidentEdge->m_vertex2, refEdgeDirection, startDot, clippedPoints1);
	
	if (clippedPoints1.size() < 2)
	{
		return;
	}

	// Now clip the incident edge to the end of the reference edge
	// So clip in the opposite direction, flip some signs
	float endDot = DotProduct(refEdgeDirection, referenceEdge->m_vertex2);

	std::vector<Vector2> clippedPoints2;
	ClipIncidentEdgeToReferenceEdge(clippedPoints1[0], clippedPoints1[1], -1.0f * refEdgeDirection, -1.0f * endDot, clippedPoints2);

	if (clippedPoints2.size() < 2)
	{
		return;
	}

	// Finally, clip all contacts that are outside the reference edge
	// It's ok to not have 2 contact points after this step!
	Vector2 refNormalForClipping = referenceEdge->m_normal;

	// Get the largest depth a contact can have
	float maxDepth = DotProduct(refNormalForClipping, referenceEdge->m_furthestVertex);

	// If any of these points are "deeper" than the max depth then they are in the collision manifold
	float penDepth1 = DotProduct(refNormalForClipping, clippedPoints2[0]) - maxDepth;
	float penDepth2 = DotProduct(refNormalForClipping, clippedPoints2[1]) - maxDepth;

	if (penDepth1 < 0.f)
	{
		m_contacts[m_numContacts].m_position = clippedPoints2[0];
		m_contacts[m_numContacts].m_normal = refNormalForClipping;
		m_contacts[m_numContacts].m_r1 = clippedPoints2[0] - m_body1->GetCenterOfMassWs();
		m_contacts[m_numContacts].m_r2 = clippedPoints2[0] - m_body2->GetCenterOfMassWs();
		m_contacts[m_numContacts].m_separation = penDepth1;
		m_contacts[m_numContacts].m_referenceEdge = *referenceEdge;
		m_contacts[m_numContacts].m_incidentEdge = *incidentEdge;
		m_numContacts++;
	}

	if (penDepth2 < 0.f)
	{
		m_contacts[m_numContacts].m_position = clippedPoints2[1];
		m_contacts[m_numContacts].m_normal = refNormalForClipping;
		m_contacts[m_numContacts].m_r1 = clippedPoints2[1] - m_body1->GetCenterOfMassWs();
		m_contacts[m_numContacts].m_r2 = clippedPoints2[1] - m_body2->GetCenterOfMassWs();
		m_contacts[m_numContacts].m_separation = penDepth2;
		m_contacts[m_numContacts].m_referenceEdge = *referenceEdge;
		m_contacts[m_numContacts].m_incidentEdge = *incidentEdge;
		m_numContacts++;
	}

	ASSERT_OR_DIE(m_numContacts <= 2, "Bad number of contacts!");
}
