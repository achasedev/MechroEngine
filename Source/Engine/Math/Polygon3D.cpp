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
		m_verticesLs.push_back(PolygonVertex3d(points[i]));
	}

	m_faces.resize(6);

	// Back
	PolygonFace3d& back = m_faces[0];
	back.m_indices.push_back(0);
	back.m_indices.push_back(1);
	back.m_indices.push_back(2);
	back.m_indices.push_back(3);
	back.m_owningPolygon = this;

	// Front
	PolygonFace3d& front = m_faces[1];
	front.m_indices.push_back(4);
	front.m_indices.push_back(5);
	front.m_indices.push_back(6);
	front.m_indices.push_back(7);
	front.m_owningPolygon = this;

	// Left
	PolygonFace3d& left = m_faces[2];
	left.m_indices.push_back(7);
	left.m_indices.push_back(6);
	left.m_indices.push_back(1);
	left.m_indices.push_back(0);
	left.m_owningPolygon = this;

	// Right
	PolygonFace3d& right = m_faces[3];
	right.m_indices.push_back(3);
	right.m_indices.push_back(2);
	right.m_indices.push_back(5);
	right.m_indices.push_back(4);
	right.m_owningPolygon = this;

	// Bottom
	PolygonFace3d& bottom = m_faces[4];
	bottom.m_indices.push_back(7);
	bottom.m_indices.push_back(0);
	bottom.m_indices.push_back(3);
	bottom.m_indices.push_back(4);
	bottom.m_owningPolygon = this;

	// Top
	PolygonFace3d& top = m_faces[5];
	top.m_indices.push_back(1);
	top.m_indices.push_back(6);
	top.m_indices.push_back(5);
	top.m_indices.push_back(2);
	top.m_owningPolygon = this;

	GenerateHalfEdgeStructure();
}


//-------------------------------------------------------------------------------------------------
void Polygon3d::Clear()
{
	m_verticesLs.clear();
	m_faces.clear();
	m_edges.clear();
}


//-------------------------------------------------------------------------------------------------
void Polygon3d::GenerateHalfEdgeStructure()
{
	int numFaces = (int)m_faces.size();
	int numVertices = (int)m_verticesLs.size();

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

			int firstIndex = face.m_indices[i];
			int secondIndex = face.m_indices[j];

			HalfEdge halfEdge;
			halfEdge.m_face = &m_faces[faceIndex];
			halfEdge.m_vertex = &m_verticesLs[firstIndex];

			HalfEdgeKey edgeKey(firstIndex, secondIndex);
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
			m_verticesLs[currIndex].m_outgoingHalfEdge = currHalfEdge;

			// Connect the next/prev edges
			// Get the next edge
			int k = (j + 1) % numIndices;
			int nextNextIndex = face.m_indices[k];
			HalfEdgeKey nextEdgeKey(nextIndex, nextNextIndex);
			HalfEdge* nextHalfEdge = &m_edges[nextEdgeKey];

			// Connect them
			currHalfEdge->m_nextEdge = nextHalfEdge;
			nextHalfEdge->m_prevEdge = currHalfEdge;

			// Connect mirror edges
			HalfEdgeKey mirrorEdgeKey(nextIndex, currIndex); // Flipped indices, since it's the mirror of this edge
			HalfEdge* mirrorHalfEdge = (m_edges.find(mirrorEdgeKey) != m_edges.end() ? &m_edges[mirrorEdgeKey] : nullptr);
			ASSERT_OR_DIE(mirrorHalfEdge != nullptr, "Mirror edge doesn't exist!");

			currHalfEdge->m_mirrorEdge = mirrorHalfEdge;
			mirrorHalfEdge->m_mirrorEdge = currHalfEdge;
		}
	}

	// Safety checks
	std::map<HalfEdgeKey, HalfEdge>::const_iterator itr = m_edges.begin();
	for (itr; itr != m_edges.end(); itr++)
	{
		ASSERT_OR_DIE(itr->second.m_mirrorEdge != nullptr, "Null mirror edge!");
		ASSERT_OR_DIE(itr->second.m_nextEdge != nullptr, "Null next edge!");
		ASSERT_OR_DIE(itr->second.m_prevEdge != nullptr, "Null prev edge!");
		ASSERT_OR_DIE(itr->second.m_vertex != nullptr, "Null vertex!");
		ASSERT_OR_DIE(itr->second.m_face != nullptr, "Null face!");
	}

	for (int faceIndex = 0; faceIndex < numFaces; ++faceIndex)
	{
		ASSERT_OR_DIE(m_faces[faceIndex].m_halfEdge != nullptr, "Null half edge!");
	}

	for (int vertexIndex = 0; vertexIndex < numVertices; ++vertexIndex)
	{
		ASSERT_OR_DIE(m_verticesLs[vertexIndex].m_outgoingHalfEdge != nullptr, "Null half edge!");
	}
}


