///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: September 25th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/DevConsole.h"
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/Polygon3d.h"
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
Vector3 GetMinkowskiDiffSupport3D(const Polygon3d* first, const Polygon3d* second, const Vector3& direction)
{
	Vector3 firstVertex, secondVertex;
	first->GetSupportPoint(direction + Vector3(DEFAULT_EPSILON), firstVertex);
	second->GetSupportPoint(-1.0f * direction - Vector3(DEFAULT_EPSILON), secondVertex);

	return firstVertex - secondVertex;
}


//-------------------------------------------------------------------------------------------------
bool SetupSimplex3D(const Polygon3d* first, const Polygon3d* second, std::vector <Vector3>& simplex)
{
	// A vertex
	Vector3 direction = second->GetCenter() - first->GetCenter();
	simplex.push_back(GetMinkowskiDiffSupport3D(first, second, direction));

	// B vertex
	direction = -1.0f * direction;
	simplex.push_back(GetMinkowskiDiffSupport3D(first, second, direction));

	if (AreMostlyEqual(simplex[0], simplex[1]))
	{
		ConsoleErrorf("3d simplex couldn't find second unique vertex, aborted");
		return false;
	}

	// C vertex
	Vector3 a = simplex[0];
	Vector3 b = simplex[1];
	Vector3 ab = b - a;
	ab.Normalize();
	Vector3 aToOrigin = -1.0f * a;

	// Get a vector orthogonal to ab
	bool abPointingVertically = AreMostlyEqual(Abs(DotProduct(Vector3::Y_AXIS, ab)), 1.f);
	Vector3 hint = (!abPointingVertically ? Vector3::Y_AXIS : Vector3::X_AXIS);
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

	if (AreMostlyEqual(simplex[0], simplex[2]) || AreMostlyEqual(simplex[1], simplex[2]))
	{
		ConsoleErrorf("3d simplex couldn't find 3rd unique vertex, aborted");
		return false;
	}

	// D vertex
	Vector3 c = simplex[2];
	Vector3 inwardNormal = -1.0f * CalculateNormalForTriangle(a, b, c); // -1.0 to make it point inward

	// Make sure the new direction is pointing towards the origin
	if (DotProduct(inwardNormal, aToOrigin) < 0.f)
	{
		// Update the ordering so the cross points towards the origin
		simplex[0] = b;
		simplex[1] = a;

		inwardNormal *= -1.0f;
	}

	simplex.push_back(GetMinkowskiDiffSupport3D(first, second, inwardNormal));

	if (AreMostlyEqual(simplex[0], simplex[3]) || AreMostlyEqual(simplex[1], simplex[3]) || AreMostlyEqual(simplex[2], simplex[3]))
	{
		ConsoleErrorf("3d simplex couldn't find 4th unique vertex, aborted");
		return false;
	}

	return true;
}


