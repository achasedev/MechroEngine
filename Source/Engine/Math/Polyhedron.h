///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 23rd, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Math/Face3.h"
#include "Engine/Math/Transform.h"
#include "Engine/Math/Vector3.h"
#include <map>
#include <vector>

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class OBB3;

//-------------------------------------------------------------------------------------------------
struct PolyhedronVertex
{
	PolyhedronVertex() {}

	PolyhedronVertex(const Vector3& position)
		: m_position(position) {}

	PolyhedronVertex(const Vector3& position, int halfEdgeIndex)
		: m_position(position), m_halfEdgeIndex(halfEdgeIndex) {}

	Vector3	m_position;
	int 	m_halfEdgeIndex = -1;
};


//-------------------------------------------------------------------------------------------------
struct PolyhedronFace
{
	PolyhedronFace() {}
	PolyhedronFace(const std::vector<int> indices)
		: m_indices(indices) {}


	std::vector<int>	m_indices;
	int					m_halfEdgeIndex = -1;
};


//-------------------------------------------------------------------------------------------------
struct HalfEdge
{
	int m_edgeIndex = -1;
	int	m_mirrorEdgeIndex = -1;
	int	m_nextEdgeIndex = -1;
	int	m_prevEdgeIndex = -1;
	int	m_vertexIndex = -1;
	int	m_faceIndex = -1;
};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class Polyhedron
{
public:
	//-----Public Methods-----


	Polyhedron() {}
	~Polyhedron() {}
	Polyhedron(const OBB3& box);

	void					Clear();
	void					GenerateHalfEdgeStructure();
	
	int						AddVertex(const Vector3& vertex);
	int						AddFace(const std::vector<int>& indices);

	// Vertices
	int						GetNumVertices() const { return (int)m_vertices.size(); }
	const PolyhedronVertex*	GetVertex(int vertexIndex) const;
	Vector3					GetVertexPosition(int vertexIndex) const;
	void					GetAllVerticesInFace(int faceIndex, std::vector<Vector3>& out_vertices) const;
	int						GetSupportPoint(const Vector3& direction, Vector3& out_vertex) const;

	// Faces
	int						GetNumFaces() const { return (int)m_faces.size(); }
	const PolyhedronFace*	GetFace(int faceIndex) const;
	int						GetIndexOfFaceMostInDirection(const Vector3& direction) const;
	Vector3					GetFaceNormal(int faceIndex) const;
	Plane3					GetFaceSupportPlane(int faceIndex) const;
	void					GetAllFacesAdjacentTo(int faceIndex, std::vector<const PolyhedronFace*>& out_faces) const;
	void					GetAllSidePlanesForFace(int faceIndex, std::vector<Plane3>& out_planes) const;

	// Edges
	int						GetNumEdges() const { return (int)m_edges.size(); }
	const HalfEdge*			GetEdge(int edgeIndex) const;
	Vector3					GetEdgeDirection(int edgeIndex) const;
	Vector3					GetEdgeDirection(const HalfEdge* edge) const;
	void					GetEdgeEndPoints(int edgeIndex, Vector3& out_start, Vector3& out_end) const;
	Vector3					GetEdgeDirectionNormalized(int edgeIndex) const;
	bool					HasGeneratedHalfEdges() const { return m_edges.size() > 0; }

	// General
	void					GetTransformed(const Matrix4& matrix, Polyhedron& out_polygon) const;
	Vector3					GetCenter() const;
	bool					IsConvex() const;
	bool					IsConcave() const { return !IsConvex(); }


private:
	//-----Private Data-----

	std::vector<PolyhedronVertex>	m_vertices;
	std::vector<PolyhedronFace>		m_faces;

	// Additional formatting on the "soup" data above for better traversal
	std::vector<HalfEdge>			m_edges;

};


//-------------------------------------------------------------------------------------------------
class UniqueHalfEdgeIterator
{
public:
	//-----Public Methods-----

	UniqueHalfEdgeIterator(const Polyhedron& polygon);

	const HalfEdge* GetNext();


private:
	//-----Private Data-----

	const Polyhedron&		m_polygon;
	int					m_currIndex = 0;
	std::vector<int>	m_visitedList;

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
