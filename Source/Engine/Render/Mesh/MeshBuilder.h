///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: February 15th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Math/AABB2.h"
#include "Engine/Render/Mesh/Mesh.h"
#include "Engine/Render/Mesh/Vertex.h"
#include "Engine/UI/UIText.h"
#include <vector>

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Font;
class Mesh;
class Polygon3;
class Polyhedron;
class Rgba;
class Tetrahedron;
class Triangle2;
class Triangle3;
class Vector2;
class Vector3;
struct VertexMaster;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class MeshBuilder
{
public:
	//-----Public Methods-----

	MeshBuilder() {};
	~MeshBuilder();

	void	BeginBuilding(MeshTopology topology, bool useIndices);
	void	FinishBuilding();
	void	Clear();
	void	AssertBuildState(bool shouldBeBuilding, MeshTopology topology, bool* usingIndices = false);

	// VertexMaster Stamp
	void	SetColor(const Rgba& color);
	void	SetUV(const Vector2& uv);
	void	SetNormal(const Vector3& normal);
	void	SetTangent(const Vector4& tangent);
	void	GenerateFlatNormals();
	
	void	SetDrawInstruction(const DrawInstruction& instruction);

	uint32	PushVertex(const Vector3& position);
	uint32	PushVertex(const VertexMaster& master);
	void	PushIndex(uint32 index);

	// 2D Helpers
	void   PushTriangle2D(const Vector2& first, const Vector2& second, const Vector2& third, const Rgba& tint = Rgba::WHITE);
	void   PushQuad2D(const AABB2& quad, const AABB2& uvs = AABB2::ZERO_TO_ONE, const Rgba& tint = Rgba::WHITE);
	uint32 PushText(const std::vector<ColoredText>& textLines, uint32 pixelHeight, Font* font, const AABB2& textBounds, const Vector2& canvasUnitsPerPixel,
		HorizontalAlignment xAlign = ALIGNMENT_LEFT, VerticalAlignment yAlign = ALIGNMENT_TOP, TextDrawMode drawMode = TEXT_DRAW_DEFAULT, std::vector<std::vector<AABB2>>* out_glyphBounds = nullptr);
	uint32 PushText(const std::vector<std::string>& textLines, uint32 pixelHeight, Font* font, const AABB2& textBounds, const Vector2& canvasUnitsPerPixel, const Rgba& color = Rgba::WHITE,
		HorizontalAlignment xAlign = ALIGNMENT_LEFT, VerticalAlignment yAlign = ALIGNMENT_TOP, TextDrawMode drawMode = TEXT_DRAW_DEFAULT, std::vector<std::vector<AABB2>>* out_glyphBounds = nullptr);
	uint32 PushText(const char* text, uint32 pixelHeight, Font* font, const AABB2& textBounds, const Vector2& canvasUnitsPerPixel, const Rgba& color = Rgba::WHITE,
		HorizontalAlignment xAlign = ALIGNMENT_LEFT, VerticalAlignment yAlign = ALIGNMENT_TOP, TextDrawMode drawMode = TEXT_DRAW_DEFAULT, std::vector<std::vector<AABB2>>* out_glyphBounds = nullptr);

