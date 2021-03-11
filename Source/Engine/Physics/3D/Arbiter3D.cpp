///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 25th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/Polygon2D.h"
#include "Engine/Physics/3D/Arbiter3D.h"
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
const float Arbiter3D::ALLOWED_PENETRATION = 0.01f;
const float Arbiter3D::BIAS_FACTOR = 0.2f; // Always do position correction for now?
const bool Arbiter3D::ACCUMULATE_IMPULSES = true; // Always for now?
const bool Arbiter3D::WARM_START_ACCUMULATIONS = true;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
Arbiter3D::Arbiter3D(RigidBody3D* body1, RigidBody3D* body2)
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
void Arbiter3D::Update(const Contact3D* newContacts, uint32 numNewContacts)
{
	UNIMPLEMENTED();
	//// Where the final result will be stored temporarily
	//Contact3D mergedContacts[2];

	//for (uint32 newContactIndex = 0U; newContactIndex < numNewContacts; ++newContactIndex)
	//{
	//	const Contact3D* newContact = newContacts + newContactIndex;

	//	// Search to see if we already have info for this contact stored from last frames result
	//	Contact3D* matchingOldContact = nullptr;
	//	for (int oldContactIndex = 0; oldContactIndex < m_numContacts; ++oldContactIndex)
	//	{
	//		Contact3D* oldContact = m_contacts + oldContactIndex;

	//		if (newContact->m_id.m_value == oldContact->m_id.m_value)
	//		{
	//			matchingOldContact = oldContact;
	//			break;
	//		}
	//	}

	//	if (matchingOldContact != nullptr)
	//	{
	//		// Overwrite our old contact info with the new stuff
	//		Contact3D* mergeContact = mergedContacts + newContactIndex;
	//		*mergeContact = *newContact;

	//		// If we want to reuse the last accumulation state of the contact, copy it now
	//		if (WARM_START_ACCUMULATIONS)
	//		{
	//			mergeContact->m_accNormalImpulse = matchingOldContact->m_accNormalImpulse;
	//			mergeContact->m_accTangentImpulse = matchingOldContact->m_accTangentImpulse;
	//			mergeContact->m_normalBiasImpulse = matchingOldContact->m_normalBiasImpulse;
	//		}
	//		else
	//		{
	//			mergeContact->m_accNormalImpulse = 0.f;
	//			mergeContact->m_accTangentImpulse = 0.f;
	//			mergeContact->m_normalBiasImpulse = 0.f;
	//		}
	//	}
	//	else
	//	{
	//		// Brand new contact, just add it to merged contacts
	//		mergedContacts[newContactIndex] = newContacts[newContactIndex];
	//	}

	//}

	//// Done merging, update the arbiter's contact data
	//for (uint32 newContactIndex = 0U; newContactIndex < numNewContacts; ++newContactIndex)
	//{
	//	m_contacts[newContactIndex] = mergedContacts[newContactIndex];
	//}

	//m_numContacts = numNewContacts;
}


//-------------------------------------------------------------------------------------------------
void Arbiter3D::DetectCollision()
{
	Polygon3D poly1, poly2;
	m_body1->GetWorldShape(poly1);
	m_body2->GetWorldShape(poly2);

	// Detect collision
	m_separation = CalculateSeparation3D(&poly1, &poly2);
	m_numContacts = 0;

	// TODO Remove this
	if (m_separation.m_collisionFound)
	{
		// Find the contact points of the collision
		// http://www.dyn4j.org/2011/11/contact-points-using-clipping/ for reference
		CalculateContactPoints(&poly1, &poly2, m_separation);
	}
}


