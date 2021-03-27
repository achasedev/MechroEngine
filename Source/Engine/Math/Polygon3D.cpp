///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 25th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Math/Matrix44.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/OBB3.h"
#include "Engine/Math/Polygon3d.h"
#include "Engine/Math/Transform.h"
#include "Engine/Render/Core/RenderContext.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------
const HalfEdgeKey Polygon3d::INVALID_HALFEDGE_KEY = HalfEdgeKey(-1, -1);

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
Polygon3d::Polygon3d(const OBB3& box)
{
	Vector3 points[8];
	box.GetPoints(points);

	for (int i = 0; i < 8; ++i)
	{
		m_vertices.push_back(PolygonVertex3d(points[i]));
	}

	// Back
	PolygonFace3d back(*this);
	back.m_indices.push_back(0);
	back.m_indices.push_back(1);
	back.m_indices.push_back(2);
	back.m_indices.push_back(3);
	m_faces.push_back(back);

	// Front
	PolygonFace3d front(*this);
	front.m_indices.push_back(4);
	front.m_indices.push_back(5);
	front.m_indices.push_back(6);
	front.m_indices.push_back(7);
	m_faces.push_back(front);

	// Left
	PolygonFace3d left(*this);
	left.m_indices.push_back(7);
	left.m_indices.push_back(6);
	left.m_indices.push_back(1);
	left.m_indices.push_back(0);
	m_faces.push_back(left);

	// Right
	PolygonFace3d right(*this);
	right.m_indices.push_back(3);
	right.m_indices.push_back(2);
	right.m_indices.push_back(5);
	right.m_indices.push_back(4);
	m_faces.push_back(right);

	// Bottom
	PolygonFace3d bottom(*this);
	bottom.m_indices.push_back(7);
	bottom.m_indices.push_back(0);
	bottom.m_indices.push_back(3);
	bottom.m_indices.push_back(4);
	m_faces.push_back(bottom);

	// Top
	PolygonFace3d top(*this);
	top.m_indices.push_back(1);
	top.m_indices.push_back(6);
	top.m_indices.push_back(5);
	top.m_indices.push_back(2);
	m_faces.push_back(top);

	GenerateHalfEdgeStructure();
}


//-------------------------------------------------------------------------------------------------
void Polygon3d::Clear()
{
	m_vertices.clear();
	m_faces.clear();
	m_edges.clear();
}


//-------------------------------------------------------------------------------------------------
void Polygon3d::GenerateHalfEdgeStructure()
{
	int numFaces = (int)m_faces.size();
	int numVertices = (int)m_vertices.size();

	ASSERT_OR_DIE(numVertices > 0, "No vertices!");
	ASSERT_OR_DIE(numFaces > 0, "No faces!");
	ASSERT_OR_DIE(!HasGeneratedHalfEdges(), "Edges already generated!");

	for (int faceIndex = 0; faceIndex < numFaces; ++faceIndex)
	{
		PolygonFace3d& face = m_faces[faceIndex];
		int numIndices = (int)face.m_indices.size();
		ASSERT_OR_DIE(numIndices > 2, "Not enough indices in face!");

		for (int i = 0; i < numIndices; ++i)
		{
			int j = (i + 1) % numIndices;

			int vertexIndex1 = face.m_indices[i];
			int vertexIndex2 = face.m_indices[j];


			HalfEdge halfEdge(this);
			halfEdge.m_faceIndex = faceIndex;
			halfEdge.m_vertexIndex = vertexIndex1;

			HalfEdgeKey edgeKey(vertexIndex1, vertexIndex2);
			m_edges[edgeKey] = halfEdge;
		}
	}

	// Now that all possible half edges are created, we can begin linking them
	// Otherwise the map may shift elements around, invalidating all pointers
	for (int faceIndex = 0; faceIndex < numFaces; ++faceIndex)
	{
		PolygonFace3d& face = m_faces[faceIndex];
		int numIndices = (int)face.m_indices.size();

		// Connect the half edges to their mirrored edges
		for (int i = 0; i < numIndices; ++i)
		{
			int j = (i + 1) % numIndices;

			int currIndex = face.m_indices[i];
			int nextIndex = face.m_indices[j];

			// Connect the half edges to their mirrors
			HalfEdgeKey currEdgeKey(currIndex, nextIndex);

			HalfEdge* currHalfEdge = &m_edges[currEdgeKey];

			if (i == 0)
			{
				// Connect this face to the first half edge
				face.m_halfEdge = currHalfEdge;
			}

			// Connect this vertex to any half edge going out of it
			// Since multiple faces share this edge, it will get set multiple times, which is fine, so long as it's set
			m_vertices[currIndex].m_outgoingHalfEdge = currHalfEdge;

			// Connect the next/prev edges
			// Get the next edge
			int k = (j + 1) % numIndices;
			int nextNextIndex = face.m_indices[k];
			HalfEdgeKey nextEdgeKey(nextIndex, nextNextIndex);
			HalfEdge* nextHalfEdge = &m_edges[nextEdgeKey];

			// Connect them
			currHalfEdge->m_nextEdgeKey = nextEdgeKey;
			nextHalfEdge->m_prevEdgeKey = currEdgeKey;

			// Connect mirror edges
			HalfEdgeKey mirrorEdgeKey(nextIndex, currIndex); // Flipped indices, since it's the mirror of this edge
			HalfEdge* mirrorHalfEdge = (m_edges.find(mirrorEdgeKey) != m_edges.end() ? &m_edges[mirrorEdgeKey] : nullptr);
			ASSERT_OR_DIE(mirrorHalfEdge != nullptr, "Mirror edge doesn't exist!");

			currHalfEdge->m_mirrorEdgeKey = mirrorEdgeKey;
			mirrorHalfEdge->m_mirrorEdgeKey = currEdgeKey;
		}
	}

	// Safety checks
	std::map<HalfEdgeKey, HalfEdge>::const_iterator itr = m_edges.begin();
	for (itr; itr != m_edges.end(); itr++)
	{
		ASSERT_OR_DIE(itr->second.m_mirrorEdgeKey != INVALID_HALFEDGE_KEY, "Invalid mirror edge!");
		ASSERT_OR_DIE(itr->second.m_nextEdgeKey != INVALID_HALFEDGE_KEY, "Invalid next edge!");
		ASSERT_OR_DIE(itr->second.m_prevEdgeKey != INVALID_HALFEDGE_KEY, "Invalid prev edge!");
		ASSERT_OR_DIE(itr->second.m_vertexIndex != -1, "Invalid vertex!");
		ASSERT_OR_DIE(itr->second.m_faceIndex != -1, "Invalid face!");
	}

	for (int faceIndex = 0; faceIndex < numFaces; ++faceIndex)
	{
		ASSERT_OR_DIE(m_faces[faceIndex].m_halfEdge != nullptr, "Null half edge!");
	}

	for (int vertexIndex = 0; vertexIndex < numVertices; ++vertexIndex)
	{
		ASSERT_OR_DIE(m_vertices[vertexIndex].m_outgoingHalfEdge != nullptr, "Null half edge!");
	}
}


