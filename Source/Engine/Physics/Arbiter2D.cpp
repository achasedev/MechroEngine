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
#define NUM_EPA_ITERATIONS 16U

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

enum EvolveSimplexResult
{
	NO_INTERSECTION,
	INTERSECTION_FOUND,
	SIMPLEX_STILL_EVOLVING
};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
static Vector2 GetMinkowskiDiffSupport2D(const Polygon2D* first, const Polygon2D* second, const Vector2& direction)
{
	Vector2 firstVertex, secondVertex;
	first->GetFarthestVertexInDirection(direction, firstVertex);
	second->GetFarthestVertexInDirection(-1.0f * direction, secondVertex);

	return firstVertex - secondVertex;
}


//-------------------------------------------------------------------------------------------------
static void SetupSimplex2D(const Polygon2D* first, const Polygon2D* second, std::vector <Vector2>& simplex)
{
	// A vertex
	Vector2 direction = second->GetCenter() - first->GetCenter();
	simplex.push_back(GetMinkowskiDiffSupport2D(first, second, direction));

	// B vertex
	direction = -1.0f * direction;
	simplex.push_back(GetMinkowskiDiffSupport2D(first, second, direction));

	// C vertex
	Vector2 a = simplex[0];
	Vector2 b = simplex[1];
	Vector2 ab = b - a;
	Vector2 abPerp = Vector2(ab.y, -ab.x);

	// Make sure the new direction is pointing towards the origin
	Vector2 aToOrigin = -1.0f * a;

	if (DotProduct(abPerp, aToOrigin) < 0.f)
	{
		abPerp *= -1.0f;

		// Force the simplex to maintain a clockwise winding order
		// This means the "right" side perp will always point inside the polygon
		Vector2 temp = a;
		simplex[0] = b;
		simplex[1] = temp;
	}

	simplex.push_back(GetMinkowskiDiffSupport2D(first, second, abPerp));
}


//-------------------------------------------------------------------------------------------------
static EvolveSimplexResult EvolveSimplex2D(const Polygon2D* first, const Polygon2D* second, std::vector<Vector2>& evolvingSimplex)
{
	ASSERT_OR_DIE(evolvingSimplex.size() == 3, "Wrong number of verts for simplex!");

	Vector2 a = evolvingSimplex[0];
	Vector2 b = evolvingSimplex[1];
	Vector2 c = evolvingSimplex[2];

	Vector2 cToOrigin = -1.0f * c;
	Vector2 bc = c - b;
	Vector2 ca = a - c;

	// "Right" hand perps so they point inward on our clockwise-winding simplex
	Vector2 bcPerp = Vector2(bc.y, -bc.x);
	Vector2 caPerp = Vector2(ca.y, -ca.x);

	if (DotProduct(bcPerp, cToOrigin) < 0) // Check if origin is outside bc side
	{
		// Keep b and c, look on the other side of bc for a new point
		// Maintain clockwise winding order
		evolvingSimplex.clear();
		evolvingSimplex.push_back(c);
		evolvingSimplex.push_back(b);
		evolvingSimplex.push_back(GetMinkowskiDiffSupport2D(first, second, -1.0f * bcPerp));

		// Check if we even went past the origin
		bool wentPastOrigin = (DotProduct(-1.0f * bcPerp, evolvingSimplex[2]) >= 0);
		return (wentPastOrigin ? SIMPLEX_STILL_EVOLVING : NO_INTERSECTION);
	}
	else if (DotProduct(caPerp, cToOrigin) < 0) // Check if origin is outside ca side
	{
		// Keep a and c, look on the other side of ca for a new point
		// Maintain clockwise winding order
		evolvingSimplex.clear();
		evolvingSimplex.push_back(a);
		evolvingSimplex.push_back(c);
		evolvingSimplex.push_back(GetMinkowskiDiffSupport2D(first, second, -1.0f * caPerp));

		// Check if we even went past the origin
		bool wentPastOrigin = (DotProduct(-1.0f * caPerp, evolvingSimplex[2]) >= 0);
		return (wentPastOrigin ? SIMPLEX_STILL_EVOLVING : NO_INTERSECTION);
	}

	// Otherwise both sides contain the origin, so intersection!
	return INTERSECTION_FOUND;
}


