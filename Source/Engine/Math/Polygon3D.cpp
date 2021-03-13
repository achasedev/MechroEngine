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
#include "Engine/Math/Polygon3D.h"
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
void Polygon3D::Clear()
{
	m_vertices.clear();
	m_indices.clear();
	m_faceIndexCounts.clear();
}


//-------------------------------------------------------------------------------------------------
int Polygon3D::PushVertex(const Vector3& vertex)
{
	m_vertices.push_back(vertex);

	return (int)(m_vertices.size() - 1);
}


//-------------------------------------------------------------------------------------------------
void Polygon3D::PushIndex(int index)
{
	m_indices.push_back(index);
}


//-------------------------------------------------------------------------------------------------
void Polygon3D::PushIndicesForTriangle(int first, int second, int third)
{
	m_indices.push_back(first);
	m_indices.push_back(second);
	m_indices.push_back(third);
}


//-------------------------------------------------------------------------------------------------
void Polygon3D::PushFaceIndexCount(int faceIndexCount)
{
	m_faceIndexCounts.push_back(faceIndexCount);
}


//-------------------------------------------------------------------------------------------------
Vector3 Polygon3D::GetVertex(int vertexIndex) const
{
	return m_vertices[vertexIndex];
}


//-------------------------------------------------------------------------------------------------
int Polygon3D::GetIndex(int indexIndex) const
{
	return m_indices[indexIndex];
}


//-------------------------------------------------------------------------------------------------
Face3 Polygon3D::GetFace(int faceIndex) const
{
	int startingIndex = GetStartingIndexForFaceIndex(faceIndex);
	int numIndicesInFace = m_faceIndexCounts[faceIndex];

	Face3 face;
	
	for (int indiceIndex = startingIndex; indiceIndex < startingIndex + numIndicesInFace; ++indiceIndex)
	{
		int vertexIndex = m_indices[indiceIndex];
		Vector3 vertex = m_vertices[vertexIndex];

		face.AddVertex(vertex);
	}

	return face;
}


//-------------------------------------------------------------------------------------------------
int Polygon3D::GetFarthestVertexInDirection(const Vector3& direction, Vector3& out_vertex) const
{
	ASSERT_OR_DIE(m_vertices.size() > 0, "No vertices to return!");

	// Early out...but this shouldn't happen
	if (m_vertices.size() == 1U)
	{
		out_vertex = m_vertices[0];
		return 0;
	}

	float maxDot = -1.f;
	int bestIndex = -1;

	for (int vertexIndex = 0; vertexIndex < (int)m_vertices.size(); ++vertexIndex)
	{
		// Treat the vertex position as a vector from 0,0
		const Vector3& currVector = m_vertices[vertexIndex];

		float dot = DotProduct(currVector, direction);
		if (bestIndex == -1 || dot > maxDot)
		{
			maxDot = dot;
			bestIndex = vertexIndex;
		}
	}

	out_vertex = m_vertices[bestIndex];
	return bestIndex;
}


//-------------------------------------------------------------------------------------------------
Vector3 Polygon3D::GetCenter() const
{
	uint32 numVertices = (uint32)m_vertices.size();
	ASSERT_RETURN(numVertices > 0U, Vector3::ZERO, "Polygon3D has no vertices!");

	Vector3 average = Vector3::ZERO;
	for (uint32 i = 0; i < numVertices; ++i)
	{
		average += m_vertices[i];
	}

	average /= static_cast<float>(numVertices);

	return average;
}


//-------------------------------------------------------------------------------------------------
void Polygon3D::GetTransformed(const Matrix44& transformMatrix, Polygon3D& out_polygonWs) const
{
	// Convert local space vertices to world space
	for (int vertexIndex = 0; vertexIndex < (int)m_vertices.size(); ++vertexIndex)
	{
		Vector3 worldVertex = transformMatrix.TransformPoint(m_vertices[vertexIndex]).xyz();
		out_polygonWs.PushVertex(worldVertex);
	}

	// Keep the same indices and faces
	out_polygonWs.m_indices = m_indices;
	out_polygonWs.m_faceIndexCounts = m_faceIndexCounts;
}


//-------------------------------------------------------------------------------------------------
void Polygon3D::GetAllFacesAdjacentTo(int faceIndex, std::vector<Face3>& out_faces) const
{
	// Get the indices in this face
	out_faces.clear();
	int numIndicesInBaseFace = m_faceIndexCounts[faceIndex];
	int baseIndexOffset = GetStartingIndexForFaceIndex(faceIndex);
	int numFaces = GetNumFaces();

	for (int currFaceIndex = 0; currFaceIndex < numFaces; ++currFaceIndex)
	{
		if (currFaceIndex == faceIndex)
		{
			continue;
		}

		int numIndicesInCurrFace = m_faceIndexCounts[currFaceIndex];
		int currIndexOffset = GetStartingIndexForFaceIndex(currFaceIndex);

		bool faceAdded = false;
		for (int currIndexIndex = currIndexOffset; currIndexIndex <  currIndexOffset + numIndicesInCurrFace; ++currIndexIndex)
		{
			int currVertexIndex = m_indices[currIndexIndex];

			for (int baseIndexIndex = baseIndexOffset; baseIndexIndex < baseIndexOffset + numIndicesInBaseFace; ++baseIndexIndex)
			{
				int baseVertexIndex = m_indices[baseIndexIndex];

				if (baseVertexIndex == currVertexIndex)
				{
					out_faces.push_back(GetFace(currFaceIndex));
					faceAdded = true;
					break;
				}
			}

			if (faceAdded)
			{
				break;
			}
		}
	}
}


//-------------------------------------------------------------------------------------------------
void Polygon3D::DebugRender(Transform* transform, Material* material, const Rgba& color)
{
	Matrix44 model = Matrix44::IDENTITY;

	if (transform != nullptr)
	{
		model = transform->GetLocalToWorldMatrix();
	}

	Polygon3D worldShape;
	GetTransformed(model, worldShape);

	g_renderContext->DrawWirePolygon3D(worldShape, material, color);
}


//-------------------------------------------------------------------------------------------------
int Polygon3D::GetStartingIndexForFaceIndex(int faceIndex) const
{
	int startingIndex = 0;
	for (int indexCountIndex = 0; indexCountIndex < faceIndex; ++indexCountIndex)
	{
		startingIndex += m_faceIndexCounts[indexCountIndex];
	}

	return startingIndex;
}