//-------------------------------------------------------------------------------------------------
void Arbiter3D::PreStep(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	UNIMPLEMENTED();

	//float invDeltaSeconds = (deltaSeconds > 0.f ? 1.0f / deltaSeconds : 0.f);
	//
	//for (uint32 contactIndex = 0; contactIndex < m_numContacts; ++contactIndex)
	//{
	//	Contact3D* contact = m_contacts + contactIndex;

	//	// Precompute normal mass, tangent mass, and bias
	//	// Mass normal is used to calculate impulse necessary to prevent penetration
	//	float r1Normal = DotProduct(contact->m_r1, contact->m_normal);
	//	float r2Normal = DotProduct(contact->m_r2, contact->m_normal);

	//	float kNormal = m_body1->m_invMass + m_body2->m_invMass;
	//	kNormal += m_body1->m_invInertia * (DotProduct(contact->m_r1, contact->m_r1) - r1Normal * r1Normal) + m_body2->m_invInertia * (DotProduct(contact->m_r2, contact->m_r2) - r2Normal * r2Normal);
	//	contact->m_massNormal = 1.0f / kNormal;

	//	// Right-handed perp for tangent
	//	// Mass tangent is used to calculate impulse to simulate friction
	//	Vector2 tangent = Vector2(contact->m_normal.y, -1.0f * contact->m_normal.x);
	//	float r1Tangent = DotProduct(contact->m_r1, tangent);
	//	float r2Tangent = DotProduct(contact->m_r2, tangent);

	//	float kTangent = m_body1->m_invMass + m_body2->m_invMass;
	//	kTangent += m_body1->m_invInertia * (DotProduct(contact->m_r1, contact->m_r1) - r1Tangent * r1Tangent) + m_body2->m_invInertia * (DotProduct(contact->m_r2, contact->m_r2) - r2Tangent * r2Tangent);
	//	contact->m_massTangent = 1.0f / kTangent;

	//	// To quote Erin Catto, this gives the normal impulse "some extra oomph"
	//	// Proportional to the penetration, so if two objects are really intersecting -> greater bias -> greater normal force -> larger correction this frame
	//	// Allowed penetration means this will correct over time, not instantaneously - make it less jittery?
	//	contact->m_bias = -BIAS_FACTOR * invDeltaSeconds * Min(contact->m_separation + ALLOWED_PENETRATION, 0.f); // separation is *always* negative, it's distance below the reference edge

	//	// Apply old accumulated impulses at the beginning of the step
	//	// This leads to less iterations and greater stability
	//	// This is considered "warm starting"
	//	if (ACCUMULATE_IMPULSES)
	//	{
	//		Vector2 impulse = contact->m_accNormalImpulse * contact->m_normal + contact->m_accTangentImpulse * tangent;

	//		// Q. But friction impulse should be in some way related to relative velocity! Yet there's no velocity here!
	//		// A. These incrementals were calculated last frame in ApplyImpulse(), so unless velocity instantaneous and largely changed outside
	//		//    the physics system this warm start should feel "continuous" from where it just left off
	//		m_body1->m_velocityWs -= m_body1->m_invMass * impulse;
	//		m_body1->m_angularVelocityDegrees -= RadiansToDegrees(m_body1->m_invInertia * CrossProduct(contact->m_r1, impulse));

	//		m_body2->m_velocityWs += m_body2->m_invMass * impulse;
	//		m_body2->m_angularVelocityDegrees += RadiansToDegrees(m_body2->m_invInertia * CrossProduct(contact->m_r2, impulse));
	//	}
	//}
}


