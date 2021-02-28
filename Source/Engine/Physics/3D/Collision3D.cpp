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
	first->GetFarthestVertexInDirection(direction + Vector3(DEFAULT_EPSILON), firstVertex);
	second->GetFarthestVertexInDirection(-1.0f * direction - Vector3(DEFAULT_EPSILON), secondVertex);

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
uint32 GetSimplexSeparation3D(const std::vector<Face3>& simplex, CollisionSeparation3D& out_separation)
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
		ASSERT_OR_DIE((distanceToOrigin) >= 0, "How is this distance not positive?");

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
CollisionSeparation3D PerformEPA3D(const Polygon3D* first, const Polygon3D* second, const std::vector<Vector3>& vertexSimplex)
{
	// Create a list of faces to work with instead of vertices
	// Ensure all normals point outward
	std::vector<Face3> faceSimplex;
	faceSimplex.push_back(Face3(vertexSimplex[0], vertexSimplex[1], vertexSimplex[2], vertexSimplex[0]));
	faceSimplex.push_back(Face3(vertexSimplex[0], vertexSimplex[1], vertexSimplex[3], vertexSimplex[0]));
	faceSimplex.push_back(Face3(vertexSimplex[0], vertexSimplex[2], vertexSimplex[3], vertexSimplex[0]));
	faceSimplex.push_back(Face3(vertexSimplex[1], vertexSimplex[2], vertexSimplex[3], vertexSimplex[1]));

	for (uint32 iteration = 0; iteration < NUM_EPA_ITERATIONS; ++iteration)
	{
		CollisionSeparation3D simplexSeparation;
		simplexSeparation.m_collisionFound = true;
		GetSimplexSeparation3D(faceSimplex, simplexSeparation);

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
					faceSimplex.push_back(Face3(currEdge.GetStart(), currEdge.GetEnd(), expandedMinkowskiPoint, currEdge.GetStart()));
				}
			}

			// Our simplex face is inside the Minkowski difference shape, not on the edge
			// So remove the bad face and add 3 new faces simulating adding this point
			//Face3 closestFace = faceSimplex[closestFaceIndex];
			//faceSimplex.erase(faceSimplex.begin() + closestFaceIndex);
			//
			//faceSimplex.push_back(Face3(closestFace.GetVertex(0), closestFace.GetVertex(1), expandedMinkowskiPoint, closestFace.GetVertex(0)));
			//faceSimplex.push_back(Face3(closestFace.GetVertex(0), closestFace.GetVertex(2), expandedMinkowskiPoint, closestFace.GetVertex(0)));
			//faceSimplex.push_back(Face3(closestFace.GetVertex(1), closestFace.GetVertex(2), expandedMinkowskiPoint, closestFace.GetVertex(1)));
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
		// Use EPA (Expanding Polytope Algorithm) to find the face we're overlapping with
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

