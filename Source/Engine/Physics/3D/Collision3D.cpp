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
#include "Engine/Math/Polygon3D.h"
#include "Engine/Physics/3D/Collision3D.h"
#include "Engine/Physics/3D/Arbiter3D.h"
#include "Engine/Physics/3D/RigidBody3D.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#define NUM_EPA_ITERATIONS 64U
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
Vector3 GetMinkowskiDiffSupport3D(const Polygon3D* first, const Polygon3D* second, const Vector3& direction)
{
	Vector3 firstVertex, secondVertex;
	first->GetFarthestVertexInDirection(direction, firstVertex);
	second->GetFarthestVertexInDirection(-1.0f * direction, secondVertex);

	return firstVertex - secondVertex;
}


//-------------------------------------------------------------------------------------------------
void SetupSimplex3D(const Polygon3D* first, const Polygon3D* second, std::vector <Vector3>& simplex)
{
	// A vertex
	Vector3 direction = second->GetCenter() - first->GetCenter();
	simplex.push_back(GetMinkowskiDiffSupport3D(first, second, direction));

	// B vertex
	direction = -1.0f * direction;
	simplex.push_back(GetMinkowskiDiffSupport3D(first, second, direction));

	// C vertex
	Vector3 a = simplex[0];
	Vector3 b = simplex[1];
	Vector3 ab = b - a;
	Vector3 aToOrigin = -1.0f * a;

	// Get a vector orthogonal to ab
	Vector3 hint = (!AreMostlyEqual(DotProduct(Vector3::Y_AXIS, ab), 0.f) ? Vector3::Y_AXIS : Vector3::X_AXIS);
	Vector3 abPerp = CrossProduct(ab, hint);
	abPerp.Normalize();

	// Sanity check
	float dot = DotProduct(abPerp, ab);
	ASSERT_OR_DIE(AreMostlyEqual(dot, 0.f), "Bad perp!");

	// Make sure the new direction is pointing towards the origin
	if (DotProduct(abPerp, aToOrigin) < 0.f)
	{
		abPerp *= -1.0f;
	}

	simplex.push_back(GetMinkowskiDiffSupport3D(first, second, abPerp));

	// D vertex
	Vector3 c = simplex[2];
	Vector3 inwardNormal = -1.0f * CalculateNormalForTriangle(a, b, c); // -1.0 to make it point inward

	// Make sure the new direction is pointing towards the origin
	if (DotProduct(inwardNormal, aToOrigin) < 0.f)
	{
		// Update the ordering so the cross points towards the origin
		Vector3 temp = a;
		simplex[0] = b;
		simplex[1] = temp;

		inwardNormal *= -1.0f;
	}

	simplex.push_back(GetMinkowskiDiffSupport3D(first, second, inwardNormal));
}


//-------------------------------------------------------------------------------------------------
EvolveSimplexResult EvolveSimplex3D(const Polygon3D* first, const Polygon3D* second, std::vector<Vector3>& evolvingSimplex)
{
	ASSERT_OR_DIE(evolvingSimplex.size() == 4, "Wrong number of verts for 3D simplex!");

	Vector3 a = evolvingSimplex[0];
	Vector3 b = evolvingSimplex[1];
	Vector3 c = evolvingSimplex[2];
	Vector3 d = evolvingSimplex[3];

	Vector3 bdcInwardNormal = -1.0f * CalculateNormalForTriangle(b, d, c);
	Vector3 cdaInwardNormal = -1.0f * CalculateNormalForTriangle(c, d, a);
	Vector3 adbInwardNormal = -1.0f * CalculateNormalForTriangle(a, d, b);

	Vector3 dToOrigin = -1.0f * d;

	if (DotProduct(dToOrigin, bdcInwardNormal) < 0.f) // Check if origin is outside bdc side
	{
		// Keep b, c, d, look on the other side of bdc for a new point
		// Maintain winding order s.t. "ab x ac" cross points outward
		evolvingSimplex.clear();
		evolvingSimplex.push_back(b);
		evolvingSimplex.push_back(c);
		evolvingSimplex.push_back(d);

		evolvingSimplex.push_back(GetMinkowskiDiffSupport3D(first, second, -1.0f * bdcInwardNormal));

		// Check if we even went past the origin
		bool wentPastOrigin = (DotProduct(-1.0f * bdcInwardNormal, evolvingSimplex[3]) >= 0);
		return (wentPastOrigin ? SIMPLEX_STILL_EVOLVING : NO_INTERSECTION);
	}
	else if (DotProduct(dToOrigin, cdaInwardNormal) < 0.f) // Check if origin is outside cda side
	{
		// Keep c, d, a, look on the other side of cda for a new point
		// Maintain winding order s.t. "ab x ac" cross points outward
		evolvingSimplex.clear();

		evolvingSimplex.push_back(a);
		evolvingSimplex.push_back(d);
		evolvingSimplex.push_back(c);

		evolvingSimplex.push_back(GetMinkowskiDiffSupport3D(first, second, -1.0f * cdaInwardNormal));

		// Check if we even went past the origin
		bool wentPastOrigin = (DotProduct(-1.0f * cdaInwardNormal, evolvingSimplex[3]) >= 0);
		return (wentPastOrigin ? SIMPLEX_STILL_EVOLVING : NO_INTERSECTION);
	}
	else if (DotProduct(dToOrigin, adbInwardNormal) < 0.f) // Check if origin is outside adb side
	{
		// Keep a, d, b, look on the other side of adb for a new point
		// Maintain winding order s.t. "ab x ac" cross points outward
		evolvingSimplex.clear();

		evolvingSimplex.push_back(a);
		evolvingSimplex.push_back(b);
		evolvingSimplex.push_back(d);

		evolvingSimplex.push_back(GetMinkowskiDiffSupport3D(first, second, -1.0f * adbInwardNormal));

		// Check if we even went past the origin
		bool wentPastOrigin = (DotProduct(-1.0f * adbInwardNormal, evolvingSimplex[3]) >= 0);
		return (wentPastOrigin ? SIMPLEX_STILL_EVOLVING : NO_INTERSECTION);
	}

	// Otherwise both sides contain the origin, so intersection!
	return INTERSECTION_FOUND;
}