//-------------------------------------------------------------------------------------------------
void Arbiter3D::ApplyImpulse()
{
	UNIMPLEMENTED();

	//for (uint32 contactIndex = 0; contactIndex < m_numContacts; ++contactIndex)
	//{
	//	Contact3D* contact = m_contacts + contactIndex;

	//	// Find how angular velocities will affect our point's velocity
	//	// Always use left-hand perps to the radius, as positive angular velocity is counter clockwise
	//	Vector2 angularContribution1 = DegreesToRadians(m_body1->m_angularVelocityDegrees) * Vector2(-1.0f * contact->m_r1.y, contact->m_r1.x);
	//	Vector2 angularContribution2 = DegreesToRadians(m_body2->m_angularVelocityDegrees) * Vector2(-1.0f * contact->m_r2.y, contact->m_r2.x);

	//	// Relative velocity at contact from body 1's point of view
	//	Vector2 relativeVelocity = m_body2->m_velocityWs + angularContribution2 - m_body1->m_velocityWs - angularContribution1;

	//	// Compute normal impulse
	//	float speedAlongNormal = DotProduct(relativeVelocity, contact->m_normal);
	//	float normalImpulseMagnitude = contact->m_massNormal * (-1.0f * speedAlongNormal + contact->m_bias); // Impulse goes against the velocity to correct, add a little bias for oomph

	//	if (ACCUMULATE_IMPULSES)
	//	{
	//		// Clamp the accumulated impulse
	//		float oldAccNormalImpulse = contact->m_accNormalImpulse;
	//		contact->m_accNormalImpulse = Max(oldAccNormalImpulse + normalImpulseMagnitude, 0.f);
	//		normalImpulseMagnitude = contact->m_accNormalImpulse - oldAccNormalImpulse;
	//	}
	//	else
	//	{
	//		normalImpulseMagnitude = Max(normalImpulseMagnitude, 0.f);
	//	}

	//	// Apply normal impulse
	//	Vector2 normalImpulse = normalImpulseMagnitude * contact->m_normal;

	//	Vector3 pos = m_body1->m_transform->GetWorldPosition();
	//	Vector2 centerOfMass = m_body1->GetCenterOfMassWs();

	//	m_body1->m_velocityWs -= m_body1->m_invMass * normalImpulse;
	//	m_body1->m_angularVelocityDegrees -= RadiansToDegrees(m_body1->m_invInertia * CrossProduct(contact->m_r1, normalImpulse));

	//	m_body2->m_velocityWs += m_body2->m_invMass * normalImpulse;
	//	m_body2->m_angularVelocityDegrees += RadiansToDegrees(m_body2->m_invInertia * CrossProduct(contact->m_r2, normalImpulse));

	//	// Recalculate the relative velocity
	//	angularContribution1 = DegreesToRadians(m_body1->m_angularVelocityDegrees) * Vector2(-1.0f * contact->m_r1.y, contact->m_r1.x);
	//	angularContribution2 = DegreesToRadians(m_body2->m_angularVelocityDegrees) * Vector2(-1.0f * contact->m_r2.y, contact->m_r2.x);
	//	relativeVelocity = m_body2->m_velocityWs + angularContribution2 - m_body1->m_velocityWs - angularContribution1;

	//	// Compute tangent impulse
	//	Vector2 tangent = Vector2(contact->m_normal.y, -1.0f * contact->m_normal.x);
	//	float speedAlongTangent = DotProduct(relativeVelocity, tangent);
	//	float tangentImpulseMagnitude = contact->m_massTangent * (-speedAlongTangent); // Friction opposes movement

	//	if (ACCUMULATE_IMPULSES)
	//	{
	//		// Factor in friction coefficient
	//		float maxTangentImpulseMag = m_friction * contact->m_accNormalImpulse; // Always >= 0.f

	//		// Clamp friction
	//		float oldTangentImpulse = contact->m_accTangentImpulse;
	//		contact->m_accTangentImpulse = Clamp(oldTangentImpulse + tangentImpulseMagnitude, -maxTangentImpulseMag, maxTangentImpulseMag);
	//		tangentImpulseMagnitude = contact->m_accTangentImpulse - oldTangentImpulse;
	//	}
	//	else
	//	{
	//		// Factor in friction coefficient
	//		float maxTangentImpulseMag = m_friction * normalImpulseMagnitude; // Always >= 0.f
	//		tangentImpulseMagnitude = Clamp(tangentImpulseMagnitude, -maxTangentImpulseMag, maxTangentImpulseMag);
	//	}

	//	// Apply the tangent impulse
	//	Vector2 tangentImpulse = tangentImpulseMagnitude * tangent;
	//	
	//	m_body1->m_velocityWs -= m_body1->m_invMass * tangentImpulse;
	//	m_body1->m_angularVelocityDegrees -= RadiansToDegrees(m_body1->m_invInertia * CrossProduct(contact->m_r1, tangentImpulse));
	//	
	//	m_body2->m_velocityWs += m_body2->m_invMass * tangentImpulse;
	//	m_body2->m_angularVelocityDegrees += RadiansToDegrees(m_body2->m_invInertia * CrossProduct(contact->m_r2, tangentImpulse));
	//}
}


