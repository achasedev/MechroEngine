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
class Material;
class OBB3;
class Polygon3d;
class PolygonFace3d;
class Transform;
struct PolygonVertex3d;


//-------------------------------------------------------------------------------------------------
struct HalfEdge
{
	HalfEdge*			m_mirrorEdge = nullptr;
	HalfEdge*			m_nextEdge = nullptr;
	HalfEdge*			m_prevEdge = nullptr;
	PolygonVertex3d*	m_vertex = nullptr;
	PolygonFace3d*		m_face = nullptr;
};
typedef std::pair<int, int> HalfEdgeKey;


//-------------------------------------------------------------------------------------------------
struct PolygonVertex3d
{
	PolygonVertex3d() {}
	explicit PolygonVertex3d(const Vector3& position)
		: m_position(position) {}

	Vector3		m_position;
	HalfEdge*	m_outgoingHalfEdge = nullptr;
};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class PolygonFace3d
{
public:
	//-----Public Methods-----

	PolygonFace3d() {}
	PolygonFace3d(const std::vector<int> indices, const Polygon3d* owningPolygon);

	void	AddIndex(int vertexIndex);

	Vector3 GetVertex(int vertexIndex) const;
	Vector3 GetNormal() const;
	Plane3	GetSupportPlane() const;
	int		GetNumVertices() const { return (int)m_indices.size(); }


public:
	//-----Public Data-----

	std::vector<int>	m_indices;
	const Polygon3d*	m_owningPolygon = nullptr;
	HalfEdge*			m_halfEdge = nullptr;

};

//-------------------------------------------------------------------------------------------------
class Polygon3d
{
	friend class UniqueHalfEdgeIterator;

public:
	//-----Public Methods-----


	Polygon3d() {}
	~Polygon3d() {}
	Polygon3d(const OBB3& box);

	void					Clear();
	void					GenerateHalfEdgeStructure();

	int						AddVertex(const Vector3& vertex);
	int						AddFace(const PolygonFace3d& face);

	int						GetNumVertices() const { return (int)m_verticesLs.size(); }
	int						GetNumFaces() const { return (int)m_faces.size(); }
	bool					HasGeneratedHalfEdges() const { return m_edges.size() > 0; }
	Vector3					GetLocalVertex(int vertexIndex) const;
	Vector3					GetWorldVertex(int vertexIndex) const;
	const PolygonFace3d*	GetFace(int faceIndex) const;
	Face3					GetLocalFaceInstance(int faceIndex) const;
	Face3					GetWorldFaceInstance(int faceIndex) const;
	int						GetFarthestLocalVertexInDirection(const Vector3& direction, Vector3& out_vertex) const;
	int						GetFarthestWorldVertexInDirection(const Vector3& direction, Vector3& out_vertex) const;
	Vector3					GetCenter() const;
	void					GetAllFacesAdjacentTo(int faceIndex, std::vector<const PolygonFace3d*>& out_faces) const;


public:
	//-----Public Data-----

	mutable Transform				m_transform;


private:
	//-----Private Data-----

	std::vector<PolygonVertex3d>	m_verticesLs;
	std::vector<PolygonFace3d>		m_faces;

	// Additional formatting on the "soup" data above for better traversal
	std::map<HalfEdgeKey, HalfEdge>	m_edges;

};


class UniqueHalfEdgeIterator
{
public:
	//-----Public Methods-----

	explicit UniqueHalfEdgeIterator(const Polygon3d& polygon);

	const HalfEdge* GetNext();


private:
	//-----Private Data-----

	const Polygon3d&								m_polygon;
	std::map<HalfEdgeKey, HalfEdge>::const_iterator m_edgeIter;
	std::vector<const HalfEdge*>					m_visitedList;

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