//-------------------------------------------------------------------------------------------------
int Polygon3d::AddVertex(const Vector3& vertex)
{
	ASSERT_OR_DIE(!HasGeneratedHalfEdges(), "Cannot edit a Polygon3d after half edges are generated!");

	PolygonVertex3d polyVertex(vertex);
	m_verticesLs.push_back(polyVertex);

	return (int)(m_verticesLs.size() - 1);
}


//-------------------------------------------------------------------------------------------------
int Polygon3d::AddFace(const PolygonFace3d& face)
{
	ASSERT_OR_DIE(!HasGeneratedHalfEdges(), "Cannot edit a Polygon3d after half edges are generated!");

	m_faces.push_back(face);
	m_faces.back().m_owningPolygon = this;

	return (int)(m_faces.size() - 1);
}


//-------------------------------------------------------------------------------------------------
Vector3 Polygon3d::GetLocalVertex(int vertexIndex) const
{
	return m_verticesLs[vertexIndex].m_position;
}


//-------------------------------------------------------------------------------------------------
Vector3 Polygon3d::GetWorldVertex(int vertexIndex) const
{
	return m_transform.TransformPoint(m_verticesLs[vertexIndex].m_position);
}


//-------------------------------------------------------------------------------------------------
const PolygonFace3d* Polygon3d::GetFace(int faceIndex) const
{
	return &m_faces[faceIndex];
}


//-------------------------------------------------------------------------------------------------
// Stores the positions in the Face3 directly, maintaining no reference to the original polygon
Face3 Polygon3d::GetLocalFaceInstance(int faceIndex) const
{
	const PolygonFace3d& polyFace = m_faces[faceIndex];
	int numIndices = (int)polyFace.m_indices.size();

	Face3 face;

	for (int indiceIndex = 0; indiceIndex < numIndices; ++indiceIndex)
	{
		int vertexIndex = polyFace.m_indices[indiceIndex];
		Vector3 vertex = m_verticesLs[vertexIndex].m_position;

		face.AddVertex(vertex);
	}

	return face;
}


//-------------------------------------------------------------------------------------------------
int Polygon3d::GetFarthestLocalVertexInDirection(const Vector3& direction, Vector3& out_vertex) const
{
	ASSERT_OR_DIE(m_verticesLs.size() > 0, "No vertices to return!");

	// Early out...but this shouldn't happen
	if (m_verticesLs.size() == 1U)
	{
		out_vertex = m_verticesLs[0].m_position;
		return 0;
	}

	float maxDot = -1.f;
	int bestIndex = -1;

	for (int vertexIndex = 0; vertexIndex < (int)m_verticesLs.size(); ++vertexIndex)
	{
		// Treat the vertex position as a vector from 0,0
		const Vector3& currVector = m_verticesLs[vertexIndex].m_position;

		float dot = DotProduct(currVector, direction);
		if (bestIndex == -1 || dot > maxDot)
		{
			maxDot = dot;
			bestIndex = vertexIndex;
		}
	}

	out_vertex = m_verticesLs[bestIndex].m_position;
	return bestIndex;
}


//-------------------------------------------------------------------------------------------------
int Polygon3d::GetFarthestWorldVertexInDirection(const Vector3& directionWs, Vector3& out_vertex) const
{
	// Transform the direction into local space
	Vector3 localDirection = m_transform.InverseTransformDirection(directionWs);
	int bestIndex = GetFarthestLocalVertexInDirection(localDirection, out_vertex);

	// Transform this vertex into world space
	out_vertex = m_transform.TransformPoint(out_vertex);

	return bestIndex;
}


//-------------------------------------------------------------------------------------------------
Vector3 Polygon3d::GetCenter() const
{
	uint32 numVertices = (uint32)m_verticesLs.size();
	ASSERT_RETURN(numVertices > 0U, Vector3::ZERO, "Polygon3D has no vertices!");

	Vector3 average = Vector3::ZERO;
	for (uint32 i = 0; i < numVertices; ++i)
	{
		average += m_verticesLs[i].m_position;
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
		const PolygonFace3d* currFace = currEdge->m_mirrorEdge->m_face;
		bool alreadyIncluded = std::find(out_faces.begin(), out_faces.end(), currFace) != out_faces.end();

		if (!alreadyIncluded)
		{
			out_faces.push_back(currFace);
		}
		
		currEdge = currEdge->m_nextEdge;
	} 
	while (currEdge != startingEdge);
}


//-------------------------------------------------------------------------------------------------
PolygonFace3d::PolygonFace3d(const std::vector<int> indices, const Polygon3d* owningPolygon)
	: m_indices(indices)
	, m_owningPolygon(owningPolygon)
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
Vector3 PolygonFace3d::GetVertex(int vertexIndex) const
{
	ASSERT_OR_DIE(m_owningPolygon != nullptr, "Face doesn't belong to a polygon!");

	return m_owningPolygon->GetWorldVertex(m_indices[vertexIndex]);
}


//-------------------------------------------------------------------------------------------------
Plane3 PolygonFace3d::GetSupportPlane() const
{
	Vector3 normal = GetNormal();

	// Get a position on the plane
	Vector3 a = m_owningPolygon->GetWorldVertex(m_indices[0]);

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
