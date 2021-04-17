///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: December 8th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Render/Buffer/UniformBuffer.h"
#include "Engine/Render/Mesh/Mesh.h"
#include "Engine/Framework/EngineCommon.h"
#include <string>

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Camera;
class RenderTargetView;
class DepthStencilTargetView;
class DrawCall;
class IndexBuffer;
class Material;
class Matrix44;
class Mesh;
class OBB2;
class Plane3;
class Polygon2D;
class Polygon3d;
class Renderable;
class Sampler;
class Shader;
class ShaderResourceView;
class Texture2D;
class Transform;
class UniformBuffer;
class VertexBuffer;
class VertexLayout;
struct ID3D11Debug;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11Texture2D;

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

	void								ClearScreen(const Rgba& color);
	void								ClearDepth(float depthValue = 1.0f);

	void								BindUniformBuffer(uint32 slot, UniformBuffer* ubo);
	void								BindMaterial(Material* material);
	void								BindShader(Shader* shader);
	void								BindShaderResourceView(uint32 slot, ShaderResourceView* view);
	void								BindSampler(uint32 slot, Sampler* sampler);
	void								UpdateModelMatrixUBO(const Matrix44& modelMatrix);

	template <typename VERT_TYPE> void	DrawVertexArray(const VERT_TYPE* vertices, uint32 numVertices, const uint32* indices = nullptr, uint32 numIndices = 0, Material* material = nullptr);
	void								DrawMesh(Mesh& mesh);
	void								DrawMeshWithMaterial(Mesh& mesh, Material* material);
	void								DrawRenderable(Renderable& renderable);
	void								Draw(const DrawCall& drawCall);
	
	void								DrawPoint2D(const Vector2& position, float radius, Material* material, const Rgba& color = Rgba::WHITE);
	void								DrawPoint3D(const Vector3& position, float radius, Material* material, const Rgba& color = Rgba::WHITE);
	void								DrawLine2D(const Vector2& start, const Vector2& end, Material* material, const Rgba& color = Rgba::WHITE);
	void								DrawLine3D(const Vector3& start, const Vector3& end, Material* material, const Rgba& color = Rgba::WHITE);
	void								DrawWirePolygon2D(const Polygon2D& polygon, Material* material, const Rgba& color = Rgba::WHITE);
	void								DrawWirePolygon3D(const Polygon3d& polygon, Material* material, const Rgba& color = Rgba::WHITE);
	void								DrawWireOBB2D(const OBB2& obb, Material* material, const Rgba& color = Rgba::WHITE);
	void								DrawPlane3(const Plane3& plane, Material* material, const Rgba& color = Rgba::WHITE);
	void								DrawTransform(Transform& transform, Material* material, float scale);

	void								SaveTextureToImage(Texture2D* texture, const char* filepath);

	ID3D11Device*						GetDxDevice();
	ID3D11DeviceContext*				GetDxContext();
	IDXGISwapChain*						GetDxSwapChain();

	Texture2D*							GetDefaultRenderTarget() const { return m_defaultColorTarget; }
	Texture2D*							GetDefaultDepthStencilTarget() const { return m_defaultDepthStencilTarget; }

	RenderTargetView*					GetDefaultRenderTargetView() const;
	DepthStencilTargetView*				GetDefaultDepthStencilTargetView() const;

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

	ID3D11Device*			m_dxDevice = nullptr;
	ID3D11DeviceContext*	m_dxContext = nullptr;
	ID3D11Debug*			m_dxDebug = nullptr;
	IDXGISwapChain*			m_dxSwapChain = nullptr;

	// Frame State
	Camera*					m_currentCamera = nullptr;
	Shader*					m_currentShader = nullptr;
	const VertexLayout*		m_currVertexLayout = nullptr;
	Texture2D*				m_defaultColorTarget = nullptr;
	Texture2D*				m_defaultDepthStencilTarget = nullptr;
	Mesh					m_immediateMesh;
	UniformBuffer			m_modelMatrixUBO;

	// Sampler
	SamplerMode				m_samplerMode = SAMPLER_MODE_POINT;
	Sampler*				m_samplers[NUM_SAMPLER_MODES];

};


//-------------------------------------------------------------------------------------------------
template <typename VERT_TYPE>
void RenderContext::DrawVertexArray(const VERT_TYPE* vertices, uint32 numVertices, const uint32* indices /*= nullptr*/, uint32 numIndices /*= 0*/, Material* material /*= nullptr*/)
{
	m_immediateMesh.SetVertices(vertices, numVertices);
	m_immediateMesh.SetIndices(indices, numIndices);

	bool useIndices = (indices != nullptr);
	DrawInstruction drawInstruction;
	drawInstruction.m_elementCount = (useIndices ? numIndices : numVertices);
	drawInstruction.m_useIndices = useIndices;
	drawInstruction.m_startIndex = 0;
	m_immediateMesh.SetDrawInstruction(drawInstruction);

	if (material == nullptr)
	{
		DrawMesh(m_immediateMesh);
	}
	else
	{
		DrawMeshWithMaterial(m_immediateMesh, material);
	}
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
