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

struct CollisionSeparation2D
{
	CollisionSeparation2D() {}
	CollisionSeparation2D(bool collisionFound)
		: m_collisionFound(collisionFound) {}

	bool	m_collisionFound = false;
	Vector2 m_dirFromA;
	float	m_separation;
};

struct CollisionFeatureEdge2D
{
	Vector2 m_furthestVertex;
	Vector2 m_vertex1;
	Vector2 m_vertex2;
	Vector2 m_normal;
};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
static Vector2 GetMinkowskiDiffSupport(const Polygon2D* first, const Polygon2D* second, const Vector2& direction)
{
	return (first->GetFarthestVertexInDirection(direction) - second->GetFarthestVertexInDirection(-1.0f * direction));
}


//-------------------------------------------------------------------------------------------------
static void SetupSimplex(const Polygon2D* first, const Polygon2D* second, std::vector <Vector2>& simplex)
{
	// A vertex
	Vector2 direction = second->GetCenter() - first->GetCenter();
	simplex.push_back(GetMinkowskiDiffSupport(first, second, direction));

	// B vertex
	direction = -1.0f * direction;
	simplex.push_back(GetMinkowskiDiffSupport(first, second, direction));

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

	simplex.push_back(GetMinkowskiDiffSupport(first, second, abPerp));
}


//-------------------------------------------------------------------------------------------------
static EvolveSimplexResult EvolveSimplex(const Polygon2D* first, const Polygon2D* second, std::vector<Vector2>& evolvingSimplex)
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
		evolvingSimplex.push_back(GetMinkowskiDiffSupport(first, second, -1.0f * bcPerp));

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
		evolvingSimplex.push_back(GetMinkowskiDiffSupport(first, second, -1.0f * caPerp));

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
	uint32 numVertices = simplex.size();
	uint32 closestIndex = 0;
	for (uint32 i = 0; i < numVertices; ++i)
	{
		uint32 j = ((i == numVertices - 1) ? 0 : i + 1);
		Vector2 edge = simplex[j] - simplex[i];

		// Use the right-hand normal - this makes it point from A to B
		Vector2 edgeNormal = Vector2(edge.y, -edge.x);
		edgeNormal.Normalize();

		float distanceToOrigin = DotProduct(simplex[i], edgeNormal);

		if (Abs(distanceToOrigin) < out_separation.m_separation)
		{
			out_separation.m_separation = distanceToOrigin;
			out_separation.m_dirFromA = edgeNormal;
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
		uint32 simplexIndex = GetSimplexSeparation2D(simplex, simplexSeparation);

		Vector2 expandedMinkowskiPoint = GetMinkowskiDiffSupport(first, second, simplexSeparation.m_dirFromA);
		float distanceToMinkowskiEdge = DotProduct(simplexSeparation.m_dirFromA, expandedMinkowskiPoint);

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
}


//-------------------------------------------------------------------------------------------------
static CollisionSeparation2D CalculateSeparation2D(const Polygon2D* first, const Polygon2D* second)
{
	EvolveSimplexResult result = SIMPLEX_STILL_EVOLVING;
	std::vector<Vector2> simplex;

	// Set up initial vertices
	SetupSimplex(first, second, simplex);

	while (result == SIMPLEX_STILL_EVOLVING)
	{
		result = EvolveSimplex(first, second, simplex);
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
	
	if (nextDot > prevDot)
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
	// Detect collision
	CollisionSeparation2D separation = CalculateSeparation2D(m_body1->GetShape(), m_body2->GetShape());

	if (separation.m_collisionFound)
	{
		// Find the contact points of the collision
		// http://www.dyn4j.org/2011/11/contact-points-using-clipping/ for reference
		CalculateContactPoints();
	}
	else
	{
		m_numContacts = 0;
	}
}


//-------------------------------------------------------------------------------------------------
void Arbiter2D::CalculateContactPoints(const Polygon2D* poly1, const Polygon2D* poly2, const CollisionSeparation2D& separation)
{
	// Find the best edges for each polygon (normal is from A)
	CollisionFeatureEdge2D edge1 = GetFeatureEdge2D(poly1, separation.m_dirFromA);
	CollisionFeatureEdge2D edge2 = GetFeatureEdge2D(poly2, -1.0f * separation.m_dirFromA);

	// Determine which is the reference edge and which is the incident edge

	// Clip the indident edge to the reference edge


}