//-------------------------------------------------------------------------------------------------
// Calculates the minimum distance from any edge on the simplex to the origin
static uint32 GetSimplexSeparation2D(const std::vector<Vector2>& simplex, CollisionSeparation2D& out_separation)
{
	uint32 numVertices = (uint32)simplex.size();
	uint32 closestIndex = 0;
	for (uint32 i = 0; i < numVertices; ++i)
	{
		uint32 j = ((i == numVertices - 1) ? 0 : i + 1);
		Vector2 edge = simplex[j] - simplex[i];

		// Get the outward facing normal
		Vector2 edgeNormal = Vector2(-edge.y, edge.x);
		edgeNormal.Normalize();

		float distanceToOrigin = DotProduct(simplex[i], edgeNormal);
		ASSERT_OR_DIE(distanceToOrigin >= 0, "How is this distance negative?");

		if (distanceToOrigin < out_separation.m_separation)
		{
			out_separation.m_separation = distanceToOrigin;
			out_separation.m_dirFromFirst = edgeNormal;
			closestIndex = i;
		}
	}

	return closestIndex;
}


//-------------------------------------------------------------------------------------------------
static CollisionSeparation2D PerformEPA(const Polygon2D* first, const Polygon2D* second, std::vector<Vector2>& simplex)
{
	for (uint32 iteration = 0; iteration < NUM_EPA_ITERATIONS; ++iteration)
	{
		CollisionSeparation2D simplexSeparation;
		simplexSeparation.m_collisionFound = true;
		uint32 simplexIndex = GetSimplexSeparation2D(simplex, simplexSeparation);

		Vector2 expandedMinkowskiPoint = GetMinkowskiDiffSupport2D(first, second, simplexSeparation.m_dirFromFirst);
		float distanceToMinkowskiEdge = DotProduct(simplexSeparation.m_dirFromFirst, expandedMinkowskiPoint);
		ASSERT_OR_DIE(distanceToMinkowskiEdge >= 0, "This should always be positive!");

		float diff = Abs(simplexSeparation.m_separation - distanceToMinkowskiEdge);
		if (diff < DEFAULT_EPSILON)
		{
			// Difference is close enough, this is the right edge
			return simplexSeparation;
		}
		else
		{
			// Our simplex edge is inside the Minkowski difference shape, not on the edge
			// Add this point to our simplex at the right posiion and try again
			simplex.insert(simplex.begin() + simplexIndex + 1, expandedMinkowskiPoint);
		}
	}

	ERROR_RECOVERABLE("Couldn't find the Minkowski edge?");
	return CollisionSeparation2D(false);
}


//-------------------------------------------------------------------------------------------------
static CollisionSeparation2D CalculateSeparation2D(const Polygon2D* first, const Polygon2D* second)
{
	EvolveSimplexResult result = SIMPLEX_STILL_EVOLVING;
	std::vector<Vector2> simplex;

	// Set up initial vertices
	SetupSimplex2D(first, second, simplex);

	while (result == SIMPLEX_STILL_EVOLVING)
	{
		result = EvolveSimplex2D(first, second, simplex);
	}

	if (result == INTERSECTION_FOUND)
	{
		// Use EPA (Expanding Polytope Algorithm) to find the edge we're overlapping with
		return PerformEPA(first, second, simplex);
	}

	// Must be no collision
	return CollisionSeparation2D(false);
}