//-------------------------------------------------------------------------------------------------
void Arbiter3D::CalculateContactPoints(const Polygon3D* poly1, const Polygon3D* poly2, const CollisionSeparation3d& separation)
{
	// Find the best edges for each polygon (normal is from A)
	CollisionFace3d face1 = GetFeatureFace3D(poly1, separation.m_dirFromFirst);
	CollisionFace3d face2 = GetFeatureFace3D(poly2, -1.0f * separation.m_dirFromFirst);
	
	// Determine which is the reference edge and which is the incident edge
	// Reference edge is the one whose normal is more parallel to the separation direction
	float dot1 = DotProduct(face1.m_normal, separation.m_dirFromFirst);
	float dot2 = DotProduct(face2.m_normal, separation.m_dirFromFirst);

	const CollisionFace3d* referenceFace = nullptr;
	const CollisionFace3d* incidentFace = nullptr;
	const Polygon3D* incidentPoly = nullptr;
	const Polygon3D* referencePoly = nullptr;

	if (Abs(dot1) > Abs(dot2))
	{
		// poly1 is our reference
		referenceFace = &face1;
		incidentFace = &face2;
		incidentPoly = poly2;
		referencePoly = poly1;
	}
	else
	{
		// poly2 is our reference
		referenceFace = &face2;
		incidentFace = &face1;
		incidentPoly = poly1;
		referencePoly = poly2;
	}

	// 3D point clipping
	// We clip all the points in the incident face by the planes created by
	// all adjacent faces to the reference face in the reference polygon
	std::vector<Face3> adjacentFaces;
	referencePoly->GetAllFacesAdjacentTo(referenceFace->m_faceIndex, adjacentFaces);
	int numAdjacentFaces = (int)adjacentFaces.size();

	std::vector<Vector3> incidentVerts = incidentFace->m_face.GetVertices();
	std::vector<ClipVertex3> clipPoints;

	for (int incidentVertexIndex = 0; incidentVertexIndex < (int)incidentVerts.size(); ++incidentVertexIndex)
	{
		ClipVertex3 newClip;
		newClip.m_position = incidentVerts[incidentVertexIndex];
		newClip.m_id = ClipVertexId((void*)incidentPoly, incidentFace->m_faceIndex, incidentVertexIndex);

		clipPoints.push_back(newClip);
	}
	
	// For each incident point...
	for (int clipIndex = 0; clipIndex < (int)clipPoints.size(); ++clipIndex)
	{
		ClipVertex3& point = clipPoints[clipIndex];

		// For each face adjacent to the reference face...
		for (int adjFaceIndex = 0; adjFaceIndex < numAdjacentFaces; ++adjFaceIndex)
		{
			Plane plane = adjacentFaces[adjFaceIndex].GetSupportPlane();

			// If the point is outside this plane, "snap" it to the plane
			if (plane.IsPointInFront(point.m_position))
			{
				point.m_position = plane.GetProjectedPointOntoPlane(point.m_position);
			}
		}
	}

	// Final clip - if the point is outside the reference face, remove it outright
	std::vector<float> penDepths;
	float maxDepth = DotProduct(referenceFace->m_normal, referenceFace->m_furthestVertex);

	for (int clipIndex = 0; clipIndex < (int)clipPoints.size(); ++clipIndex)
	{
		// If any of these points are "deeper" than the max depth then they are in the collision manifold
		float penDepth = DotProduct(referenceFace->m_normal, clipPoints[clipIndex].m_position) - maxDepth;
		penDepths.push_back(penDepth);
	}

	for (int clipIndex = 0; clipIndex < (int)clipPoints.size(); ++clipIndex)
	{
		ASSERT_OR_DIE(m_numContacts < MAX_CONTACTS, "Ran out of room for contacts!");

		if (penDepths[clipIndex] < DEFAULT_EPSILON)
		{
			m_contacts[m_numContacts].m_position = clipPoints[clipIndex].m_position;
			m_contacts[m_numContacts].m_normal = (poly1 == incidentPoly ? -1.0f * referenceFace->m_normal : referenceFace->m_normal);
			//m_contacts[m_numContacts].m_r1 = clipPoints[clipIndex].m_position - m_body1->GetCenterOfMassWs();
			//m_contacts[m_numContacts].m_r2 = clipPoints[clipIndex].m_position - m_body2->GetCenterOfMassWs();
			m_contacts[m_numContacts].m_separation = penDepths[clipIndex];
			m_contacts[m_numContacts].m_referenceFace = *referenceFace;
			m_contacts[m_numContacts].m_incidentFace = *incidentFace;
			m_contacts[m_numContacts].m_id = clipPoints[clipIndex].m_id;
			m_numContacts++;
		}
	}
}
