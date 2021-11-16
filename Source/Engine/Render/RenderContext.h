///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: December 8th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Render/Buffer/ConstantBuffer.h"
#include "Engine/Render/Mesh/Mesh.h"
#include "Engine/Core/EngineCommon.h"
#include "Engine/Render/Shader/Shader.h"
#include <string>

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#define ENGINE_RESERVED_CONSTANT_BUFFER_COUNT 8
#define MAX_RESOURCES_PER_SLOT (8)

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Camera;
class RenderTargetView;
class DepthStencilView;
class DrawCall;
class IndexBuffer;
class Light;
class Material;
class Matrix4;
class Mesh;
class OBB2;
class Plane3;
class Polygon2;
class Polygon3;
class Renderable;
class Sampler;
class Shader;
class ShaderResourceView;
class Texture2D;
class TextureCube;
class Transform;
class ConstantBuffer;
class VertexBuffer;
class VertexLayout;
struct ID3D11Debug;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11Texture2D;

enum ConstantBufferSlot : uint32
{
	CONSTANT_BUFFER_SLOT_FRAME_TIME = 0,
	CONSTANT_BUFFER_SLOT_CAMERA = 1,
	CONSTANT_BUFFER_SLOT_MODEL_MATRIX = 2,
	CONSTANT_BUFFER_SLOT_LIGHT = 3,
};

enum SamplerMode
{
	SAMPLER_MODE_POINT,
	SAMPLER_MODE_LINEAR,
	NUM_SAMPLER_MODES
};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class RenderContext
{
public:
	//-----Public Methods-----

	static void							Initialize();
	static void							Shutdown();
	
	void								BeginFrame();
	void								EndFrame();

	void								BeginCamera(Camera* camera);
	void								EndCamera();

	void								BindUniformBuffer(uint32 slot, ConstantBuffer* ubo);
	void								BindMaterial(Material* material);
	void								BindShader(Shader* shader);
	void								BindShaderResourceView(uint32 slot, ShaderResourceView* view);
	void								BindSampler(uint32 slot, Sampler* sampler);
	void								UpdateModelMatrixUBO(const Matrix4& modelMatrix);
	void								SetLightsForDrawCall(const DrawCall& drawCall);

	template <typename VERT_TYPE> void	DrawVertexArray(const VERT_TYPE* vertices, uint32 numVertices, MeshTopology topology = TOPOLOGY_TRIANGLE_LIST, const uint32* indices = nullptr, uint32 numIndices = 0, Material* material = nullptr);
	void								DrawMesh(Mesh& mesh);
	void								DrawMeshWithMaterial(Mesh& mesh, Material* material);
	void								DrawRenderable(Renderable& renderable);
	void								Draw(const DrawCall& drawCall);

	void								DrawPoint2D(const Vector2& position, float radius, Material* material, const Rgba& color = Rgba::WHITE);
	void								DrawPoint3D(const Vector3& position, float radius, const Rgba& color = Rgba::WHITE, Shader* shader = nullptr);
	void								DrawLine2D(const Vector2& start, const Vector2& end, Material* material, const Rgba& color = Rgba::WHITE);
	void								DrawLine3D(const Vector3& start, const Vector3& end, const Rgba& color = Rgba::WHITE, Shader* shader = nullptr);
	void								DrawWirePolygon2D(const Polygon2& polygon, Material* material, const Rgba& color = Rgba::WHITE);
	void								DrawWirePolygon3D(const Polygon3& polygon, const Rgba& color = Rgba::WHITE, Shader* shader = nullptr);
	void								DrawWireOBB2D(const OBB2& obb, Material* material, const Rgba& color = Rgba::WHITE);
	void								DrawPlane3(const Plane3& plane, const Rgba& color = Rgba::WHITE, Shader* shader = nullptr);
	void								DrawTransform(const Transform& transform, float scale, Shader* shader = nullptr);

	void								SaveTextureToImage(Texture2D* texture, const char* filepath);

	ID3D11Device*						GetDxDevice();
	ID3D11DeviceContext*				GetDxContext();
	IDXGISwapChain*						GetDxSwapChain();

	Texture2D*							GetDefaultRenderTarget() const { return m_defaultColorTarget; }
	Texture2D*							GetDefaultDepthStencilTarget() const { return m_defaultDepthStencil; }

	RenderTargetView*					GetDefaultColorTargetView() const;
	DepthStencilView*					GetDefaultDepthStencilView() const;

	bool								Event_WindowResize(NamedProperties& args);


private:
	//-----Private Methods-----

	RenderContext();
	~RenderContext();
	RenderContext(const RenderContext& copy) = delete;

	void		DxInit();
	void		PostDxInit();

	void		InitDefaultColorAndDepthViews();
	void		BindVertexStream(const VertexBuffer* vbo);
	void		BindIndexStream(const IndexBuffer* ibo);
	void		UpdateInputLayout(const VertexLayout* vertexLayout);


private:
	//-----Private Data-----

	ID3D11Device*				m_dxDevice = nullptr;
	ID3D11DeviceContext*		m_dxContext = nullptr;
	ID3D11Debug*				m_dxDebug = nullptr;
	IDXGISwapChain*				m_dxSwapChain = nullptr;

	// Frame State
	Camera*						m_currentCamera = nullptr;
	Shader*						m_currentShader = nullptr;
	ShaderInputLayout			m_lastInputLayout;
	Texture2D*					m_defaultColorTarget = nullptr;
	Texture2D*					m_defaultDepthStencil = nullptr;
	Mesh						m_immediateMesh;
	ConstantBuffer				m_modelMatrixUBO;
	ConstantBuffer				m_lightUBO;

	// Sampler
	SamplerMode					m_samplerMode = SAMPLER_MODE_LINEAR;
	Sampler*					m_samplers[NUM_SAMPLER_MODES];

};


//-------------------------------------------------------------------------------------------------
template <typename VERT_TYPE>
void RenderContext::DrawVertexArray(const VERT_TYPE* vertices, uint32 numVertices, MeshTopology topology /*= TOPOLOGY_TRIANGLE_LIST*/, const uint32* indices /*= nullptr*/, uint32 numIndices /*= 0*/, Material* material /*= nullptr*/)
{
	m_immediateMesh.SetVertices(vertices, numVertices);
	m_immediateMesh.SetIndices(indices, numIndices);

	bool useIndices = (indices != nullptr);
	DrawInstruction drawInstruction;
	drawInstruction.m_elementCount = (useIndices ? numIndices : numVertices);
	drawInstruction.m_useIndices = useIndices;
	drawInstruction.m_startIndex = 0;
	drawInstruction.m_topology = topology;
	m_immediateMesh.SetDrawInstruction(drawInstruction);

	DrawMeshWithMaterial(m_immediateMesh, material);
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
