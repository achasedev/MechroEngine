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
class HalfEdge;
class Material;
class OBB3;
class Polygon3d;
class PolygonFace3d;
class Transform;
struct PolygonVertex3d;
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

	PolygonFace3d(const Polygon3d& polygon);
	PolygonFace3d(const Polygon3d& polygon, const std::vector<int> indices);

	void	AddIndex(int vertexIndex);
	void	SetIndices(const std::vector<int>& indices);

	Vector3 GetVertex(int vertexIndex) const;
	Vector3 GetNormal() const;
	Plane3	GetSupportPlane() const;
	int		GetNumVertices() const { return (int)m_indices.size(); }


public:
	//-----Public Data-----

	std::vector<int>	m_indices;
	const Polygon3d&	m_owningPolygon;
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
	int						AddFace(const std::vector<int>& indices);

	const PolygonVertex3d*	GetVertex(int vertexIndex) const;
	const PolygonFace3d*	GetFace(int faceIndex) const;
	const HalfEdge*			GetEdge(const HalfEdgeKey& edgeKey) const;
	Vector3					GetVertexPosition(int vertexIndex) const;

	void					GetTransformed(const Matrix44& matrix, Polygon3d& out_polygon) const;
	int						GetNumVertices() const { return (int)m_vertices.size(); }
	int						GetNumFaces() const { return (int)m_faces.size(); }
	bool					HasGeneratedHalfEdges() const { return m_edges.size() > 0; }
	int						GetSupportPoint(const Vector3& direction, Vector3& out_vertex) const;
	Vector3					GetCenter() const;
	void					GetAllFacesAdjacentTo(int faceIndex, std::vector<const PolygonFace3d*>& out_faces) const;


private:
	//-----Private Data-----

	std::vector<PolygonVertex3d>	m_vertices;
	std::vector<PolygonFace3d>		m_faces;

	// Additional formatting on the "soup" data above for better traversal
	std::map<HalfEdgeKey, HalfEdge>	m_edges;

	static const HalfEdgeKey INVALID_HALFEDGE_KEY;

};


//-------------------------------------------------------------------------------------------------
class HalfEdge
{
	friend class Polygon3d;

public:
	//-----Public Methods-----

	HalfEdge() {}
	HalfEdge(const Polygon3d* owningPolygon)
		: m_owningPolygon(owningPolygon) {}

	const HalfEdge*			GetMirrorEdge() const { return m_owningPolygon->GetEdge(m_mirrorEdgeKey); }
	const HalfEdge*			GetNextEdge() const { return m_owningPolygon->GetEdge(m_nextEdgeKey); }
	const HalfEdge*			GetPrevEdge() const { return m_owningPolygon->GetEdge(m_prevEdgeKey); }
	const PolygonFace3d*	GetFace() const { return m_owningPolygon->GetFace(m_faceIndex); }
	const PolygonVertex3d*	GetPolyVertex() const { return m_owningPolygon->GetVertex(m_vertexIndex); }

	Vector3 GetStartPosition() const;
	Vector3 GetEndPosition() const;
	Vector3 GetAsVector() const;
	Vector3 GetAsNormalizedVector() const;


private:
	//-----Private Data-----

	HalfEdgeKey				m_mirrorEdgeKey;
	HalfEdgeKey				m_nextEdgeKey;
	HalfEdgeKey				m_prevEdgeKey;
	int						m_vertexIndex = -1;
	int						m_faceIndex = -1;
	const Polygon3d*		m_owningPolygon = nullptr;

};


//-------------------------------------------------------------------------------------------------
class UniqueHalfEdgeIterator
{
public:
	//-----Public Methods-----

	UniqueHalfEdgeIterator(const Polygon3d& polygon);

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
