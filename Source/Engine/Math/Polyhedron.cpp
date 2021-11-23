///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 25th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/Math/Matrix4.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/OBB3.h"
#include "Engine/Math/Polyhedron.h"
#include "Engine/Math/Transform.h"
#include "Engine/Render/RenderContext.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
typedef std::pair<int, int> HalfEdgeKey;

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
Polyhedron::Polyhedron(const OBB3& box)
{
	Vector3 points[8];
	box.GetPoints(points);

	for (int i = 0; i < 8; ++i)
	{
		m_vertices.push_back(PolyhedronVertex(points[i]));
	}

	// Back
	PolyhedronFace back(*this);
	back.m_indices.push_back(0);
	back.m_indices.push_back(1);
	back.m_indices.push_back(2);
	back.m_indices.push_back(3);
	m_faces.push_back(back);

	// Front
	PolyhedronFace front(*this);
	front.m_indices.push_back(4);
	front.m_indices.push_back(5);
	front.m_indices.push_back(6);
	front.m_indices.push_back(7);
	m_faces.push_back(front);

	// Left
	PolyhedronFace left(*this);
	left.m_indices.push_back(7);
	left.m_indices.push_back(6);
	left.m_indices.push_back(1);
	left.m_indices.push_back(0);
	m_faces.push_back(left);

	// Right
	PolyhedronFace right(*this);
	right.m_indices.push_back(3);
	right.m_indices.push_back(2);
	right.m_indices.push_back(5);
	right.m_indices.push_back(4);
	m_faces.push_back(right);

	// Bottom
	PolyhedronFace bottom(*this);
	bottom.m_indices.push_back(7);
	bottom.m_indices.push_back(0);
	bottom.m_indices.push_back(3);
	bottom.m_indices.push_back(4);
	m_faces.push_back(bottom);

	// Top
	PolyhedronFace top(*this);
	top.m_indices.push_back(1);
	top.m_indices.push_back(6);
	top.m_indices.push_back(5);
	top.m_indices.push_back(2);
	m_faces.push_back(top);

	GenerateHalfEdgeStructure();
}


//-------------------------------------------------------------------------------------------------
// Copy constructor to ensure faces point to the right polygon
Polyhedron::Polyhedron(const Polyhedron& copy)
{
	m_vertices = copy.m_vertices;
	m_faces = copy.m_faces;
	for (int iFace = 0; iFace < (int)m_faces.size(); ++iFace)
	{
		m_faces[iFace].m_poly = this;
	}

	m_edges = copy.m_edges;
}


//-------------------------------------------------------------------------------------------------
void Polyhedron::Clear()
{
	m_vertices.clear();
	m_faces.clear();
	m_edges.clear();
}