	// 3D Helpers
	void		PushLine3D(const Vector3& start, const Vector3& end, const Rgba& color = Rgba::WHITE);
	void		PushTriangle3(const Triangle3& triangle, const Rgba& tint = Rgba::WHITE);
	void		PushTriangle3(const Vector3& first, const Vector3& second, const Vector3& third, const Rgba& tint = Rgba::WHITE);
	void		PushQuad3D(const Vector3& bottomLeft, const Vector3& topLeft, const Vector3& topRight, const Vector3& bottomRight, const AABB2& uvs = AABB2::ZERO_TO_ONE, const Rgba& tint = Rgba::WHITE);
	void		PushQuad3D(const Vector3& position, const Vector2& dimensions, const AABB2& uvs = AABB2::ZERO_TO_ONE, const Rgba& tint = Rgba::WHITE, const Vector3& rightVector = Vector3::X_AXIS, const Vector3& upVector = Vector3::Y_AXIS, const Vector2& pivot = Vector2(0.5f, 0.5f));
	void		PushCube(const Vector3& center, const Vector3& dimensions, const AABB2& topUVs = AABB2::ZERO_TO_ONE, const AABB2& sideUVs = AABB2::ZERO_TO_ONE, const AABB2& bottomUVs = AABB2::ZERO_TO_ONE, const Rgba& tint = Rgba::WHITE);
	void		PushSphere(const Vector3& center, float radius, const Rgba& color = Rgba::WHITE, int numUSteps = 10, int numVSteps = 10);
	void		PushTopHemiSphere(const Vector3& center, float radius, const Rgba& color = Rgba::WHITE, int numUSteps = 10, int numVSteps = 10, float startV = 0.5f, float endV = 1.0f);
	void		PushTube(const Vector3& bottom, const Vector3& top, float radius, const Rgba& color = Rgba::WHITE, int numUSteps = 10, float startV = 0.f, float endV = 1.f);
	void		PushBottomHemiSphere(const Vector3& center, float radius, const Rgba& color = Rgba::WHITE, int numUSteps = 10, int numVSteps = 10, float startV = 0.f, float endV = 0.5f);
	void		PushCapsule(const Vector3& start, const Vector3& end, float radius, const Rgba& color = Rgba::WHITE);
	void		PushTetrahedron(const Tetrahedron& tetra, const Rgba& color = Rgba::WHITE);
	void		PushPolygon(const Polygon3& poly, const Rgba& color = Rgba::WHITE);
	void		PushPolyhedron(const Polyhedron& poly, const Rgba& color = Rgba::WHITE);
	void		PushDisc(const Vector3& center, float radius, const Vector3& normal, const Vector3& tangent, const Rgba& color = Rgba::WHITE, int numUSteps = 10, float startV = 0.f, float endV = (1.f / 3.f));
	void		PushCylinder(const Vector3& bottom, const Vector3& top, float radius, const Rgba& color = Rgba::WHITE, int numUSteps = 10);

	uint32		GetVertexCount() const { return (uint32)m_vertices.size(); }
	uint32		GetIndexCount() const { return (uint32)m_indices.size(); }

	//-------------------------------------------------------------------------------------------------
	template <typename VERT_TYPE>
	Mesh* CreateMesh() const
	{
		Mesh* mesh = new Mesh();
		UpdateMesh<VERT_TYPE>(*mesh);
	
		return mesh;
	}
	
	
	//-------------------------------------------------------------------------------------------------
	template <typename VERT_TYPE>
	void UpdateMesh(Mesh& out_mesh) const
	{
		ASSERT_OR_DIE(!m_isBuilding, "Finish building before creating/updating a mesh!");

		// Convert the list of VertexMasters to the specified vertex type
		uint32 vertexCount = (uint32)m_vertices.size();
		uint32 indexCount = (uint32)m_indices.size();
	
		ASSERT_OR_DIE(vertexCount > 0, "You're creating a mesh with no vertices! Don't do that.");
	
		if (m_instruction.m_useIndices)
		{
			ASSERT_OR_DIE(indexCount > 0, "You're creating an indexed mesh with no indices! Don't do that.");
		}
	
		VERT_TYPE* temp = (VERT_TYPE*)malloc(sizeof(VERT_TYPE) * vertexCount);
	
		for (uint32 vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
		{
			temp[vertexIndex] = VERT_TYPE(m_vertices[vertexIndex]);
		}
	
		// Set up the mesh
		out_mesh.SetVertices(temp, vertexCount);
		out_mesh.SetIndices(m_indices.data(), indexCount); // Will release index buffer if no indices used
		
	
		out_mesh.SetDrawInstruction(m_instruction);
	
		SAFE_FREE(temp);
	}


private:
	//-----Private Data-----

	bool						m_isBuilding = false;
	VertexMaster				m_stamp;
	DrawInstruction				m_instruction;
	std::vector<VertexMaster>	m_vertices;
	std::vector<uint32>			m_indices;

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