//-------------------------------------------------------------------------------------------------
EvolveSimplexResult EvolveSimplex3D(const Polygon3d* first, const Polygon3d* second, std::vector<Vector3>& evolvingSimplex)
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
uint32 GetSimplexSeparation3D(const std::vector<Face3>& simplex, CollisionSeparation3d& out_separation)
{
	uint32 numFaces = (uint32)simplex.size();
	uint32 closestIndex = 0;

	for (uint32 faceIndex = 0; faceIndex < numFaces; ++faceIndex)
	{
		const Face3& face = simplex[faceIndex];

		// Get the outward facing normal
		Vector3 normal = face.GetNormal();
		Vector3 faceVertex = face.GetVertex(0);

		float distanceToOrigin = DotProduct(normal, faceVertex);
		ASSERT_OR_DIE((distanceToOrigin) >= -DEFAULT_EPSILON, "How is this distance not positive?");

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
void AddOrRemoveLooseEdge(const Edge3& edge, std::vector<Edge3>& looseEdges)
{
	int numLooseEdges = (int)looseEdges.size();

	for (int edgeIndex = 0; edgeIndex < numLooseEdges; ++edgeIndex)
	{
		if (edge.IsEquivalentTo(looseEdges[edgeIndex]))
		{
			looseEdges.erase(looseEdges.begin() + edgeIndex);
			return;
		}
	}

	// This edge doesn't already exist in the list, so re-add it
	looseEdges.push_back(edge);
}


//-------------------------------------------------------------------------------------------------
CollisionSeparation3d PerformEPA3D(const Polygon3d* first, const Polygon3d* second, const std::vector<Vector3>& vertexSimplex)
{
	// Create a list of faces to work with instead of vertices
	// Ensure all normals point outward
	std::vector<Face3> faceSimplex;
	faceSimplex.push_back(Face3(vertexSimplex[0], vertexSimplex[1], vertexSimplex[2], -1.0f * vertexSimplex[3]));
	faceSimplex.push_back(Face3(vertexSimplex[0], vertexSimplex[1], vertexSimplex[3], -1.0f * vertexSimplex[2]));
	faceSimplex.push_back(Face3(vertexSimplex[0], vertexSimplex[2], vertexSimplex[3], -1.0f * vertexSimplex[1]));
	faceSimplex.push_back(Face3(vertexSimplex[1], vertexSimplex[2], vertexSimplex[3], -1.0f * vertexSimplex[0]));

	for (uint32 iteration = 0; iteration < NUM_EPA_ITERATIONS; ++iteration)
	{
		CollisionSeparation3d simplexSeparation;
		simplexSeparation.m_collisionFound = true;
		GetSimplexSeparation3D(faceSimplex, simplexSeparation);

		Vector3 expandedMinkowskiPoint = GetMinkowskiDiffSupport3D(first, second, simplexSeparation.m_dirFromFirst);
		float distanceToMinkowskiEdge = DotProduct(simplexSeparation.m_dirFromFirst, expandedMinkowskiPoint);
		ASSERT_OR_DIE(distanceToMinkowskiEdge >= -DEFAULT_EPSILON, "This should always be positive!");

		float diff = Abs(simplexSeparation.m_separation - distanceToMinkowskiEdge);
		if (diff < DEFAULT_EPSILON)
		{
			// Difference is close enough, this is the right edge
			return simplexSeparation;
		}
		else
		{
			// Remove all faces the point "sees"
			int numFaces = faceSimplex.size();
			std::vector<Edge3> looseEdges;

			for (int faceIndex = numFaces - 1; faceIndex >= 0; --faceIndex)
			{
				Face3 face = faceSimplex[faceIndex];

				Vector3 normal = face.GetNormal();
				Vector3 pointToFace = face.GetVertex(0) - expandedMinkowskiPoint;
				float dot = DotProduct(normal, pointToFace);
				if (dot < 0.f)
				{
					// Face is facing towards from the point, so remove it
					faceSimplex.erase(faceSimplex.begin() + faceIndex);

					// Keep track of any edges that this face leaves "loose"
					// When adding the new point in we'll connect it to these edges
					int numEdges = face.GetNumEdges();
					for (int edgeIndex = 0; edgeIndex < numEdges; ++edgeIndex)
					{
						AddOrRemoveLooseEdge(face.GetEdge(edgeIndex), looseEdges);
					}
				}
			}

			// Reconstruct the simplex adding in this new closest point
			int numLooseEdges = looseEdges.size();
			if (numLooseEdges > 0)
			{
				for (int edgeIndex = 0; edgeIndex < numLooseEdges; ++edgeIndex)
				{
					const Edge3& currEdge = looseEdges[edgeIndex];
					Face3 newFace(currEdge.GetStart(), currEdge.GetEnd(), expandedMinkowskiPoint, currEdge.GetStart());

					// Check for duplicates.......shouldn't have duplicates...
					for (int faceIndex = 0; faceIndex < (int)faceSimplex.size(); ++faceIndex)
					{
						if (faceSimplex[faceIndex].IsEquivalentTo(newFace))
						{
							ERROR_AND_DIE("Dupe face!");
						}
					}

					faceSimplex.push_back(newFace);
				}
			}
		}
	}
	
	ERROR_RECOVERABLE("Couldn't find the Minkowski face?");
	return CollisionSeparation3d(false);
}


//-------------------------------------------------------------------------------------------------
CollisionSeparation3d CalculateSeparation3D(const Polygon3d* first, const Polygon3d* second)
{
	EvolveSimplexResult result = SIMPLEX_STILL_EVOLVING;
	std::vector<Vector3> simplex;

	// Set up initial vertices
	bool success = SetupSimplex3D(first, second, simplex);

	if (!success)
	{
		// Sometimes due to floating point error we get false positives which leads to math falling apart...
		return CollisionSeparation3d(false);
	}

	while (result == SIMPLEX_STILL_EVOLVING)
	{
		result = EvolveSimplex3D(first, second, simplex);
	}

	if (result == INTERSECTION_FOUND)
	{
		// Use EPA (Expanding Polytope Algorithm) to find the face we're overlapping with
		return PerformEPA3D(first, second, simplex);
	}

	// Must be no collision
	return CollisionSeparation3d(false);
}


//-------------------------------------------------------------------------------------------------
CollisionFace3d GetFeatureFace3D(const Polygon3d* polygon, const Vector3& outwardSeparationNormal)
{
	// Get the face who's outward normal is mostly in this direction
	CollisionFace3d featureFace;
	
	float bestDot = -1.0f;
	int numFaces = polygon->GetNumFaces();

	for (int faceIndex = 0; faceIndex < numFaces; ++faceIndex)
	{
		Vector3 faceNormal = polygon->GetFaceNormal(faceIndex);
		float dot = DotProduct(faceNormal, outwardSeparationNormal);

		if (faceIndex == 0 || dot > bestDot)
		{
			bestDot = dot;
			featureFace.m_normal = faceNormal;
			featureFace.m_faceIndex = faceIndex;
		}
	}

	// Get the furthest point along the separation normal
	const PolygonFace3d* face = polygon->GetFace(featureFace.m_faceIndex);
	int numVertsInFace = (int)face->m_indices.size();

	bestDot = -1.0f;

	for (int faceVertexIndex = 0; faceVertexIndex < numVertsInFace; ++faceVertexIndex)
	{
		Vector3 vertPosition = polygon->GetVertexPosition(face->m_indices[faceVertexIndex]);

		float dot = DotProduct(vertPosition, outwardSeparationNormal);

		if (faceVertexIndex == 0 || dot > bestDot)
		{
			bestDot = dot;
			featureFace.m_furthestVertex = vertPosition;
		}
	}

	return featureFace;
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