//-------------------------------------------------------------------------------------------------
int Polygon3d::AddVertex(const Vector3& vertex)
{
	ASSERT_OR_DIE(!HasGeneratedHalfEdges(), "Cannot edit a Polygon3d after half edges are generated!");

	PolygonVertex3d polyVertex(vertex);
	m_vertices.push_back(polyVertex);

	return (int)(m_vertices.size() - 1);
}


//-------------------------------------------------------------------------------------------------
int Polygon3d::AddFace(const std::vector<int>& indices)
{
	ASSERT_OR_DIE(!HasGeneratedHalfEdges(), "Cannot edit a Polygon3d after half edges are generated!");

	m_faces.push_back(PolygonFace3d(*this, indices));
	return (int)(m_faces.size() - 1);
}


//-------------------------------------------------------------------------------------------------
const PolygonVertex3d* Polygon3d::GetVertex(int vertexIndex) const
{
	return &m_vertices[vertexIndex];
}


//-------------------------------------------------------------------------------------------------
void Polygon3d::GetTransformed(const Matrix44& matrix, Polygon3d& out_polygon) const
{
	out_polygon.Clear();

	int numVertices = (int)m_vertices.size();
	for (int vertexIndex = 0; vertexIndex < numVertices; ++vertexIndex)
	{
		Vector3 position = matrix.TransformPoint(m_vertices[vertexIndex].m_position).xyz();
		out_polygon.m_vertices.push_back(PolygonVertex3d(position));
	}

	int numFaces = (int)m_faces.size();
	for (int faceIndex = 0; faceIndex < numFaces; ++faceIndex)
	{
		out_polygon.m_faces.push_back(PolygonFace3d(out_polygon, m_faces[faceIndex].m_indices));
	}

	if (HasGeneratedHalfEdges())
	{
		out_polygon.GenerateHalfEdgeStructure();
	}
}


//-------------------------------------------------------------------------------------------------
Vector3 Polygon3d::GetVertexPosition(int vertexIndex) const
{
	return m_vertices[vertexIndex].m_position;
}


//-------------------------------------------------------------------------------------------------
const PolygonFace3d* Polygon3d::GetFace(int faceIndex) const
{
	return &m_faces[faceIndex];
}


//-------------------------------------------------------------------------------------------------
const HalfEdge* Polygon3d::GetEdge(const HalfEdgeKey& edgeKey) const
{
	ASSERT_OR_DIE(HasGeneratedHalfEdges(), "No edges!");

	bool edgeExists = m_edges.find(edgeKey) != m_edges.end();
	ASSERT_OR_DIE(edgeExists, "Cannot find edge!");

	return &m_edges.at(edgeKey);
}


