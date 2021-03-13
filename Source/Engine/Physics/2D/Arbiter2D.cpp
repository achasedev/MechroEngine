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
#include "Engine/Physics/2D/Arbiter2D.h"
#include "Engine/Physics/2D/RigidBody2D.h"

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
const bool Arbiter2D::WARM_START_ACCUMULATIONS = true;

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
void Arbiter2D::Update(const Contact2D* newContacts, uint32 numNewContacts)
{
	// Where the final result will be stored temporarily
	Contact2D mergedContacts[2];

	for (uint32 newContactIndex = 0U; newContactIndex < numNewContacts; ++newContactIndex)
	{
		const Contact2D* newContact = newContacts + newContactIndex;

		// Search to see if we already have info for this contact stored from last frames result
		Contact2D* matchingOldContact = nullptr;
		for (uint32 oldContactIndex = 0; oldContactIndex < m_numContacts; ++oldContactIndex)
		{
			Contact2D* oldContact = m_contacts + oldContactIndex;

			if (newContact->m_id.m_value == oldContact->m_id.m_value)
			{
				matchingOldContact = oldContact;
				break;
			}
		}

		if (matchingOldContact != nullptr)
		{
			// Overwrite our old contact info with the new stuff
			Contact2D* mergeContact = mergedContacts + newContactIndex;
			*mergeContact = *newContact;

			// If we want to reuse the last accumulation state of the contact, copy it now
			if (WARM_START_ACCUMULATIONS)
			{
				mergeContact->m_accNormalImpulse = matchingOldContact->m_accNormalImpulse;
				mergeContact->m_accTangentImpulse = matchingOldContact->m_accTangentImpulse;
				mergeContact->m_normalBiasImpulse = matchingOldContact->m_normalBiasImpulse;
			}
			else
			{
				mergeContact->m_accNormalImpulse = 0.f;
				mergeContact->m_accTangentImpulse = 0.f;
				mergeContact->m_normalBiasImpulse = 0.f;
			}
		}
		else
		{
			// Brand new contact, just add it to merged contacts
			mergedContacts[newContactIndex] = newContacts[newContactIndex];
		}

	}

	// Done merging, update the arbiter's contact data
	for (uint32 newContactIndex = 0U; newContactIndex < numNewContacts; ++newContactIndex)
	{
		m_contacts[newContactIndex] = mergedContacts[newContactIndex];
	}

	m_numContacts = numNewContacts;
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
inline float Cross(const Vector2& a, const Vector2& b)
{
	return a.x * b.y - a.y * b.x;
}


//-------------------------------------------------------------------------------------------------
inline Vector2 Cross(const Vector2& a, float s)
{
	return Vector2(s * a.y, -s * a.x);
}


//-------------------------------------------------------------------------------------------------
inline Vector2 Cross(float s, const Vector2& a)
{
	return Vector2(-s * a.y, s * a.x);
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

		float kNormal = m_body1->m_invMass + m_body2->m_invMass;
		kNormal += m_body1->m_invInertia * (DotProduct(contact->m_r1, contact->m_r1) - r1Normal * r1Normal) + m_body2->m_invInertia * (DotProduct(contact->m_r2, contact->m_r2) - r2Normal * r2Normal);
		contact->m_massNormal = 1.0f / kNormal;

		// Right-handed perp for tangent
		// Mass tangent is used to calculate impulse to simulate friction
		Vector2 tangent = Vector2(contact->m_normal.y, -1.0f * contact->m_normal.x);
		float r1Tangent = DotProduct(contact->m_r1, tangent);
		float r2Tangent = DotProduct(contact->m_r2, tangent);

		float kTangent = m_body1->m_invMass + m_body2->m_invMass;
		kTangent += m_body1->m_invInertia * (DotProduct(contact->m_r1, contact->m_r1) - r1Tangent * r1Tangent) + m_body2->m_invInertia * (DotProduct(contact->m_r2, contact->m_r2) - r2Tangent * r2Tangent);
		contact->m_massTangent = 1.0f / kTangent;

		// To quote Erin Catto, this gives the normal impulse "some extra oomph"
		// Proportional to the penetration, so if two objects are really intersecting -> greater bias -> greater normal force -> larger correction this frame
		// Allowed penetration means this will correct over time, not instantaneously - make it less jittery?
		contact->m_bias = -BIAS_FACTOR * invDeltaSeconds * Min(contact->m_separation + ALLOWED_PENETRATION, 0.f); // separation is *always* negative, it's distance below the reference edge

		// Apply old accumulated impulses at the beginning of the step
		// This leads to less iterations and greater stability
		// This is considered "warm starting"
		if (ACCUMULATE_IMPULSES)
		{
			Vector2 impulse = contact->m_accNormalImpulse * contact->m_normal + contact->m_accTangentImpulse * tangent;

			// Q. But friction impulse should be in some way related to relative velocity! Yet there's no velocity here!
			// A. These incrementals were calculated last frame in ApplyImpulse(), so unless velocity instantaneous and largely changed outside
			//    the physics system this warm start should feel "continuous" from where it just left off
			m_body1->m_velocityWs -= m_body1->m_invMass * impulse;
			m_body1->m_angularVelocityDegrees -= RadiansToDegrees(m_body1->m_invInertia * CrossProduct(contact->m_r1, impulse));

			m_body2->m_velocityWs += m_body2->m_invMass * impulse;
			m_body2->m_angularVelocityDegrees += RadiansToDegrees(m_body2->m_invInertia * CrossProduct(contact->m_r2, impulse));
		}
	}
}