//-------------------------------------------------------------------------------------------------
void Polyhedron::GenerateHalfEdgeStructure()
{
	int numFaces = (int)m_faces.size();
	int numVertices = (int)m_vertices.size();

	ASSERT_OR_DIE(numVertices > 0, "No vertices!");
	ASSERT_OR_DIE(numFaces > 0, "No faces!");
	ASSERT_OR_DIE(!HasGeneratedHalfEdges(), "Edges already generated!");

	// Create all the edges
	// Keep a local map for now to help hook up references later
	std::map<HalfEdgeKey, int> edgeIndexMap;
	for (int faceIndex = 0; faceIndex < numFaces; ++faceIndex)
	{
		PolyhedronFace& face = m_faces[faceIndex];
		int numIndices = (int)face.m_indices.size();
		ASSERT_OR_DIE(numIndices > 2, "Not enough indices in face!");

		for (int i = 0; i < numIndices; ++i)
		{
			int j = (i + 1) % numIndices;

			int vertexIndex1 = face.m_indices[i];
			int vertexIndex2 = face.m_indices[j];

			HalfEdge halfEdge;
			halfEdge.m_faceIndex = faceIndex;
			halfEdge.m_vertexIndex = vertexIndex1;
			halfEdge.m_edgeIndex = (int)m_edges.size();

			m_edges.push_back(halfEdge);

			HalfEdgeKey edgeKey(vertexIndex1, vertexIndex2);
			edgeIndexMap[edgeKey] = (int)m_edges.size() - 1;
		}
	}

	// Check if this holds true...
	ASSERT_OR_DIE((int)m_edges.size() == 2 * (numVertices + numFaces - 2), "Euler's Formula failed!");

	// Now that all possible half edges are created, we can begin linking them
	// Otherwise the map may shift elements around, invalidating all pointers
	for (int faceIndex = 0; faceIndex < numFaces; ++faceIndex)
	{
		PolyhedronFace& face = m_faces[faceIndex];
		int numIndices = (int)face.m_indices.size();

		for (int i = 0; i < numIndices; ++i)
		{
			int j = (i + 1) % numIndices;

			int vertexIndexI = face.m_indices[i];
			int vertexIndexJ = face.m_indices[j];

			HalfEdgeKey currEdgeKey(vertexIndexI, vertexIndexJ);
			int currEdgeIndex = edgeIndexMap.at(currEdgeKey);
			HalfEdge& currHalfEdge = m_edges[currEdgeIndex];

			// Connect the next/prev edges
			{
				int k = (j + 1) % numIndices;
				int vertexIndexK = face.m_indices[k];

				HalfEdgeKey nextEdgeKey(vertexIndexJ, vertexIndexK);
				int nextEdgeIndex = edgeIndexMap[nextEdgeKey];
				HalfEdge& nextHalfEdge = m_edges[nextEdgeIndex];

				currHalfEdge.m_nextEdgeIndex = nextEdgeIndex;
				nextHalfEdge.m_prevEdgeIndex = currEdgeIndex;
			}

			// Connect mirror edges
			{
				HalfEdgeKey mirrorEdgeKey(vertexIndexJ, vertexIndexI); // Flipped indices, since it's the mirror of this edge
				int mirrorEdgeIndex = edgeIndexMap[mirrorEdgeKey];
				HalfEdge& mirrorHalfEdge = m_edges[mirrorEdgeIndex];
				currHalfEdge.m_mirrorEdgeIndex = mirrorEdgeIndex;
				mirrorHalfEdge.m_mirrorEdgeIndex = currEdgeIndex;
			}
			

			// Connect this face to any half edge inside it
			face.m_iHalfEdge = currEdgeIndex;

			// Connect this vertex to any half edge going out of it
			m_vertices[vertexIndexI].m_halfEdgeIndex = currEdgeIndex;
		}
	}

	// Safety checks
	int numEdges = (int)m_edges.size();
	for (int edgeIndex = 0; edgeIndex < numEdges; ++edgeIndex)
	{
		const HalfEdge& currEdge = m_edges[edgeIndex];

		ASSERT_OR_DIE(currEdge.m_mirrorEdgeIndex != -1, "Invalid mirror edge!");
		ASSERT_OR_DIE(currEdge.m_nextEdgeIndex != -1, "Invalid next edge!");
		ASSERT_OR_DIE(currEdge.m_prevEdgeIndex != -1, "Invalid prev edge!");
		ASSERT_OR_DIE(currEdge.m_vertexIndex != -1, "Invalid vertex!");
		ASSERT_OR_DIE(currEdge.m_faceIndex != -1, "Invalid face!");
		ASSERT_OR_DIE(currEdge.m_edgeIndex == edgeIndex, "Edge index mismatch!");
	}

	for (int faceIndex = 0; faceIndex < numFaces; ++faceIndex)
	{
		ASSERT_OR_DIE(m_faces[faceIndex].m_iHalfEdge != -1, "Invalid half edge!");
	}

	for (int vertexIndex = 0; vertexIndex < numVertices; ++vertexIndex)
	{
		ASSERT_OR_DIE(m_vertices[vertexIndex].m_halfEdgeIndex != -1, "Invalid half edge!");
	}

	// Now we can build edge planes for all the faces
	for (int iFace = 0; iFace < (int)m_faces.size(); ++iFace)
	{
		m_faces[iFace].CalculateNormalAndSidePlanes();
	}
}


//-------------------------------------------------------------------------------------------------
int Polyhedron::AddVertex(const Vector3& vertex)
{
	ASSERT_OR_DIE(!HasGeneratedHalfEdges(), "Cannot edit a Polygon3d after half edges are generated!");

	PolyhedronVertex polyVertex(vertex);
	m_vertices.push_back(polyVertex);

	return (int)(m_vertices.size() - 1);
}


