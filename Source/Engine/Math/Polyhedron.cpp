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
	PolyhedronFace back;
	back.m_indices.push_back(0);
	back.m_indices.push_back(1);
	back.m_indices.push_back(2);
	back.m_indices.push_back(3);
	m_faces.push_back(back);

	// Front
	PolyhedronFace front;
	front.m_indices.push_back(4);
	front.m_indices.push_back(5);
	front.m_indices.push_back(6);
	front.m_indices.push_back(7);
	m_faces.push_back(front);

	// Left
	PolyhedronFace left;
	left.m_indices.push_back(7);
	left.m_indices.push_back(6);
	left.m_indices.push_back(1);
	left.m_indices.push_back(0);
	m_faces.push_back(left);

	// Right
	PolyhedronFace right;
	right.m_indices.push_back(3);
	right.m_indices.push_back(2);
	right.m_indices.push_back(5);
	right.m_indices.push_back(4);
	m_faces.push_back(right);

	// Bottom
	PolyhedronFace bottom;
	bottom.m_indices.push_back(7);
	bottom.m_indices.push_back(0);
	bottom.m_indices.push_back(3);
	bottom.m_indices.push_back(4);
	m_faces.push_back(bottom);

	// Top
	PolyhedronFace top;
	top.m_indices.push_back(1);
	top.m_indices.push_back(6);
	top.m_indices.push_back(5);
	top.m_indices.push_back(2);
	m_faces.push_back(top);

	GenerateHalfEdgeStructure();
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
			face.m_halfEdgeIndex = currEdgeIndex;

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
		ASSERT_OR_DIE(m_faces[faceIndex].m_halfEdgeIndex != -1, "Invalid half edge!");
	}

	for (int vertexIndex = 0; vertexIndex < numVertices; ++vertexIndex)
	{
		ASSERT_OR_DIE(m_vertices[vertexIndex].m_halfEdgeIndex != -1, "Invalid half edge!");
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

	m_faces.push_back(PolyhedronFace(indices));
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
	out_polygon.Clear();

	int numVertices = (int)m_vertices.size();
	for (int vertexIndex = 0; vertexIndex < numVertices; ++vertexIndex)
	{
		Vector3 position = matrix.TransformPosition(m_vertices[vertexIndex].m_position);
		out_polygon.m_vertices.push_back(PolyhedronVertex(position, m_vertices[vertexIndex].m_halfEdgeIndex));
	}

	out_polygon.m_faces = m_faces;
	out_polygon.m_edges = m_edges;
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
	int startingEdge = GetFace(faceIndex)->m_halfEdgeIndex;
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
Vector3 Polyhedron::GetFaceNormal(int faceIndex) const
{
	const PolyhedronFace* face = GetFace(faceIndex);
	ASSERT_OR_DIE(face->m_indices.size() > 2, "Not enough vertices!");

	// Get the positions
	Vector3 a = GetVertexPosition(face->m_indices[0]);
	Vector3 b = GetVertexPosition(face->m_indices[1]);
	Vector3 c = GetVertexPosition(face->m_indices[2]);

	return CalculateNormalForTriangle(a, b, c);
}


//-------------------------------------------------------------------------------------------------
Plane3 Polyhedron::GetFaceSupportPlane(int faceIndex) const
{
	Vector3 normal = GetFaceNormal(faceIndex);

	// Get a position on the plane
	const PolyhedronFace* face = GetFace(faceIndex);
	Vector3 p = GetVertexPosition(face->m_indices[0]);

	// Get the distance between origin and plane
	float distance = DotProduct(normal, p);

	return Plane3(normal, distance);
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
				
				if (plane.GetDistanceFromPlane(vertex) > 0.000001f)
					return false;
			}
		}
	}

	return true;
}


//-------------------------------------------------------------------------------------------------
void Polyhedron::GetAllFacesAdjacentTo(int faceIndex, std::vector<const PolyhedronFace*>& out_faces) const
{
	const PolyhedronFace& face = m_faces[faceIndex];

	const HalfEdge* startingEdge = GetEdge(face.m_halfEdgeIndex);
	const HalfEdge* currEdge = startingEdge;

	do 
	{
		// Get my mirror, then the face my mirror points to
		const HalfEdge* mirrorEdge = GetEdge(currEdge->m_mirrorEdgeIndex);
		const PolyhedronFace* currFace = GetFace(mirrorEdge->m_faceIndex);

		bool alreadyIncluded = std::find(out_faces.begin(), out_faces.end(), currFace) != out_faces.end();

		if (!alreadyIncluded)
		{
			out_faces.push_back(currFace);
		}
		
		currEdge = GetEdge(currEdge->m_nextEdgeIndex);
	} 
	while (currEdge != startingEdge);
}


//-------------------------------------------------------------------------------------------------
void Polyhedron::GetAllSidePlanesForFace(int faceIndex, std::vector<Plane3>& out_planes) const
{
	out_planes.clear();

	const PolyhedronFace* refFace = GetFace(faceIndex);
	Vector3 refFaceNormal = GetFaceNormal(faceIndex);

	int startingEdgeIndex = refFace->m_halfEdgeIndex;
	int edgeIndex = startingEdgeIndex;

	do
	{
		Vector3 edgeDir = GetEdgeDirection(edgeIndex);
		Vector3 refEdgeNormal = CrossProduct(edgeDir, refFaceNormal); // Outward pointing normal, assuming clockwise winding
		refEdgeNormal.Normalize();

		const HalfEdge* edge = GetEdge(edgeIndex);
		float d = DotProduct(refEdgeNormal, GetVertexPosition(edge->m_vertexIndex));
		out_planes.push_back(Plane3(refEdgeNormal, d));

		edgeIndex = edge->m_nextEdgeIndex;
	} 
	while (edgeIndex != startingEdgeIndex);
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