//-------------------------------------------------------------------------------------------------
void Arbiter2D::ApplyImpulse()
{
	for (uint32 contactIndex = 0; contactIndex < m_numContacts; ++contactIndex)
	{
		Contact2D* contact = m_contacts + contactIndex;

		// Find how angular velocities will affect our point's velocity
		// Always use left-hand perps to the radius, as positive angular velocity is counter clockwise
		Vector2 angularContribution1 = DegreesToRadians(m_body1->m_angularVelocityDegrees) * Vector2(-1.0f * contact->m_r1.y, contact->m_r1.x);
		Vector2 angularContribution2 = DegreesToRadians(m_body2->m_angularVelocityDegrees) * Vector2(-1.0f * contact->m_r2.y, contact->m_r2.x);

		// Relative velocity at contact from body 1's point of view
		Vector2 relativeVelocity = m_body2->m_velocityWs + angularContribution2 - m_body1->m_velocityWs - angularContribution1;

		// Compute normal impulse
		float speedAlongNormal = DotProduct(relativeVelocity, contact->m_normal);
		float normalImpulseMagnitude = contact->m_massNormal * (-1.0f * speedAlongNormal + contact->m_bias); // Impulse goes against the velocity to correct, add a little bias for oomph

		if (ACCUMULATE_IMPULSES)
		{
			// Clamp the accumulated impulse
			float oldAccNormalImpulse = contact->m_accNormalImpulse;
			contact->m_accNormalImpulse = Max(oldAccNormalImpulse + normalImpulseMagnitude, 0.f);
			normalImpulseMagnitude = contact->m_accNormalImpulse - oldAccNormalImpulse;
		}
		else
		{
			normalImpulseMagnitude = Max(normalImpulseMagnitude, 0.f);
		}

		// Apply normal impulse
		Vector2 normalImpulse = normalImpulseMagnitude * contact->m_normal;

		Vector3 pos = m_body1->m_transform->GetWorldPosition();
		Vector2 centerOfMass = m_body1->GetCenterOfMassWs();

		m_body1->m_velocityWs -= m_body1->m_invMass * normalImpulse;
		m_body1->m_angularVelocityDegrees -= RadiansToDegrees(m_body1->m_invInertia * CrossProduct(contact->m_r1, normalImpulse));

		m_body2->m_velocityWs += m_body2->m_invMass * normalImpulse;
		m_body2->m_angularVelocityDegrees += RadiansToDegrees(m_body2->m_invInertia * CrossProduct(contact->m_r2, normalImpulse));

		// Recalculate the relative velocity
		angularContribution1 = DegreesToRadians(m_body1->m_angularVelocityDegrees) * Vector2(-1.0f * contact->m_r1.y, contact->m_r1.x);
		angularContribution2 = DegreesToRadians(m_body2->m_angularVelocityDegrees) * Vector2(-1.0f * contact->m_r2.y, contact->m_r2.x);
		relativeVelocity = m_body2->m_velocityWs + angularContribution2 - m_body1->m_velocityWs - angularContribution1;

		// Compute tangent impulse
		Vector2 tangent = Vector2(contact->m_normal.y, -1.0f * contact->m_normal.x);
		float speedAlongTangent = DotProduct(relativeVelocity, tangent);
		float tangentImpulseMagnitude = contact->m_massTangent * (-speedAlongTangent); // Friction opposes movement

		if (ACCUMULATE_IMPULSES)
		{
			// Factor in friction coefficient
			float maxTangentImpulseMag = m_friction * contact->m_accNormalImpulse; // Always >= 0.f

			// Clamp friction
			float oldTangentImpulse = contact->m_accTangentImpulse;
			contact->m_accTangentImpulse = Clamp(oldTangentImpulse + tangentImpulseMagnitude, -maxTangentImpulseMag, maxTangentImpulseMag);
			tangentImpulseMagnitude = contact->m_accTangentImpulse - oldTangentImpulse;
		}
		else
		{
			// Factor in friction coefficient
			float maxTangentImpulseMag = m_friction * normalImpulseMagnitude; // Always >= 0.f
			tangentImpulseMagnitude = Clamp(tangentImpulseMagnitude, -maxTangentImpulseMag, maxTangentImpulseMag);
		}

		// Apply the tangent impulse
		Vector2 tangentImpulse = tangentImpulseMagnitude * tangent;
		
		m_body1->m_velocityWs -= m_body1->m_invMass * tangentImpulse;
		m_body1->m_angularVelocityDegrees -= RadiansToDegrees(m_body1->m_invInertia * CrossProduct(contact->m_r1, tangentImpulse));
		
		m_body2->m_velocityWs += m_body2->m_invMass * tangentImpulse;
		m_body2->m_angularVelocityDegrees += RadiansToDegrees(m_body2->m_invInertia * CrossProduct(contact->m_r2, tangentImpulse));
	}
	/*
	RigidBody2D* b1 = m_body1;
	RigidBody2D* b2 = m_body2;

	for (int i = 0; i < (int)m_numContacts; ++i)
	{
		Contact2D* c = m_contacts + i;
		c->m_r1 = c->m_position - b1->m_transform->position.xy;
		c->m_r2 = c->m_position - b2->m_transform->position.xy;

		// Relative velocity at contact
		Vector2 dv = b2->m_velocityWs + Cross(b2->m_angularVelocityDegrees, c->m_r2) - b1->m_velocityWs - Cross(b1->m_angularVelocityDegrees, c->m_r1);

		// Compute normal impulse
		float vn = DotProduct(dv, c->m_normal);

		float dPn = c->m_massNormal * (-vn + c->m_bias);

		if (false)
		{
			// Clamp the accumulated impulse
			//float Pn0 = c->Pn;
			//c->Pn = Max(Pn0 + dPn, 0.0f);
			//dPn = c->Pn - Pn0;
		}
		else
		{
			dPn = Max(dPn, 0.0f);
		}

		// Apply contact impulse
		Vector2 Pn = dPn * c->m_normal;

		b1->m_velocityWs -= b1->m_invMass * Pn;
		b1->m_angularVelocityDegrees -= b1->m_invInertia * Cross(c->m_r1, Pn);

		b2->m_velocityWs += b2->m_invMass * Pn;
		b2->m_angularVelocityDegrees += b2->m_invInertia * Cross(c->m_r2, Pn);

		// Relative velocity at contact
		dv = b2->m_velocityWs + Cross(b2->m_angularVelocityDegrees, c->m_r2) - b1->m_velocityWs - Cross(b1->m_angularVelocityDegrees, c->m_r1);

		Vector2 tangent = Cross(c->m_normal, 1.0f);
		float vt = DotProduct(dv, tangent);
		float dPt = c->m_massTangent * (-vt);

		if (false)
		{
			// Compute friction impulse
			//float maxPt = friction * c->Pn;
			//
			//// Clamp friction
			//float oldTangentImpulse = c->Pt;
			//c->Pt = Clamp(oldTangentImpulse + dPt, -maxPt, maxPt);
			//dPt = c->Pt - oldTangentImpulse;
		}
		else
		{
			float maxPt = m_friction * dPn;
			dPt = Clamp(dPt, -maxPt, maxPt);
		}

		// Apply contact impulse
		Vector2 Pt = dPt * tangent;

		b1->m_velocityWs -= b1->m_invMass * Pt;
		b1->m_angularVelocityDegrees -= RadiansToDegrees(b1->m_invInertia * Cross(c->m_r1, Pt));

		b2->m_velocityWs += b2->m_invMass * Pt;
		b2->m_angularVelocityDegrees += RadiansToDegrees(b2->m_invInertia * Cross(c->m_r2, Pt));
	}*/
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
	const Polygon2D* incidentPoly = nullptr;

	if (Abs(dot1) > Abs(dot2))
	{
		// poly1 is our reference
		referenceEdge = &edge1;
		incidentEdge = &edge2;
		incidentPoly = poly2;
	}
	else
	{
		// poly2 is our reference
		referenceEdge = &edge2;
		incidentEdge = &edge1;
		incidentPoly = poly1;
	}

	Vector2 refEdgeDirection = referenceEdge->m_vertex2 - referenceEdge->m_vertex1;
	refEdgeDirection.Normalize();
	
	// Keep all our results from each clip for debugging purposes
	std::vector<ClipVertex2D> clippedPoints1;

	// Clip the incident edge to the start of the reference edge
	// First determine the min value the dot would need to be in order to be inside the clipping edge

	// Also set up our initial vertices to be clipped

	// Incident edge start vertex
	ClipVertex2D initialStartVertex;
	initialStartVertex.m_position = incidentEdge->m_vertex1;

	int prevVertexIndex = incidentPoly->GetPreviousValidIndex(incidentEdge->m_edgeId);
	ASSERT_OR_DIE(prevVertexIndex < 256, "We can't support Polygons with more than 256 side here :(");

	int prevEdgeId = (prevVertexIndex != 0 ? prevVertexIndex : incidentPoly->GetNumVertices()); // Edges are labeled by the index of the start vertex + 1 (in other words, by the index of their end vertex, with the last edge not using 0)

	initialStartVertex.m_id.m_incidentEdgeIn = (int8)prevEdgeId;
	initialStartVertex.m_id.m_incidentEdgeOut = (int8)incidentEdge->m_edgeId;

	// Incident edge end vertex
	ClipVertex2D initialEndVertex;
	initialEndVertex.m_position = incidentEdge->m_vertex2;
	initialEndVertex.m_id.m_incidentEdgeIn = (int8)incidentEdge->m_edgeId;
	
	int nextVertexIndex = incidentPoly->GetNextValidIndex(incidentEdge->m_edgeId);
	ASSERT_OR_DIE(nextVertexIndex < 256, "We can't support Polygons with more than 256 side here :(");

	int nextEdgeId = (nextVertexIndex != 0 ? nextVertexIndex : incidentPoly->GetNumVertices());// Edges are labeled by the index of the start vertex + 1 (in other words, by the index of their end vertex, with the last edge not using 0)
	initialEndVertex.m_id.m_incidentEdgeOut = (int8)nextEdgeId;

	float startDot = DotProduct(refEdgeDirection, referenceEdge->m_vertex1);
	ClipIncidentEdgeToReferenceEdge(initialStartVertex, initialEndVertex, refEdgeDirection, startDot, clippedPoints1);
	
	if (clippedPoints1.size() < 2)
	{
		return;
	}

	// Now clip the incident edge to the end of the reference edge
	// So clip in the opposite direction, flip some signs
	float endDot = DotProduct(refEdgeDirection, referenceEdge->m_vertex2);

	std::vector<ClipVertex2D> clippedPoints2;
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
	float penDepth1 = DotProduct(refNormalForClipping, clippedPoints2[0].m_position) - maxDepth;
	float penDepth2 = DotProduct(refNormalForClipping, clippedPoints2[1].m_position) - maxDepth;

	if (penDepth1 < 0.f)
	{
		m_contacts[m_numContacts].m_position = clippedPoints2[0].m_position;
		m_contacts[m_numContacts].m_normal = (poly1 == incidentPoly ? -1.0f * refNormalForClipping : refNormalForClipping);
		m_contacts[m_numContacts].m_r1 = clippedPoints2[0].m_position - m_body1->GetCenterOfMassWs();
		m_contacts[m_numContacts].m_r2 = clippedPoints2[0].m_position - m_body2->GetCenterOfMassWs();
		m_contacts[m_numContacts].m_separation = penDepth1;
		m_contacts[m_numContacts].m_referenceEdge = *referenceEdge;
		m_contacts[m_numContacts].m_incidentEdge = *incidentEdge;
		m_contacts[m_numContacts].m_id = clippedPoints2[0].m_id;
		m_numContacts++;
	}

	if (penDepth2 < 0.f)
	{
		m_contacts[m_numContacts].m_position = clippedPoints2[1].m_position;
		m_contacts[m_numContacts].m_normal = (poly1 == incidentPoly ? -1.0f * refNormalForClipping : refNormalForClipping);
		m_contacts[m_numContacts].m_r1 = clippedPoints2[1].m_position - m_body1->GetCenterOfMassWs();
		m_contacts[m_numContacts].m_r2 = clippedPoints2[1].m_position - m_body2->GetCenterOfMassWs();
		m_contacts[m_numContacts].m_separation = penDepth2;
		m_contacts[m_numContacts].m_referenceEdge = *referenceEdge;
		m_contacts[m_numContacts].m_incidentEdge = *incidentEdge;
		m_contacts[m_numContacts].m_id = clippedPoints2[1].m_id;
		m_numContacts++;
	}

	ASSERT_OR_DIE(m_numContacts <= 2, "Bad number of contacts!");
}