//-------------------------------------------------------------------------------------------------
int Polyhedron::AddFace(const std::vector<int>& indices)
{
	ASSERT_OR_DIE(!HasGeneratedHalfEdges(), "Cannot edit a Polygon3d after half edges are generated!");

	m_faces.push_back(PolyhedronFace(*this, indices));
	return (int)(m_faces.size() - 1);
}


//-------------------------------------------------------------------------------------------------
const PolyhedronVertex* Polyhedron::GetVertex(int vertexIndex) const
{
	return &m_vertices[vertexIndex];
}


//-------------------------------------------------------------------------------------------------
void Polyhedron::GetTransformed(const Matrix4& matrix, Polyhedron& out_polygon) const
{
	int numVertices = (int)m_vertices.size();
	for (int vertexIndex = 0; vertexIndex < numVertices; ++vertexIndex)
	{
		Vector3 position = matrix.TransformPosition(m_vertices[vertexIndex].m_position);
		out_polygon.m_vertices.push_back(PolyhedronVertex(position, m_vertices[vertexIndex].m_halfEdgeIndex));
	}

	// Edges first, since faces depend on them
	out_polygon.m_edges = m_edges;

	out_polygon.m_faces = m_faces;
	for (int iFace = 0; iFace < (int)m_faces.size(); ++iFace)
	{
		out_polygon.m_faces[iFace].m_poly = &out_polygon; // Ensure they point to the new polyhedron
		out_polygon.m_faces[iFace].ApplyTransform(matrix);
	}
}


//-------------------------------------------------------------------------------------------------
Vector3 Polyhedron::GetVertexPosition(int vertexIndex) const
{
	return m_vertices[vertexIndex].m_position;
}


//-------------------------------------------------------------------------------------------------
void Polyhedron::GetAllVerticesInFace(int faceIndex, std::vector<Vector3>& out_vertices) const
{
	out_vertices.clear();
	int startingEdge = GetFace(faceIndex)->m_iHalfEdge;
	int edgeIndex = startingEdge;

	do 
	{
		const HalfEdge* edge = GetEdge(edgeIndex);
		out_vertices.push_back(GetVertexPosition(edge->m_vertexIndex));
		edgeIndex = edge->m_nextEdgeIndex;
	} 
	while (edgeIndex != startingEdge);
}


//-------------------------------------------------------------------------------------------------
const PolyhedronFace* Polyhedron::GetFace(int faceIndex) const
{
	return &m_faces[faceIndex];
}


//-------------------------------------------------------------------------------------------------
// Returns the face who's normal is the most in the given direction
int Polyhedron::GetIndexOfFaceMostInDirection(const Vector3& direction) const
{
	float maxDot = 0.f;
	int maxFaceIndex = -1;
	int numIncFaces = GetNumFaces();

	for (int faceIndex = 0; faceIndex < numIncFaces; ++faceIndex)
	{
		Vector3 faceNormal = GetFaceNormal(faceIndex);
		float dot = DotProduct(faceNormal, direction);

		if (maxFaceIndex == -1 || dot > maxDot)
		{
			maxDot = dot;
			maxFaceIndex = faceIndex;
		}
	}

	return maxFaceIndex;
}


//-------------------------------------------------------------------------------------------------
const PolyhedronFace* Polyhedron::GetFaceMostInDirection(const Vector3& direction) const
{
	int faceIndex = GetIndexOfFaceMostInDirection(direction);
	return &m_faces[faceIndex];
}


//-------------------------------------------------------------------------------------------------
Vector3 Polyhedron::GetFaceNormal(int faceIndex) const
{
	return m_faces[faceIndex].m_normal;
}


//-------------------------------------------------------------------------------------------------
Plane3 Polyhedron::GetFaceSupportPlane(int faceIndex) const
{
	const PolyhedronFace& face = m_faces[faceIndex];
	return face.GetSupportPlane();
}


//-------------------------------------------------------------------------------------------------
const HalfEdge* Polyhedron::GetEdge(int edgeIndex) const
{
	ASSERT_OR_DIE(HasGeneratedHalfEdges(), "No edges!");
	return &m_edges[edgeIndex];
}


//-------------------------------------------------------------------------------------------------
Vector3 Polyhedron::GetEdgeDirection(int edgeIndex) const
{
	const HalfEdge* edge = GetEdge(edgeIndex);
	return GetEdgeDirection(edge);
}