//-------------------------------------------------------------------------------------------------
static CollisionFeatureEdge2D GetFeatureEdge2D(const Polygon2D* polygon, const Vector2& outwardSeparationNormal)
{
	// Get the vertex farthest along the separation normal
	Vector2 vertex;
	int vertexIndex = polygon->GetFarthestVertexInDirection(outwardSeparationNormal, vertex);

	// This vertex is part of the feature edge - but it could be paired with the previous or next vertex (Clockwise winding order)
	Vector2 prevVertex = polygon->GetPreviousVertexToIndex(vertexIndex);
	Vector2 nextVertex = polygon->GetNextVertexToIndex(vertexIndex);

	Vector2 prevEdge = (vertex - prevVertex);
	Vector2 nextEdge = (nextVertex - vertex);

	// "Left" perps always point out
	Vector2 prevNormal = Vector2(-prevEdge.y, prevEdge.x);
	Vector2 nextNormal = Vector2(-nextEdge.y, nextEdge.x);
	prevNormal.Normalize();
	nextNormal.Normalize();

	float prevDot = DotProduct(prevNormal, outwardSeparationNormal);
	float nextDot = DotProduct(nextNormal, outwardSeparationNormal);

	CollisionFeatureEdge2D featureEdge;
	featureEdge.m_furthestVertex = vertex;
	
	// Use an epsilon bias to avoid floating point errors in tie breaker cases
	if (nextDot - prevDot >= DEFAULT_EPSILON)
	{
		featureEdge.m_vertex1 = vertex;
		featureEdge.m_vertex2 = nextVertex;
		featureEdge.m_normal = nextNormal;
	}
	else
	{
		featureEdge.m_vertex1 = prevVertex;
		featureEdge.m_vertex2 = vertex;
		featureEdge.m_normal = prevNormal;
	}

	return featureEdge;
}


//-------------------------------------------------------------------------------------------------
static void ClipIncidentEdgeToReferenceEdge(const Vector2& incident1, const Vector2& incident2, const Vector2& refEdgeDirection, float offset, std::vector<Vector2>& clippedPoints)
{
	float distance1 = DotProduct(incident1, refEdgeDirection) - offset;
	float distance2 = DotProduct(incident2, refEdgeDirection) - offset;

	if (distance1 >= 0)
	{
		clippedPoints.push_back(incident1);
	}

	if (distance2 >= 0)
	{
		clippedPoints.push_back(incident2);
	}

	// Special case
	// If the two incident points are on opposite sides of the imaginary clipping line, create a new point at the clipping line
	if (distance1 * distance2 < 0)
	{
		Vector2 incidentEdge = incident2 - incident1; // Don't normalize! We'll take a fractional portion of this
		float t = distance1 / (distance1 - distance2);
		
		Vector2 incidentEdgeOffset = incidentEdge * t;
		Vector2 finalPos = incident1 + incidentEdgeOffset;

		clippedPoints.push_back(finalPos);
	}
}


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
		m_contacts[m_numContacts].m_r1 = clippedPoints2[0] - m_body1->GetCenterOfMass();
		m_contacts[m_numContacts].m_r2 = clippedPoints2[0] - m_body2->GetCenterOfMass();
		m_contacts[m_numContacts].m_separation = penDepth1;
		m_contacts[m_numContacts].m_referenceEdge = *referenceEdge;
		m_contacts[m_numContacts].m_incidentEdge = *incidentEdge;
		m_numContacts++;
	}

	if (penDepth2 < 0.f)
	{
		m_contacts[m_numContacts].m_position = clippedPoints2[1];
		m_contacts[m_numContacts].m_normal = refNormalForClipping;
		m_contacts[m_numContacts].m_r1 = clippedPoints2[1] - m_body1->GetCenterOfMass();
		m_contacts[m_numContacts].m_r2 = clippedPoints2[1] - m_body2->GetCenterOfMass();
		m_contacts[m_numContacts].m_separation = penDepth2;
		m_contacts[m_numContacts].m_referenceEdge = *referenceEdge;
		m_contacts[m_numContacts].m_incidentEdge = *incidentEdge;
		m_numContacts++;
	}

	ASSERT_OR_DIE(m_numContacts <= 2, "Bad number of contacts!");
}