//Skip to content
//Why GitHub ?
//Team
//Enterprise
//Explore
//Marketplace
//Pricing
//Search
//
//Sign in
//Sign up
//kevinmoran
///
//GJK
//478
//Code
//Issues
//1
//Pull requests
//Actions
//Projects
//Security
//Insights
//GJK / GJK.h
//@kevinmoran
//kevinmoran Small EPA fix
//…
//Latest commit 106d049 on Mar 2, 2017
//History
//1 contributor
//295 lines(259 sloc)  11.2 KB
//
//#pragma once
//#include "GameMaths.h"
//#include "Collider.h"
//
////Kevin's implementation of the Gilbert-Johnson-Keerthi intersection algorithm
////and the Expanding Polytope Algorithm
////Most useful references (Huge thanks to all the authors):
//
//// "Implementing GJK" by Casey Muratori:
//// The best description of the algorithm from the ground up
//// https://www.youtube.com/watch?v=Qupqu1xe7Io
//
//// "Implementing a GJK Intersection Query" by Phill Djonov
//// Interesting tips for implementing the algorithm
//// http://vec3.ca/gjk/implementation/
//
//// "GJK Algorithm 3D" by Sergiu Craitoiu
//// Has nice diagrams to visualise the tetrahedral case
//// http://in2gpu.com/2014/05/18/gjk-algorithm-3d/
//
//// "GJK + Expanding Polytope Algorithm - Implementation and Visualization"
//// Good breakdown of EPA with demo for visualisation
//// https://www.youtube.com/watch?v=6rgiPrzqt9w
////-----------------------------------------------------------------------------
//
////Returns true if two colliders are intersecting. Has optional Minimum Translation Vector output param;
////If supplied the EPA will be used to find the vector to separate coll1 from coll2
//bool gjk(Collider* coll1, Collider* coll2, vec3* mtv = NULL);
////Internal functions used in the GJK algorithm
//void update_simplex3(vec3 &a, vec3 &b, vec3 &c, vec3 &d, int &simp_dim, vec3 &search_dir);
//bool update_simplex4(vec3 &a, vec3 &b, vec3 &c, vec3 &d, int &simp_dim, vec3 &search_dir);
////Expanding Polytope Algorithm. Used to find the mtv of two intersecting 
////colliders using the final simplex obtained with the GJK algorithm
//vec3 EPA(vec3 a, vec3 b, vec3 c, vec3 d, Collider* coll1, Collider* coll2);
//
//#define GJK_MAX_NUM_ITERATIONS 64
//
//bool gjk(Collider* coll1, Collider* coll2, vec3* mtv) {
//	vec3 a, b, c, d; //Simplex: just a set of points (a is always most recently added)
//	vec3 search_dir = coll1->pos - coll2->pos; //initial search direction between colliders
//
//	//Get initial point for simplex
//	c = coll2->support(search_dir) - coll1->support(-search_dir);
//	search_dir = -c; //search in direction of origin
//
//	//Get second point for a line segment simplex
//	b = coll2->support(search_dir) - coll1->support(-search_dir);
//
//	if (dot(b, search_dir) < 0) { return false; }//we didn't reach the origin, won't enclose it
//
//	search_dir = cross(cross(c - b, -b), c - b); //search perpendicular to line segment towards origin
//	if (search_dir == vec3(0, 0, 0)) { //origin is on this line segment
//		//Apparently any normal search vector will do?
//		search_dir = cross(c - b, vec3(1, 0, 0)); //normal with x-axis
//		if (search_dir == vec3(0, 0, 0)) search_dir = cross(c - b, vec3(0, 0, -1)); //normal with z-axis
//	}
//	int simp_dim = 2; //simplex dimension
//
//	for (int iterations = 0; iterations < GJK_MAX_NUM_ITERATIONS; iterations++)
//	{
//		a = coll2->support(search_dir) - coll1->support(-search_dir);
//		if (dot(a, search_dir) < 0) { return false; }//we didn't reach the origin, won't enclose it
//
//		simp_dim++;
//		if (simp_dim == 3) {
//			update_simplex3(a, b, c, d, simp_dim, search_dir);
//		}
//		else if (update_simplex4(a, b, c, d, simp_dim, search_dir)) {
//			if (mtv) *mtv = EPA(a, b, c, d, coll1, coll2);
//			return true;
//		}
//	}//endfor
//	return false;
//}
//
////Triangle case
//void update_simplex3(vec3 &a, vec3 &b, vec3 &c, vec3 &d, int &simp_dim, vec3 &search_dir) {
//	/* Required winding order:
//	//  b
//	//  | \
//	//  |   \
//	//  |    a
//	//  |   /
//	//  | /
//	//  c
//	*/
//	vec3 n = cross(b - a, c - a); //triangle's normal
//	vec3 AO = -a; //direction to origin
//
//	//Determine which feature is closest to origin, make that the new simplex
//
//	simp_dim = 2;
//	if (dot(cross(b - a, n), AO) > 0) { //Closest to edge AB
//		c = a;
//		//simp_dim = 2;
//		search_dir = cross(cross(b - a, AO), b - a);
//		return;
//	}
//	if (dot(cross(n, c - a), AO) > 0) { //Closest to edge AC
//		b = a;
//		//simp_dim = 2;
//		search_dir = cross(cross(c - a, AO), c - a);
//		return;
//	}
//
//	simp_dim = 3;
//	if (dot(n, AO) > 0) { //Above triangle
//		d = c;
//		c = b;
//		b = a;
//		//simp_dim = 3;
//		search_dir = n;
//		return;
//	}
//	//else //Below triangle
//	d = b;
//	b = a;
//	//simp_dim = 3;
//	search_dir = -n;
//	return;
//}
//
////Tetrahedral case
//bool update_simplex4(vec3 &a, vec3 &b, vec3 &c, vec3 &d, int &simp_dim, vec3 &search_dir) {
//	// a is peak/tip of pyramid, BCD is the base (counterclockwise winding order)
//	//We know a priori that origin is above BCD and below a
//
//	//Get normals of three new faces
//	vec3 ABC = cross(b - a, c - a);
//	vec3 ACD = cross(c - a, d - a);
//	vec3 ADB = cross(d - a, b - a);
//
//	vec3 AO = -a; //dir to origin
//	simp_dim = 3; //hoisting this just cause
//
//	//Plane-test origin with 3 faces
//	/*
//	// Note: Kind of primitive approach used here; If origin is in front of a face, just use it as the new simplex.
//	// We just go through the faces sequentially and exit at the first one which satisfies dot product. Not sure this
//	// is optimal or if edges should be considered as possible simplices? Thinking this through in my head I feel like
//	// this method is good enough. Makes no difference for AABBS, should test with more complex colliders.
//	*/
//	if (dot(ABC, AO) > 0) { //In front of ABC
//		d = c;
//		c = b;
//		b = a;
//		search_dir = ABC;
//		return false;
//	}
//	if (dot(ACD, AO) > 0) { //In front of ACD
//		b = a;
//		search_dir = ACD;
//		return false;
//	}
//	if (dot(ADB, AO) > 0) { //In front of ADB
//		c = d;
//		d = b;
//		b = a;
//		search_dir = ADB;
//		return false;
//	}
//
//	//else inside tetrahedron; enclosed!
//	return true;
//
//	//Note: in the case where two of the faces have similar normals,
//	//The origin could conceivably be closest to an edge on the tetrahedron
//	//Right now I don't think it'll make a difference to limit our new simplices
//	//to just one of the faces, maybe test it later.
//}
//
////Expanding Polytope Algorithm
////Find minimum translation vector to resolve collision
//#define EPA_TOLERANCE 0.0001
//#define EPA_MAX_NUM_FACES 64
//#define EPA_MAX_NUM_LOOSE_EDGES 32
//#define EPA_MAX_NUM_ITERATIONS 64
//vec3 EPA(vec3 a, vec3 b, vec3 c, vec3 d, Collider* coll1, Collider* coll2) {
//	vec3 faces[EPA_MAX_NUM_FACES][4]; //Array of faces, each with 3 verts and a normal
//
//	//Init with final simplex from GJK
//	faces[0][0] = a;
//	faces[0][1] = b;
//	faces[0][2] = c;
//	faces[0][3] = normalise(cross(b - a, c - a)); //ABC
//	faces[1][0] = a;
//	faces[1][1] = c;
//	faces[1][2] = d;
//	faces[1][3] = normalise(cross(c - a, d - a)); //ACD
//	faces[2][0] = a;
//	faces[2][1] = d;
//	faces[2][2] = b;
//	faces[2][3] = normalise(cross(d - a, b - a)); //ADB
//	faces[3][0] = b;
//	faces[3][1] = d;
//	faces[3][2] = c;
//	faces[3][3] = normalise(cross(d - b, c - b)); //BDC
//
//	int num_faces = 4;
//	int closest_face;
//
//	for (int iterations = 0; iterations < EPA_MAX_NUM_ITERATIONS; iterations++) {
//		//Find face that's closest to origin
//		float min_dist = dot(faces[0][0], faces[0][3]);
//		closest_face = 0;
//		for (int i = 1; i < num_faces; i++) {
//			float dist = dot(faces[i][0], faces[i][3]);
//			if (dist < min_dist) {
//				min_dist = dist;
//				closest_face = i;
//			}
//		}
//
//		//search normal to face that's closest to origin
//		vec3 search_dir = faces[closest_face][3];
//		vec3 p = coll2->support(search_dir) - coll1->support(-search_dir);
//
//		if (dot(p, search_dir) - min_dist < EPA_TOLERANCE) {
//			//Convergence (new point is not significantly further from origin)
//			return faces[closest_face][3] * dot(p, search_dir); //dot vertex with normal to resolve collision along normal!
//		}
//
//		vec3 loose_edges[EPA_MAX_NUM_LOOSE_EDGES][2]; //keep track of edges we need to fix after removing faces
//		int num_loose_edges = 0;
//
//		//Find all triangles that are facing p
//		for (int i = 0; i < num_faces; i++)
//		{
//			if (dot(faces[i][3], p - faces[i][0]) > 0) //triangle i faces p, remove it
//			{
//				//Add removed triangle's edges to loose edge list.
//				//If it's already there, remove it (both triangles it belonged to are gone)
//				for (int j = 0; j < 3; j++) //Three edges per face
//				{
//					vec3 current_edge[2] = { faces[i][j], faces[i][(j + 1) % 3] };
//					bool found_edge = false;
//					for (int k = 0; k < num_loose_edges; k++) //Check if current edge is already in list
//					{
//						if (loose_edges[k][1] == current_edge[0] && loose_edges[k][0] == current_edge[1]) {
//							//Edge is already in the list, remove it
//							//THIS ASSUMES EDGE CAN ONLY BE SHARED BY 2 TRIANGLES (which should be true)
//							//THIS ALSO ASSUMES SHARED EDGE WILL BE REVERSED IN THE TRIANGLES (which 
//							//should be true provided every triangle is wound CCW)
//							loose_edges[k][0] = loose_edges[num_loose_edges - 1][0]; //Overwrite current edge
//							loose_edges[k][1] = loose_edges[num_loose_edges - 1][1]; //with last edge in list
//							num_loose_edges--;
//							found_edge = true;
//							k = num_loose_edges; //exit loop because edge can only be shared once
//						}
//					}//endfor loose_edges
//
//					if (!found_edge) { //add current edge to list
//						// assert(num_loose_edges<EPA_MAX_NUM_LOOSE_EDGES);
//						if (num_loose_edges >= EPA_MAX_NUM_LOOSE_EDGES) break;
//						loose_edges[num_loose_edges][0] = current_edge[0];
//						loose_edges[num_loose_edges][1] = current_edge[1];
//						num_loose_edges++;
//					}
//				}
//
//				//Remove triangle i from list
//				faces[i][0] = faces[num_faces - 1][0];
//				faces[i][1] = faces[num_faces - 1][1];
//				faces[i][2] = faces[num_faces - 1][2];
//				faces[i][3] = faces[num_faces - 1][3];
//				num_faces--;
//				i--;
//			}//endif p can see triangle i
//		}//endfor num_faces
//
//		//Reconstruct polytope with p added
//		for (int i = 0; i < num_loose_edges; i++)
//		{
//			// assert(num_faces<EPA_MAX_NUM_FACES);
//			if (num_faces >= EPA_MAX_NUM_FACES) break;
//			faces[num_faces][0] = loose_edges[i][0];
//			faces[num_faces][1] = loose_edges[i][1];
//			faces[num_faces][2] = p;
//			faces[num_faces][3] = normalise(cross(loose_edges[i][0] - loose_edges[i][1], loose_edges[i][0] - p));
//
//			//Check for wrong normal to maintain CCW winding
//			float bias = 0.000001; //in case dot result is only slightly < 0 (because origin is on face)
//			if (dot(faces[num_faces][0], faces[num_faces][3]) + bias < 0) {
//				vec3 temp = faces[num_faces][0];
//				faces[num_faces][0] = faces[num_faces][1];
//				faces[num_faces][1] = temp;
//				faces[num_faces][3] = -faces[num_faces][3];
//			}
//			num_faces++;
//		}
//	} //End for iterations
//	printf("EPA did not converge\n");
//	//Return most recent closest point
//	return faces[closest_face][3] * dot(faces[closest_face][0], faces[closest_face][3]);
//}
//© 2021 GitHub, Inc.
//Terms
//Privacy
//Security
//Status
//Docs
//Contact GitHub
//Pricing
//API
//Training
//Blog
//About