//-------------------------------------------------------------------------------------------------
Vector3 Polyhedron::GetEdgeDirection(const HalfEdge* edge) const
{
	Vector3 start = GetVertexPosition(edge->m_vertexIndex);
	const HalfEdge* nextEdge = GetEdge(edge->m_nextEdgeIndex);
	Vector3 end = GetVertexPosition(nextEdge->m_vertexIndex);

	return (end - start);
}


//-------------------------------------------------------------------------------------------------
void Polyhedron::GetEdgeEndPoints(int edgeIndex, Vector3& out_start, Vector3& out_end) const
{
	const HalfEdge* edge = GetEdge(edgeIndex);
	const HalfEdge* nextEdge = GetEdge(edge->m_nextEdgeIndex);

	out_start = GetVertexPosition(edge->m_vertexIndex);
	out_end = GetVertexPosition(nextEdge->m_vertexIndex);
}


//-------------------------------------------------------------------------------------------------
Vector3 Polyhedron::GetEdgeDirectionNormalized(int edgeIndex) const
{
	return GetEdgeDirection(edgeIndex).GetNormalized();
}


//-------------------------------------------------------------------------------------------------
int Polyhedron::GetSupportPoint(const Vector3& direction, Vector3& out_vertex) const
{
	ASSERT_OR_DIE(m_vertices.size() > 0, "No vertices to return!");

	// Early out...but this shouldn't happen
	if (m_vertices.size() == 1U)
	{
		out_vertex = m_vertices[0].m_position;
		return 0;
	}

	float maxDot = -1.f;
	int bestIndex = -1;

	for (int vertexIndex = 0; vertexIndex < (int)m_vertices.size(); ++vertexIndex)
	{
		// Treat the vertex position as a vector from 0,0
		const Vector3& currVector = m_vertices[vertexIndex].m_position;

		float dot = DotProduct(currVector, direction);
		if (bestIndex == -1 || dot > maxDot)
		{
			maxDot = dot;
			bestIndex = vertexIndex;
		}
	}

	out_vertex = m_vertices[bestIndex].m_position;
	return bestIndex;
}


//-------------------------------------------------------------------------------------------------
Vector3 Polyhedron::GetCenter() const
{
	uint32 numVertices = (uint32)m_vertices.size();
	ASSERT_RETURN(numVertices > 0U, Vector3::ZERO, "Polygon3D has no vertices!");

	Vector3 average = Vector3::ZERO;
	for (uint32 i = 0; i < numVertices; ++i)
	{
		average += m_vertices[i].m_position;
	}

	average /= static_cast<float>(numVertices);

	return average;
}


//-------------------------------------------------------------------------------------------------
bool Polyhedron::IsConvex() const
{
	for (int iPlane = 0; iPlane < (int)m_faces.size(); ++iPlane)
	{
		Plane3 plane = GetFaceSupportPlane(iPlane);

		for (int iCheckFace = 0; iCheckFace < (int)m_faces.size(); ++iCheckFace)
		{
			if (iPlane == iCheckFace)
				continue;

			const PolyhedronFace& face = m_faces[iCheckFace];
			for (int iVertex : face.m_indices)
			{
				Vector3 vertex = GetVertexPosition(iVertex);
				
				if (plane.GetDistanceFromPlane(vertex) > DEFAULT_EPSILON)
					return false;
			}
		}
	}

	return true;
}


//-------------------------------------------------------------------------------------------------
int Polyhedron::GetAllAdjacentFaces(int faceIndex, std::vector<const PolyhedronFace*>& out_faces) const
{
	const PolyhedronFace& face = m_faces[faceIndex];
	return face.GetAllAdjacentFaces(out_faces);
}


//-------------------------------------------------------------------------------------------------
UniqueHalfEdgeIterator::UniqueHalfEdgeIterator(const Polyhedron& polygon)
	: m_polygon(polygon)
{
}


//-------------------------------------------------------------------------------------------------
const HalfEdge* UniqueHalfEdgeIterator::GetNext()
{
	for (m_currIndex; m_currIndex < m_polygon.GetNumEdges(); ++m_currIndex)
	{
		// Get the next edge
		const HalfEdge* currEdge = m_polygon.GetEdge(m_currIndex);

		// Check if this edge's mirror has been visited already
		bool mirrorVisited = std::find(m_visitedList.begin(), m_visitedList.end(), currEdge->m_mirrorEdgeIndex) != m_visitedList.end();

		if (!mirrorVisited)
		{
			m_visitedList.push_back(m_currIndex);
			m_visitedList.push_back(currEdge->m_mirrorEdgeIndex);
			++m_currIndex;

			return currEdge;
		}
	}

	return nullptr;
}