//-------------------------------------------------------------------------------------------------
int Polygon3d::GetSupportPoint(const Vector3& direction, Vector3& out_vertex) const
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
Vector3 Polygon3d::GetCenter() const
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
void Polygon3d::GetAllFacesAdjacentTo(int faceIndex, std::vector<const PolygonFace3d*>& out_faces) const
{
	const PolygonFace3d& face = m_faces[faceIndex];

	const HalfEdge* startingEdge = face.m_halfEdge;
	const HalfEdge* currEdge = startingEdge;

	do 
	{
		// Get my mirror, then the face my mirror points to
		const PolygonFace3d* currFace = currEdge->GetMirrorEdge()->GetFace();
		bool alreadyIncluded = std::find(out_faces.begin(), out_faces.end(), currFace) != out_faces.end();

		if (!alreadyIncluded)
		{
			out_faces.push_back(currFace);
		}
		
		currEdge = currEdge->GetNextEdge();
	} 
	while (currEdge != startingEdge);
}


//-------------------------------------------------------------------------------------------------
PolygonFace3d::PolygonFace3d(const Polygon3d& polygon)
	: m_owningPolygon(polygon)
{
}


//-------------------------------------------------------------------------------------------------
PolygonFace3d::PolygonFace3d(const Polygon3d& polygon, const std::vector<int> indices)
	: m_owningPolygon(polygon)
	, m_indices(indices)
{
}


//-------------------------------------------------------------------------------------------------
void PolygonFace3d::AddIndex(int vertexIndex)
{
	bool alreadyExists = std::find(m_indices.begin(), m_indices.end(), vertexIndex) != m_indices.end();
	ASSERT_OR_DIE(!alreadyExists, "Duplicate index!");
	
	m_indices.push_back(vertexIndex);
}


//-------------------------------------------------------------------------------------------------
void PolygonFace3d::SetIndices(const std::vector<int>& indices)
{
	m_indices = indices;
}


//-------------------------------------------------------------------------------------------------
Vector3 PolygonFace3d::GetVertex(int vertexIndex) const
{
	return m_owningPolygon.GetVertexPosition(m_indices[vertexIndex]);
}


//-------------------------------------------------------------------------------------------------
Plane3 PolygonFace3d::GetSupportPlane() const
{
	Vector3 normal = GetNormal();

	// Get a position on the plane
	Vector3 a = m_owningPolygon.GetVertexPosition(m_indices[0]);

	// Get the distance between origin and plane
	float distance = DotProduct(normal, a);

	return Plane3(normal, distance);
}


//-------------------------------------------------------------------------------------------------
Vector3 PolygonFace3d::GetNormal() const
{
	ASSERT_OR_DIE(m_indices.size() >= 3, "Cannot get the plane without at least 3 points!");

	// Get the positions
	Vector3 a = GetVertex(0);
	Vector3 b = GetVertex(1);
	Vector3 c = GetVertex(2);

	return CalculateNormalForTriangle(a, b, c);
}


//-------------------------------------------------------------------------------------------------
UniqueHalfEdgeIterator::UniqueHalfEdgeIterator(const Polygon3d& polygon)
	: m_polygon(polygon)
{
	m_edgeIter = m_polygon.m_edges.begin();
}


//-------------------------------------------------------------------------------------------------
const HalfEdge* UniqueHalfEdgeIterator::GetNext()
{
	while (m_visitedList.size() < m_polygon.m_edges.size())
	{
		// Get the next edge
		const HalfEdge* currEdge = &m_edgeIter->second;
		m_edgeIter++;

		// Check if this edge has already been visited...which should never happen, half edges should uniquely exist
#ifndef DISABLE_ASSERTS
		ASSERT_OR_DIE(std::find(m_visitedList.begin(), m_visitedList.end(), currEdge) != m_visitedList.end(), "Duplicate edge!");
#endif

		// Check if this edge's mirror has been visited already
		bool mirrorVisited = std::find(m_visitedList.begin(), m_visitedList.end(), currEdge->GetMirrorEdge()) != m_visitedList.end();

		if (!mirrorVisited)
		{
			m_visitedList.push_back(currEdge);
			m_visitedList.push_back(currEdge->GetMirrorEdge());
			return currEdge;
		}
	}

	return nullptr;
}


//-------------------------------------------------------------------------------------------------
Vector3 HalfEdge::GetStartPosition() const
{
	return m_owningPolygon->GetVertexPosition(m_vertexIndex);
}


//-------------------------------------------------------------------------------------------------
Vector3 HalfEdge::GetEndPosition() const
{
	const HalfEdge* nextEdge = m_owningPolygon->GetEdge(m_nextEdgeKey);
	return nextEdge->GetStartPosition();
}


//-------------------------------------------------------------------------------------------------
Vector3 HalfEdge::GetAsVector() const
{
	return (GetEndPosition() - GetStartPosition());
}


//-------------------------------------------------------------------------------------------------
Vector3 HalfEdge::GetAsNormalizedVector() const
{
	return GetAsVector().GetNormalized();
}