//-------------------------------------------------------------------------------------------------
// Calculates the minimum distance from any face on the simplex to the origin
uint32 GetSimplexSeparation3D(const std::vector<Face3D>& simplex, CollisionSeparation3D& out_separation)
{
	uint32 numFaces = (uint32)simplex.size();
	uint32 closestIndex = 0;

	for (uint32 faceIndex = 0; faceIndex < numFaces; ++faceIndex)
	{
		const Face3D& face = simplex[faceIndex];

		// Get the outward facing normal
		Vector3 normal = face.GetNormal();
		Vector3 faceVertex = face.GetVertex(0);

		float distanceToOrigin = DotProduct(normal, faceVertex);
		ASSERT_OR_DIE((distanceToOrigin + DEFAULT_EPSILON) >= 0, "How is this distance negative?");

		if (distanceToOrigin < out_separation.m_separation)
		{
			out_separation.m_separation = distanceToOrigin;
			out_separation.m_dirFromFirst = normal;
			closestIndex = faceIndex;
		}
	}

	return closestIndex;
}


//-------------------------------------------------------------------------------------------------
CollisionSeparation3D PerformEPA3D(const Polygon3D* first, const Polygon3D* second, const std::vector<Vector3>& vertexSimplex)
{
	// Create a list of faces to work with instead of vertices
	// Ensure all normals point outward
	std::vector<Face3D> faceSimplex;
	faceSimplex.push_back(Face3D(vertexSimplex[0], vertexSimplex[1], vertexSimplex[2], vertexSimplex[0]));
	faceSimplex.push_back(Face3D(vertexSimplex[0], vertexSimplex[1], vertexSimplex[3], vertexSimplex[0]));
	faceSimplex.push_back(Face3D(vertexSimplex[0], vertexSimplex[2], vertexSimplex[3], vertexSimplex[0]));
	faceSimplex.push_back(Face3D(vertexSimplex[1], vertexSimplex[2], vertexSimplex[3], vertexSimplex[1]));

	for (uint32 iteration = 0; iteration < NUM_EPA_ITERATIONS; ++iteration)
	{
		CollisionSeparation3D simplexSeparation;
		simplexSeparation.m_collisionFound = true;
		uint32 closestFaceIndex = GetSimplexSeparation3D(faceSimplex, simplexSeparation);

		Vector3 expandedMinkowskiPoint = GetMinkowskiDiffSupport3D(first, second, simplexSeparation.m_dirFromFirst);
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
			// Our simplex face is inside the Minkowski difference shape, not on the edge
			// So remove the bad face and add 3 new faces simulating adding this point
			Face3D closestFace = faceSimplex[closestFaceIndex];
			faceSimplex.erase(faceSimplex.begin() + closestFaceIndex);

			faceSimplex.push_back(Face3D(closestFace.GetVertex(0), closestFace.GetVertex(1), expandedMinkowskiPoint, closestFace.GetVertex(0)));
			faceSimplex.push_back(Face3D(closestFace.GetVertex(0), closestFace.GetVertex(2), expandedMinkowskiPoint, closestFace.GetVertex(0)));
			faceSimplex.push_back(Face3D(closestFace.GetVertex(1), closestFace.GetVertex(2), expandedMinkowskiPoint, closestFace.GetVertex(1)));
		}
	}

	ERROR_RECOVERABLE("Couldn't find the Minkowski face?");
	return CollisionSeparation3D(false);
}


//-------------------------------------------------------------------------------------------------
CollisionSeparation3D CalculateSeparation3D(const Polygon3D* first, const Polygon3D* second)
{
	EvolveSimplexResult result = SIMPLEX_STILL_EVOLVING;
	std::vector<Vector3> simplex;

	// Set up initial vertices
	SetupSimplex3D(first, second, simplex);

	while (result == SIMPLEX_STILL_EVOLVING)
	{
		result = EvolveSimplex3D(first, second, simplex);
	}

	if (result == INTERSECTION_FOUND)
	{
		// Use EPA (Expanding Polytope Algorithm) to find the edge we're overlapping with
		return PerformEPA3D(first, second, simplex);
	}

	// Must be no collision
	return CollisionSeparation3D(false);
}