//-------------------------------------------------------------------------------------------------
PolyhedronFace::PolyhedronFace(const Polyhedron& polyhedron, const std::vector<int> indices)
	: m_poly(&polyhedron), m_indices(indices)
{
}


//-------------------------------------------------------------------------------------------------
PolyhedronFace::PolyhedronFace(const Polyhedron& polyhedron)
	: m_poly(&polyhedron)
{
}


//-------------------------------------------------------------------------------------------------
Plane3 PolyhedronFace::GetSupportPlane() const
{
	return Plane3(m_normal, m_poly->GetVertexPosition(m_indices[0]));
}


//-------------------------------------------------------------------------------------------------
int PolyhedronFace::GetAllAdjacentFaces(std::vector<const PolyhedronFace*>& out_faces) const
{
	const HalfEdge* startingEdge = m_poly->GetEdge(m_iHalfEdge);
	const HalfEdge* currEdge = startingEdge;

	do
	{
		// Get my mirror, then the face my mirror points to
		const HalfEdge* mirrorEdge = m_poly->GetEdge(currEdge->m_mirrorEdgeIndex);
		const PolyhedronFace* currFace = m_poly->GetFace(mirrorEdge->m_faceIndex);

		bool alreadyIncluded = std::find(out_faces.begin(), out_faces.end(), currFace) != out_faces.end();

		if (!alreadyIncluded)
		{
			out_faces.push_back(currFace);
		}

		currEdge = m_poly->GetEdge(currEdge->m_nextEdgeIndex);
	} 
	while (currEdge != startingEdge);

	return (int)out_faces.size();
}


//-------------------------------------------------------------------------------------------------
bool PolyhedronFace::IsPointWithinEdges(const Vector3& point) const
{
	for (Plane3 plane : m_sidePlanes)
	{
		if (plane.IsPointInFront(point))
			return false;
	}

	return true;
}


//-------------------------------------------------------------------------------------------------
// Returns true if the segment is over the face and was even up for consideration for clipping
bool PolyhedronFace::ClipEdgeToFace(LineSegment3& inout_edge) const
{
	// Can't clip the segment if it never even crosses the face
	for (int iSidePlane = 0; iSidePlane < (int)m_sidePlanes.size(); ++iSidePlane)
	{
		const Plane3& sidePlane = m_sidePlanes[iSidePlane];
		
		float aDistance = sidePlane.GetDistanceFromPlane(inout_edge.m_a);
		float bDistance = sidePlane.GetDistanceFromPlane(inout_edge.m_b);
		
		// Positive distance is outside this edge's plane
		// One being positive and one being negative means for certain it's over the face
		// Both negative could mean both are over face, but also could mean they're so far 
		// behind they're off the face in the other direction - in that case, we'll get a 
		// double positive in the opposite edge plane case
		// Double positive means for certain it's not over the face
		if (aDistance > 0.f && bDistance > 0.f)
			return false;
	}

	for (int iSidePlane = 0; iSidePlane < (int)m_sidePlanes.size(); ++iSidePlane)
	{
		const Plane3& sidePlane = m_sidePlanes[iSidePlane];

		for (int i = 0; i < 2; ++i)
		{
			Vector3& clipPt = (i == 0 ? inout_edge.m_a : inout_edge.m_b);
			Vector3& otherPt = (i == 0 ? inout_edge.m_b : inout_edge.m_a);

			if (sidePlane.GetDistanceFromPlane(clipPt) > 0.f)
			{
				// From the above check, this should be true
				ASSERT_OR_DIE(sidePlane.GetDistanceFromPlane(otherPt) <= 0.f, "Other end point wasn't inside the edge!");

				// New clip location will be where the segment crosses the edge plane
				Vector3 dir = (otherPt - clipPt);
				clipPt = SolveLinePlaneIntersection(Line3(clipPt, dir), sidePlane);
			}
		}	
	}

	return true;
}


