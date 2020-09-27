///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: September 25th, 2020
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
#define NO_EDGE 0

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
Vector2 GetMinkowskiDiffSupport2D(const Polygon2D* first, const Polygon2D* second, const Vector2& direction)
{
	Vector2 firstVertex, secondVertex;
	first->GetFarthestVertexInDirection(direction, firstVertex);
	second->GetFarthestVertexInDirection(-1.0f * direction, secondVertex);

	return firstVertex - secondVertex;
}


//-------------------------------------------------------------------------------------------------
void SetupSimplex2D(const Polygon2D* first, const Polygon2D* second, std::vector <Vector2>& simplex)
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
EvolveSimplexResult EvolveSimplex2D(const Polygon2D* first, const Polygon2D* second, std::vector<Vector2>& evolvingSimplex)
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
uint32 GetSimplexSeparation2D(const std::vector<Vector2>& simplex, CollisionSeparation2D& out_separation)
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
CollisionSeparation2D PerformEPA(const Polygon2D* first, const Polygon2D* second, std::vector<Vector2>& simplex)
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
			simplex.insert(simplex.begin() + simplexIndex + 1U, expandedMinkowskiPoint);
		}
	}

	ERROR_RECOVERABLE("Couldn't find the Minkowski edge?");
	return CollisionSeparation2D(false);
}


//-------------------------------------------------------------------------------------------------
CollisionSeparation2D CalculateSeparation2D(const Polygon2D* first, const Polygon2D* second)
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
CollisionFeatureEdge2D GetFeatureEdge2D(const Polygon2D* polygon, const Vector2& outwardSeparationNormal)
{
	// Get the vertex farthest along the separation normal
	Vector2 vertex;
	int vertexIndex = polygon->GetFarthestVertexInDirection(outwardSeparationNormal, vertex);

	// This vertex is part of the feature edge - but it could be paired with the previous or next vertex (Clockwise winding order)
	Vector2 prevVertex, nextVertex;

	int prevVertexIndex = polygon->GetPreviousVertexToIndex(vertexIndex, prevVertex);
	int nextVertexIndex = polygon->GetNextVertexToIndex(vertexIndex, nextVertex);

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
		featureEdge.m_edgeId = nextVertexIndex; // Edges are ID'd by the index of their end vertex, 0 reserved for invalid
	}
	else
	{
		featureEdge.m_vertex1 = prevVertex;
		featureEdge.m_vertex2 = vertex;
		featureEdge.m_normal = prevNormal;
		featureEdge.m_edgeId = vertexIndex; // Edges are ID'd by the index of their end vertex, 0 reserved for invalid
	}

	return featureEdge;
}


//-------------------------------------------------------------------------------------------------
void ClipIncidentEdgeToReferenceEdge(const ClipVertex& incident1, const ClipVertex& incident2, const Vector2& refEdgeDirection, float offset, std::vector<ClipVertex>& out_clippedPoints)
{
	Vector2 incident1Pos = incident1.m_position;
	Vector2 incident2Pos = incident2.m_position;

	float distance1 = DotProduct(incident1Pos, refEdgeDirection) - offset;
	float distance2 = DotProduct(incident2Pos, refEdgeDirection) - offset;

	if (distance1 >= 0)
	{
		out_clippedPoints.push_back(incident1);
	}

	if (distance2 >= 0)
	{
		out_clippedPoints.push_back(incident2);
	}

	// Special case
	// If the two incident points are on opposite sides of the imaginary clipping line, create a new point at the clipping line
	if (distance1 * distance2 < 0)
	{
		Vector2 incidentEdge = incident2Pos - incident1Pos; // Don't normalize! We'll take a fractional portion of this
		float t = distance1 / (distance1 - distance2);

		Vector2 incidentEdgeOffset = incidentEdge * t;
		Vector2 finalPos = incident1Pos + incidentEdgeOffset;

		ClipVertex clipVertex;
		clipVertex.m_position = finalPos;

		// Update the ID of the two edges this vertex is "between"
		if (distance2 < 0)
		{
			// We clipped the edge end point
			clipVertex.m_id = incident2.m_id;
			clipVertex.m_id.m_maxRefEdgeClipped = 1;
		}
		else
		{
			// We clipped the edge start point
			clipVertex.m_id = incident1.m_id;
			clipVertex.m_id.m_minRefEdgeClipped = 1;
		}

		out_clippedPoints.push_back(clipVertex);
	}
}

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