//-------------------------------------------------------------------------------------------------
CollisionFeatureFace3D GetFeatureFace3D(const Polygon3D* polygon, const Vector3& outwardSeparationNormal)
{
	UNUSED(polygon);
	UNUSED(outwardSeparationNormal);
	return CollisionFeatureFace3D();
	// TODO lol
}


////-------------------------------------------------------------------------------------------------
//CollisionFeatureEdge2D GetFeatureEdge2D(const Polygon2D* polygon, const Vector2& outwardSeparationNormal)
//{
//	// Get the vertex farthest along the separation normal
//	Vector2 vertex;
//	int vertexIndex = polygon->GetFarthestVertexInDirection(outwardSeparationNormal, vertex); // YO REMEMBER THIS IS JUST THE VERTEX, NOT THE INDEX
//
//	// This vertex is part of the feature edge - but it could be paired with the previous or next vertex (Clockwise winding order)
//	Vector2 prevVertex, nextVertex;
//
//	polygon->GetPreviousVertexToIndex(vertexIndex, prevVertex);
//	int nextVertexIndex = polygon->GetNextVertexToIndex(vertexIndex, nextVertex);
//
//	Vector2 prevEdge = (vertex - prevVertex);
//	Vector2 nextEdge = (nextVertex - vertex);
//
//	// "Left" perps always point out
//	Vector2 prevNormal = Vector2(-prevEdge.y, prevEdge.x);
//	Vector2 nextNormal = Vector2(-nextEdge.y, nextEdge.x);
//	prevNormal.Normalize();
//	nextNormal.Normalize();
//
//	float prevDot = DotProduct(prevNormal, outwardSeparationNormal);
//	float nextDot = DotProduct(nextNormal, outwardSeparationNormal);
//
//	CollisionFeatureEdge2D featureEdge;
//	featureEdge.m_furthestVertex = vertex;
//
//	// Use an epsilon bias to avoid floating point errors in tie breaker cases
//	if (nextDot - prevDot >= DEFAULT_EPSILON)
//	{
//		featureEdge.m_vertex1 = vertex;
//		featureEdge.m_vertex2 = nextVertex;
//		featureEdge.m_normal = nextNormal;
//		featureEdge.m_edgeId = nextVertexIndex; // Edges are ID'd by the index of their end vertex, 0 reserved for invalid
//	}
//	else
//	{
//		featureEdge.m_vertex1 = prevVertex;
//		featureEdge.m_vertex2 = vertex;
//		featureEdge.m_normal = prevNormal;
//		featureEdge.m_edgeId = vertexIndex; // Edges are ID'd by the index of their end vertex, 0 reserved for invalid
//	}
//
//	return featureEdge;
//}
//
//
////-------------------------------------------------------------------------------------------------
//void ClipIncidentEdgeToReferenceEdge(const ClipVertex2D& incident1, const ClipVertex2D& incident2, const Vector2& refEdgeDirection, float offset, std::vector<ClipVertex2D>& out_clippedPoints)
//{
//	Vector2 incident1Pos = incident1.m_position;
//	Vector2 incident2Pos = incident2.m_position;
//
//	float distance1 = DotProduct(incident1Pos, refEdgeDirection) - offset;
//	float distance2 = DotProduct(incident2Pos, refEdgeDirection) - offset;
//
//	if (distance1 >= 0)
//	{
//		out_clippedPoints.push_back(incident1);
//	}
//
//	if (distance2 >= 0)
//	{
//		out_clippedPoints.push_back(incident2);
//	}
//
//	// Special case
//	// If the two incident points are on opposite sides of the imaginary clipping line, create a new point at the clipping line
//	if (distance1 * distance2 < 0)
//	{
//		Vector2 incidentEdge = incident2Pos - incident1Pos; // Don't normalize! We'll take a fractional portion of this
//		float t = distance1 / (distance1 - distance2);
//
//		Vector2 incidentEdgeOffset = incidentEdge * t;
//		Vector2 finalPos = incident1Pos + incidentEdgeOffset;
//
//		ClipVertex2D clipVertex;
//		clipVertex.m_position = finalPos;
//
//		// Update the ID of the two edges this vertex is "between"
//		if (distance2 < 0)
//		{
//			// We clipped the edge end point
//			clipVertex.m_id = incident2.m_id;
//			clipVertex.m_id.m_maxRefEdgeClipped = 1;
//		}
//		else
//		{
//			// We clipped the edge start point
//			clipVertex.m_id = incident1.m_id;
//			clipVertex.m_id.m_minRefEdgeClipped = 1;
//		}
//
//		out_clippedPoints.push_back(clipVertex);
//	}
//}

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