//-------------------------------------------------------------------------------------------------
void PolyhedronFace::ClipFaceToFace(Polygon3& inout_FaceToClip) const
{
	int numVerts = (int)inout_FaceToClip.GetNumVertices();

	for (int iVertex = 0; iVertex < numVerts; ++iVertex)
	{
		int iNextVertex = (iVertex + 1) % numVerts;

		LineSegment3 edge(inout_FaceToClip.GetVertex(iVertex), inout_FaceToClip.GetVertex(iNextVertex));
		bool wasElgibleForClip = ClipEdgeToFace(edge);

		if (wasElgibleForClip)
		{
			inout_FaceToClip.SetVertex(iVertex, edge.m_a);
			inout_FaceToClip.SetVertex(iNextVertex, edge.m_b);
		}
	}
}


//-------------------------------------------------------------------------------------------------
LineSegment3 PolyhedronFace::GetEdgeInDirection(const Vector3& direction) const
{
	int bestIndex = -1;
	float bestDot = -1.f;

	for (int iSidePlane = 0; iSidePlane < (int)m_sidePlanes.size(); ++iSidePlane)
	{
		const Plane3& sidePlane = m_sidePlanes[iSidePlane];
		float dot = DotProduct(sidePlane.m_normal, direction);

		if (bestIndex == -1 || dot > bestDot)
		{
			bestDot = dot;
			bestIndex = iSidePlane;
		}
	}

	const HalfEdge* edge = m_poly->GetEdge(m_iHalfEdge);
	for (int iEdge = 0; iEdge < bestIndex; ++iEdge)
	{
		edge = m_poly->GetEdge(edge->m_nextEdgeIndex);
	}

	Vector3 a = m_poly->GetVertexPosition(edge->m_vertexIndex);
	Vector3 b = m_poly->GetVertexPosition(m_poly->GetEdge(edge->m_nextEdgeIndex)->m_vertexIndex);

	return LineSegment3(a, b);
}


////-------------------------------------------------------------------------------------------------
void PolyhedronFace::CalculateNormalAndSidePlanes()
{
	ASSERT_OR_DIE(m_indices.size() > 2, "Degenerate polyhedron face!");
	ASSERT_OR_DIE(m_iHalfEdge != -1, "Half edges not set up!");

	// Compute normal
	Vector3 a = m_poly->GetVertexPosition(m_indices[0]);
	Vector3 b = m_poly->GetVertexPosition(m_indices[1]);
	Vector3 c = m_poly->GetVertexPosition(m_indices[2]);
	m_normal = CalculateNormalForTriangle(a, b, c);

	// Build list of side planes
	m_sidePlanes.clear();
	int iStartingEdge = m_iHalfEdge;
	int iCurrEdge = iStartingEdge;

	do
	{
		Vector3 edgeDir = m_poly->GetEdgeDirection(iCurrEdge);
		Vector3 edgeNormal = CrossProduct(edgeDir, m_normal); // Outward pointing normal, assuming clockwise winding
		edgeNormal.Normalize();

		const HalfEdge* edge = m_poly->GetEdge(iCurrEdge);
		float d = DotProduct(edgeNormal, m_poly->GetVertexPosition(edge->m_vertexIndex));
		m_sidePlanes.push_back(Plane3(edgeNormal, d));

		iCurrEdge = edge->m_nextEdgeIndex;
	} 
	while (iCurrEdge != iStartingEdge);

	ASSERT_OR_DIE(m_sidePlanes.size() == m_indices.size(), "Edges and indices don't match up!");
}


//-------------------------------------------------------------------------------------------------
void PolyhedronFace::ApplyTransform(const Matrix4& transform)
{
	m_normal = transform.TransformDirection(m_normal);
	ASSERT_OR_DIE(AreMostlyEqual(m_normal.GetLength(), 1.0f), "Not normal!");

	int iCurrEdge = m_iHalfEdge;
	int numSidePlanes = (int)m_sidePlanes.size();
	for (int iSidePlane = 0; iSidePlane < numSidePlanes; ++iSidePlane)
	{
		Plane3& sidePlane = m_sidePlanes[iSidePlane];
		const HalfEdge* edge = m_poly->GetEdge(iCurrEdge);

		Vector3 newSideNormal = transform.TransformDirection(sidePlane.m_normal);
		ASSERT_OR_DIE(AreMostlyEqual(newSideNormal.GetLength(), 1.0f), "Not normal!");

		sidePlane = Plane3(newSideNormal, m_poly->GetVertexPosition(edge->m_vertexIndex));

		iCurrEdge = edge->m_nextEdgeIndex;
	}
}
