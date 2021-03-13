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
#include "Engine/Math/Vector3.h"
#include <vector>

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Material;
class Transform;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class Polygon3D
{
public:
	//-----Public Methods-----


	Polygon3D() {}
	~Polygon3D() {}

	void		Clear();

	int			PushVertex(const Vector3& vertex);
	void		PushIndex(int index);
	void		PushIndicesForTriangle(int first, int second, int third);
	void		PushFaceIndexCount(int faceIndexCount);

	int			GetNumVertices() const { return (int)m_vertices.size(); }
	int			GetNumIndices() const { return (int)m_indices.size(); }
	int			GetNumFaces() const { return (int)m_faceIndexCounts.size(); }
	Vector3		GetVertex(int vertexIndex) const;
	int			GetIndex(int indexIndex) const;
	Face3		GetFace(int faceIndex) const;
	int			GetFarthestVertexInDirection(const Vector3& direction, Vector3& out_vertex) const;
	Vector3		GetCenter() const;
	void		GetTransformed(const Matrix44& transformMatrix, Polygon3D& out_polygonWs) const;
	void		GetAllFacesAdjacentTo(int faceIndex, std::vector<Face3>& out_faces) const;

	void		DebugRender(Transform* transform, Material* material, const Rgba& color);


private:
	//-----Private Methods-----

	int			GetStartingIndexForFaceIndex(int faceIndex) const;


private:
	//-----Private Data-----

	std::vector<Vector3>	m_vertices;
	std::vector<int>		m_indices;
	std::vector<int>		m_faceIndexCounts;

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
