///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: December 8th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Event/EventSystem.h"
#include "Engine/Core/DevConsole.h"
#include "Engine/Core/Window.h"
#include "Engine/IO/File.h"
#include "Engine/IO/Image.h"
#include "Engine/IO/InputSystem.h"
#include "Engine/Job/EngineJobs.h"
#include "Engine/Job/JobSystem.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/OBB2.h"
#include "Engine/Math/Polygon2.h"
#include "Engine/Math/Polygon3.h"
#include "Engine/Render/Buffer/ConstantBuffer.h"
#include "Engine/Render/Buffer/PropertyBlockDescription.h"
#include "Engine/Render/Buffer/VertexBuffer.h"
#include "Engine/Render/Camera.h"
#include "Engine/Render/DrawCall.h"
#include "Engine/Render/DX11Common.h"
#include "Engine/Render/Renderable.h"
#include "Engine/Render/RenderContext.h"
#include "Engine/Render/Material/Material.h"
#include "Engine/Render/Material/MaterialPropertyBlock.h"
#include "Engine/Render/Mesh/Mesh.h"
#include "Engine/Render/Mesh/MeshBuilder.h"
#include "Engine/Render/Mesh/Vertex.h"
#include "Engine/Render/Sampler.h"
#include "Engine/Render/Shader/Shader.h"
#include "Engine/Render/Texture/Texture2DArray.h"
#include "Engine/Render/Texture/TextureCubeArray.h"
#include "Engine/Render/Texture/TextureCube.h"
#include "Engine/Render/View/RenderTargetView.h"
#include "Engine/Render/View/ShaderResourceView.h"
#include "Engine/Render/View/DepthStencilView.h"
#include "Engine/Render/Texture/Texture2D.h"
#include "Engine/Resource/ResourceSystem.h"
#include "Engine/Time/Time.h"
#include "Engine/Utility/NamedProperties.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

struct FrameTimeBufferData
{
	float m_deltaTimeSeconds;
	float m_totalTime;
	float m_padding0;
	float m_padding1;
};

struct LightBufferData
{
	Vector4		m_ambience = Vector4::ONES;	// xyz color, w intensity
	LightData	m_lights[MAX_NUMBER_OF_LIGHTS];
};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------
RenderContext* g_renderContext = nullptr;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
static D3D_PRIMITIVE_TOPOLOGY ToDxTopology(MeshTopology topoology)
{
	switch (topoology)
	{
	case TOPOLOGY_TRIANGLE_LIST: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST; break;
	case TOPOLOGY_LINE_LIST: return D3D11_PRIMITIVE_TOPOLOGY_LINELIST; break;
	default:
		ERROR_RECOVERABLE("Invalid topology!")
			return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		break;
	}
}


//-------------------------------------------------------------------------------------------------
// TODO remove this
static void SetupMaterial(Texture2D* albedo, Shader* shader, Material& out_material)
{
	ShaderResourceView* albedoView = (albedo != nullptr ? albedo->CreateOrGetShaderResourceView() : nullptr);

	if (albedoView == nullptr)
	{
		albedoView = g_resourceSystem->CreateOrGetTexture2D("white")->CreateOrGetShaderResourceView();
	}

	if (shader == nullptr)
	{
		shader = g_resourceSystem->CreateOrGetShader("Data/Shader/default_alpha.shader");
	}

	out_material.SetAlbedoTextureView(albedoView);
	out_material.SetShader(shader);
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
void RenderContext::Initialize()
{
	ASSERT_OR_DIE(g_renderContext == nullptr, "RenderContext is already initialized!");
	g_renderContext = new RenderContext();

	g_renderContext->DxInit();
	g_renderContext->PostDxInit();

	g_eventSystem->SubscribeEventCallbackObjectMethod("window-resize", &RenderContext::Event_WindowResize, *g_renderContext);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::Shutdown()
{
	g_eventSystem->UnsubscribeEventCallbackObjectMethod("window-resize", &RenderContext::Event_WindowResize, *g_renderContext);
	SAFE_DELETE(g_renderContext);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::BeginFrame()
{
	// No need to get the new backbuffer, since DX11 maintains the handle we have to
	// whatever is the current backbuffer
	
	// Clear color target
	Vector4 blackColor = Vector4(0.f, 0.f, 0.f, 1.0f);
	ID3D11RenderTargetView* dxRTV = nullptr;
	dxRTV = m_defaultColorTarget->CreateOrGetColorTargetView()->GetDxHandle();
	m_dxContext->ClearRenderTargetView(dxRTV, blackColor.data);

	// Clear depth stencil
	ID3D11DepthStencilView* dxView = nullptr;
	dxView = m_defaultDepthStencil->CreateOrGetDepthStencilView()->GetDxHandle();
	m_dxContext->ClearDepthStencilView(dxView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0U);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::EndFrame()
{
	// Copy our default color target to the back buffer
	ID3D11Texture2D* backbuffer = nullptr;
	m_dxSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backbuffer);

	m_dxContext->CopyResource(backbuffer, m_defaultColorTarget->GetDxHandle());

	// Swap buffers
	m_dxSwapChain->Present(0, 0);

	// If screenshot was pressed
	if (g_inputSystem != nullptr && g_inputSystem->WasKeyJustPressed(InputSystem::KEYBOARD_F9))
	{
		CreateDirectoryA("Data/Screenshots", NULL);

		std::string latestPath = "Data/Screenshots/Latest.png";
		std::string timestampPath = Stringf("Data/Screenshots/Screenshot_%s.png", GetFormattedSystemDateAndTime().c_str()).c_str();

		g_renderContext->SaveTextureToImage(g_renderContext->GetDefaultRenderTarget(), latestPath.c_str());
		g_renderContext->SaveTextureToImage(g_renderContext->GetDefaultRenderTarget(), timestampPath.c_str());
		ConsolePrintf(Rgba::WHITE, 5.f, "Screenshot saved to %s and %s", latestPath.c_str(), timestampPath.c_str());
		ConsoleLogf(Rgba::WHITE, "Screenshot saved to %s and %s", latestPath.c_str(), timestampPath.c_str());
	}
}


//-------------------------------------------------------------------------------------------------
void RenderContext::BeginCamera(Camera* camera)
{
	m_currentCamera = camera;

	// Render to the camera's target
	RenderTargetView* colorView = camera->GetColorTargetView();
	ID3D11RenderTargetView* rtv = (colorView != nullptr ? colorView->GetDxHandle() : nullptr);
	
	DepthStencilView* depthView = camera->GetDepthStencilTargetView();
	ID3D11DepthStencilView* dsv = (depthView != nullptr ? depthView->GetDxHandle() : nullptr);

	if (rtv == nullptr && dsv == nullptr)
	{
		ConsoleLogErrorf("Bound a camera with a nullptr color and depth.");
	}

	m_dxContext->OMSetRenderTargets(1, &rtv, dsv);

	// Use color view for dimensions, if none use depth, if none use default
	TextureView* viewForDimensions = m_defaultColorTarget->CreateOrGetColorTargetView();

	if (colorView != nullptr)
	{
		viewForDimensions = colorView;
	}
	else if (depthView != nullptr)
	{
		viewForDimensions = depthView;
	}

	// Viewport
	D3D11_VIEWPORT viewport;
	memset(&viewport, 0, sizeof(viewport));
	viewport.TopLeftX = 0U;
	viewport.TopLeftY = 0U;
	viewport.Width = (FLOAT)viewForDimensions->GetWidth();
	viewport.Height = (FLOAT)viewForDimensions->GetHeight();
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	m_dxContext->RSSetViewports(1, &viewport);

	// Uniform Buffer
	camera->UpdateUBO();
	BindUniformBuffer(CONSTANT_BUFFER_SLOT_CAMERA, camera->GetUniformBuffer());
}


//-------------------------------------------------------------------------------------------------
void RenderContext::EndCamera()
{
	m_dxContext->OMSetRenderTargets(0, nullptr, nullptr);
	m_currentCamera = nullptr;
}


//-------------------------------------------------------------------------------------------------
void RenderContext::BindUniformBuffer(uint32 slot, ConstantBuffer* ubo)
{
	ID3D11Buffer *buffer = (ubo != nullptr) ? ubo->GetDxHandle() : nullptr;
	m_dxContext->VSSetConstantBuffers(slot, 1U, &buffer);
	m_dxContext->PSSetConstantBuffers(slot, 1U, &buffer);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::BindMaterial(Material* material)
{
	if (material == nullptr)
	{
		material = g_resourceSystem->CreateOrGetMaterial("Data/Material/default.material");
	}

	ShaderResourceView* albedoView = material->GetShaderResourceView(SRV_SLOT_ALBEDO);
	BindShaderResourceView(SRV_SLOT_ALBEDO, albedoView);
	

	ShaderResourceView* normalView = material->GetShaderResourceView(SRV_SLOT_NORMAL);	
	BindShaderResourceView(SRV_SLOT_NORMAL, normalView);
	

	// Bind Shader
	BindShader(material->GetShader());

	// Bind property blocks (constant buffers)
	int numBlocks = material->GetPropertyBlockCount();
	
	for (int i = 0; i < numBlocks; ++i)
	{
		MaterialPropertyBlock* block = material->GetPropertyBlockAtIndex(i);

		// Make sure GPU is up-to-date
		block->UpdateGPUData();

		// Make sure block has valid bind slots
		int bindSlot = block->GetDescription()->GetBindSlot();
		if (bindSlot < ENGINE_RESERVED_CONSTANT_BUFFER_COUNT)
		{
			ConsoleLogErrorf("Material %s has a property block with a reserved bind slot!", material->GetResourceID().ToString());
		}
		else
		{
			BindUniformBuffer(block->GetDescription()->GetBindSlot(), block->GetConstantBuffer());
		}
	}
}


//-------------------------------------------------------------------------------------------------
void RenderContext::BindShader(Shader* shader)
{
	if (shader == nullptr)
	{
		shader = g_resourceSystem->CreateOrGetShader("Data/Shader/default_alpha.shader");
		ASSERT_OR_DIE(shader != nullptr, "Default shader missing!");
	}

	if (m_currentShader != shader || m_currentShader->IsDirty())
	{
		ID3D11VertexShader* dxVShader = shader->GetDxVertexStage();
		ID3D11PixelShader* dxPShader = shader->GetDxFragmentStage();

		m_dxContext->VSSetShader(dxVShader, 0, 0);
		m_dxContext->PSSetShader(dxPShader, 0, 0);

		shader->UpdateBlendState();
		float black[] = { 0.f, 0.f, 0.f, 0.f };
		m_dxContext->OMSetBlendState(shader->GetDxBlendState(), black, 0xFFFFFFFF);

		shader->UpdateRasterizerState();
		m_dxContext->RSSetState(shader->GetDxRasterizerState());

		shader->UpdateDepthState();
		m_dxContext->OMSetDepthStencilState(shader->GetDxDepthState(), 1);

		m_currentShader = shader;
	}
}


//-------------------------------------------------------------------------------------------------
void RenderContext::BindShaderResourceView(uint32 slot, ShaderResourceView* view)
{
	if (view != nullptr)
	{
		BindSampler(slot, view->GetSampler());
	}

	ID3D11ShaderResourceView* dxViewHandle = (view != nullptr ? view->GetDxHandle() : nullptr);
	m_dxContext->PSSetShaderResources(slot, 1U, &dxViewHandle);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::BindSampler(uint32 slot, Sampler* sampler)
{
	if (sampler == nullptr)
	{
		sampler = m_samplers[m_samplerMode];
	}

	sampler->CreateOrUpdate();

	ID3D11SamplerState* handle = sampler->GetDxSamplerState();
	m_dxContext->PSSetSamplers(slot, 1U, &handle);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::UpdateModelMatrixUBO(const Matrix4& modelMatrix)
{
	m_modelMatrixUBO.CopyToGPU(&modelMatrix, sizeof(Matrix4));
}


//-------------------------------------------------------------------------------------------------
// Updates the light constant buffer to have the given light information
void RenderContext::SetLightsForDrawCall(const DrawCall& drawCall)
{
	Rgba ambience = drawCall.GetAmbience();
	int numLights = drawCall.GetNumLights();

	LightBufferData data;
	data.m_ambience = ambience.GetAsFloats();

	for (int i = 0; i < MAX_NUMBER_OF_LIGHTS; ++i)
	{
		if (i < numLights)
		{
			data.m_lights[i] = drawCall.GetLight(i)->GetLightData();
		}
		else
		{
			// Disable all unused lights by turning their intensity to 0
			data.m_lights[i].m_color.w = 0.f;
			data.m_lights[i].m_attenuation = Vector3(1.f, 0.f, 0.f);
		}
	}

	m_lightUBO.CopyToGPU(&data, sizeof(LightBufferData));

	// Bind shadow textures
	BindShaderResourceView(SRV_SLOT_CONE_DIR_SHADOWMAP, drawCall.GetConeDirectionalShadowMaps()->CreateOrGetShaderResourceView());
	BindShaderResourceView(SRV_SLOT_POINT_SHADOWMAP, drawCall.GetPointLightShadowMaps()->CreateOrGetShaderResourceView());
}


//-------------------------------------------------------------------------------------------------
void RenderContext::DrawMesh(Mesh& mesh)
{
	DrawMeshWithMaterial(mesh, g_resourceSystem->CreateOrGetMaterial("Data/Material/default_alpha.material"));
}


//-------------------------------------------------------------------------------------------------
void RenderContext::DrawMeshWithMaterial(Mesh& mesh, Material* material)
{
	Renderable immediateRenderable;
	immediateRenderable.AddDraw(&mesh, material);

	DrawRenderable(immediateRenderable);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::DrawRenderable(Renderable& renderable)
{
	uint32 numDraws = renderable.GetNumDrawCalls();
	ASSERT_RECOVERABLE(numDraws > 0, "Renderable drawn with no draws!");

	for (uint32 drawIndex = 0; drawIndex < numDraws; ++drawIndex)
	{
		DrawCall dc;
		dc.SetFromRenderable(renderable, drawIndex);

		if (dc.GetMesh() != nullptr)
		{
			Draw(dc);
		}
		else
		{
			ConsoleLogErrorf("Draw attempted with a null mesh!");
		}
	}
}


//-------------------------------------------------------------------------------------------------
void RenderContext::Draw(const DrawCall& drawCall)
{
	BindMaterial(drawCall.GetMaterial());
	
	Mesh* mesh = drawCall.GetMesh();
	ASSERT_OR_DIE(mesh != nullptr, "Attempting to draw null mesh!");

	// Mesh
	BindVertexStream(mesh->GetVertexBuffer());
	BindIndexStream(mesh->GetIndexBuffer());
	UpdateInputLayout(mesh->GetVertexLayout());
	m_dxContext->IASetPrimitiveTopology(ToDxTopology(mesh->GetDrawInstruction().m_topology));

	UpdateModelMatrixUBO(drawCall.GetModelMatrix());

	// Update light constant buffer
	if (drawCall.GetMaterial()->UsesLights())
	{
		SetLightsForDrawCall(drawCall);
	}

	DrawInstruction draw = mesh->GetDrawInstruction();
	if (draw.m_useIndices)
	{
		m_dxContext->DrawIndexed(draw.m_elementCount, draw.m_startIndex, 0);
	}
	else
	{
		m_dxContext->Draw(draw.m_elementCount, draw.m_startIndex);
	}
}


//-------------------------------------------------------------------------------------------------
void RenderContext::DrawPoint2D(const Vector2& position, float radius, Material* material, const Rgba& color /*= Rgba::WHITE*/)
{
	std::vector<Vertex3D_PCU> vertices;

	Vector3 n	= Vector3(position + radius * Vector2::Y_AXIS, 0.f);
	Vector3 ne	= Vector3(position + radius * Vector2::ONES, 0.f);
	Vector3 e	= Vector3(position + radius * Vector2::X_AXIS, 0.f);
	Vector3 se	= Vector3(position + radius * Vector2(1.0f, -1.0f), 0.f);
	Vector3 s	= Vector3(position + radius * Vector2::MINUS_Y_AXIS, 0.f);
	Vector3 sw	= Vector3(position + radius * Vector2(-1.0f, -1.0f), 0.f);
	Vector3 w	= Vector3(position + radius * Vector2::MINUS_X_AXIS, 0.f);
	Vector3 nw	= Vector3(position + radius * Vector2(-1.0f, 1.0f), 0.f);

	vertices.push_back(Vertex3D_PCU(n, color, Vector2::ZERO));
	vertices.push_back(Vertex3D_PCU(s, color, Vector2::ZERO));
	vertices.push_back(Vertex3D_PCU(e, color, Vector2::ZERO));
	vertices.push_back(Vertex3D_PCU(w, color, Vector2::ZERO));
	vertices.push_back(Vertex3D_PCU(nw, color, Vector2::ZERO));
	vertices.push_back(Vertex3D_PCU(se, color, Vector2::ZERO));
	vertices.push_back(Vertex3D_PCU(sw, color, Vector2::ZERO));
	vertices.push_back(Vertex3D_PCU(ne, color, Vector2::ZERO));

	DrawVertexArray(vertices.data(), (uint32)vertices.size(), TOPOLOGY_LINE_LIST, nullptr, 0U, material);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::DrawPoint3D(const Vector3& position, float radius, const Rgba& color /*= Rgba::WHITE*/, Shader* shader /*= nullptr*/)
{
	std::vector<Vertex3D_PCU> vertices;

	Vector3 up = position + (Vector3::Y_AXIS * radius);
	Vector3 down = position - (Vector3::Y_AXIS * radius);
	Vector3 right = position + (Vector3::X_AXIS * radius);
	Vector3 left = position - (Vector3::X_AXIS * radius);
	Vector3 front = position + (Vector3::Z_AXIS * radius);
	Vector3 back = position - (Vector3::Z_AXIS * radius);

	vertices.push_back(Vertex3D_PCU(up, color, Vector2::ZERO));
	vertices.push_back(Vertex3D_PCU(down, color, Vector2::ZERO));
	vertices.push_back(Vertex3D_PCU(left, color, Vector2::ZERO));
	vertices.push_back(Vertex3D_PCU(right, color, Vector2::ZERO));
	vertices.push_back(Vertex3D_PCU(front, color, Vector2::ZERO));
	vertices.push_back(Vertex3D_PCU(back, color, Vector2::ZERO));

	Material material;
	SetupMaterial(nullptr, shader, material);

	DrawVertexArray(vertices.data(), (uint32)vertices.size(), TOPOLOGY_LINE_LIST, nullptr, 0U, &material);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::DrawLine2D(const Vector2& start, const Vector2& end, Material* material, const Rgba& color /*= Rgba::WHITE*/)
{
	std::vector<Vertex3D_PCU> vertices;

	Vector3 start3D = Vector3(start, 0.f);
	Vector3 end3D = Vector3(end, 0.f);

	vertices.push_back(Vertex3D_PCU(start3D, color, Vector2::ZERO));
	vertices.push_back(Vertex3D_PCU(end3D, color, Vector2::ZERO));

	DrawVertexArray(vertices.data(), (uint32)vertices.size(), TOPOLOGY_LINE_LIST, nullptr, 0U, material);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::DrawLine3D(const Vector3& start, const Vector3& end, const Rgba& color /*= Rgba::WHITE*/, Shader* shader /*= nullptr*/)
{
	std::vector<Vertex3D_PCU> vertices;

	vertices.push_back(Vertex3D_PCU(start, color, Vector2::ZERO));
	vertices.push_back(Vertex3D_PCU(end, color, Vector2::ZERO));

	Material material;
	SetupMaterial(nullptr, shader, material);

	DrawVertexArray(vertices.data(), (uint32)vertices.size(), TOPOLOGY_LINE_LIST, nullptr, 0U, &material);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::DrawWirePolygon2D(const Polygon2& polygon, Material* material, const Rgba& color /*= Rgba::WHITE*/)
{
	uint32 numVertices = polygon.GetNumVertices();
	std::vector<Vertex3D_PCU> vertices;

	for (uint32 i = 0; i < numVertices; ++i)
	{
		Vector3 position = Vector3(polygon.GetVertexAtIndex(i), 0.f);
		vertices.push_back(Vertex3D_PCU(position, color, Vector2::ZERO));

		uint32 nextIndex = ((i == (numVertices - 1U)) ? 0 : i + 1U);
		Vector3 nextPosition = Vector3(polygon.GetVertexAtIndex(nextIndex), 0.f);
		vertices.push_back(Vertex3D_PCU(nextPosition, color, Vector2::ZERO));
	}

	DrawVertexArray(vertices.data(), (uint32)vertices.size(), TOPOLOGY_LINE_LIST, nullptr, 0U, material);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::DrawWirePolygon3D(const Polygon3& polygon, const Rgba& color /*= Rgba::WHITE*/, Shader* shader /*= nullptr*/)
{
	int numFaces = polygon.GetNumFaces();
	ASSERT_RETURN(numFaces > 0, NO_RETURN_VAL, "No Faces!");

	std::vector<Vertex3D_PCU> vertices;

	for (int faceIndex = 0; faceIndex < numFaces; ++faceIndex)
	{
		const PolygonFace3d* face = polygon.GetFace(faceIndex);

		int numVertsInFace = (int)face->m_indices.size();

		for (int faceVertexIndex = 0; faceVertexIndex < numVertsInFace; ++faceVertexIndex)
		{
			int nextVertexIndex = (faceVertexIndex + 1) % numVertsInFace;

			Vector3 pos1 = polygon.GetVertexPosition(face->m_indices[faceVertexIndex]);
			Vector3 pos2 = polygon.GetVertexPosition(face->m_indices[nextVertexIndex]);

			vertices.push_back(Vertex3D_PCU(pos1, color, Vector2::ZERO));
			vertices.push_back(Vertex3D_PCU(pos2, color, Vector2::ZERO));
		}
	}

	Material material;
	SetupMaterial(nullptr, shader, material);

	DrawVertexArray(vertices.data(), (uint32)vertices.size(), TOPOLOGY_LINE_LIST, nullptr, 0U, &material);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::DrawWireOBB2D(const OBB2& obb, Material* material, const Rgba& color /*= Rgba::WHITE*/)
{
	Vector2 positions[4];
	obb.GetPositions(positions);

	std::vector<Vertex3D_PCU> vertices;

	for (uint32 i = 0; i < 4; ++i)
	{
		Vector3 position = Vector3(positions[i], 0.f);

		uint32 nextIndex = (i + 1) % 4;
		Vector3 nextPosition = Vector3(positions[nextIndex], 0.f);

		vertices.push_back(Vertex3D_PCU(position, color, Vector2::ZERO));
		vertices.push_back(Vertex3D_PCU(nextPosition, color, Vector2::ZERO));
	}

	DrawVertexArray(vertices.data(), (uint32)vertices.size(), TOPOLOGY_LINE_LIST, nullptr, 0U, material);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::DrawPlane3(const Plane3& plane, const Rgba& color /*= Rgba::WHITE*/, Shader* shader /*= nullptr*/)
{
	MeshBuilder mb;
	mb.BeginBuilding(TOPOLOGY_TRIANGLE_LIST, true);

	Vector3 position = plane.GetNormal() * plane.GetDistance();
	Vector3 right = CrossProduct(Vector3::Y_AXIS, plane.GetNormal());
	Vector3 up = CrossProduct(plane.GetNormal(), right);

	mb.PushQuad3D(position, Vector2(5.f), AABB2::ZERO_TO_ONE, color, right, up);
	mb.PushQuad3D(position, Vector2(5.f), AABB2::ZERO_TO_ONE, color, -1.0f * right, up);
	mb.FinishBuilding();
	mb.UpdateMesh<Vertex3D_PCU>(m_immediateMesh);

	Material material;
	SetupMaterial(nullptr, shader, material);

	DrawMeshWithMaterial(m_immediateMesh, &material);

	// Draw the normal
	DrawLine3D(position, position + plane.GetNormal(), color, shader);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::DrawTransform(const Transform& transform, float scale, Shader* shader /*= nullptr*/)
{
	std::vector<Vertex3D_PCU> vertices;

	Vector3 position = transform.GetWorldPosition();
	Vector3 right = transform.GetIVector();
	Vector3 up = transform.GetJVector();
	Vector3 forward = transform.GetKVector();

	vertices.push_back(Vertex3D_PCU(position, Rgba::RED, Vector2::ZERO));
	vertices.push_back(Vertex3D_PCU(position + right * scale, Rgba::RED, Vector2::ZERO));
	vertices.push_back(Vertex3D_PCU(position, Rgba::GREEN, Vector2::ZERO));
	vertices.push_back(Vertex3D_PCU(position + up * scale, Rgba::GREEN, Vector2::ZERO));
	vertices.push_back(Vertex3D_PCU(position, Rgba::BLUE, Vector2::ZERO));
	vertices.push_back(Vertex3D_PCU(position + forward * scale, Rgba::BLUE, Vector2::ZERO));

	Material material;
	SetupMaterial(nullptr, shader, material);

	DrawVertexArray(vertices.data(), static_cast<uint32>(vertices.size()), TOPOLOGY_LINE_LIST, nullptr, 0, &material);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::SaveTextureToImage(Texture2D* texture, const char* filepath)
{
	ASSERT_RETURN(texture != nullptr, NO_RETURN_VAL, "Attempted to save a null texture!");

	D3D11_MAPPED_SUBRESOURCE mappedSubResource;
	ID3D11Texture2D* dxSrcTexture = (ID3D11Texture2D*) texture->GetDxHandle();
	ASSERT_RETURN(dxSrcTexture != nullptr, NO_RETURN_VAL, "Attempted to save an uninitialized texture!");

	HRESULT hr = m_dxContext->Map(dxSrcTexture, 0, D3D11_MAP_READ, 0, &mappedSubResource);
	ID3D11Texture2D* dxTextureMapped = dxSrcTexture;

	Texture2D* mappedTexture = texture;
	bool createdStagingTexture = false;

	if (FAILED(hr))
	{
		// Probably failed to map if the texture isn't CPU accessible
		// Create a staging texture and copy it to that, then access
		if (hr == E_INVALIDARG)
		{
			mappedTexture = new Texture2D();
			mappedTexture->CreateWithNoData(texture->GetWidth(), texture->GetHeight(), TEXTURE_FORMAT_R8G8B8A8_UNORM, TEXTURE_USAGE_NO_BIND, GPU_MEMORY_USAGE_STAGING);

			ID3D11Texture2D* dxStagingTexture = (ID3D11Texture2D*)mappedTexture->GetDxHandle();
			m_dxContext->CopyResource(dxStagingTexture, dxSrcTexture);

			// Now map the staging texture
			hr = m_dxContext->Map(dxStagingTexture, 0, D3D11_MAP_READ, 0, &mappedSubResource);
			dxTextureMapped = dxStagingTexture;
			createdStagingTexture = true;
			ASSERT_RETURN(SUCCEEDED(hr), NO_RETURN_VAL, "Couldn't map a staging texture for saving!");
		}
		else
		{
			ASSERT_RETURN(false, NO_RETURN_VAL, "Tried to save a texture to file but failed!");
		}
	}

	// Setup Job info
	// Mapping the texture map pad out the texture to a 16 or 4 byte alignment, so use those dimensions here
	int numComponentsPerTexel = 4;
	int texelWidth = mappedSubResource.RowPitch / 4;
	int texelHeight = mappedSubResource.DepthPitch / mappedSubResource.RowPitch;
	int totalBytes = texelWidth * texelHeight * numComponentsPerTexel;
	void* imgData = malloc(totalBytes); // The job will free this buffer
	memcpy(imgData, mappedSubResource.pData, totalBytes);

	// Unmap and clean up staging texture if used
	m_dxContext->Unmap(dxTextureMapped, 0);

	if (createdStagingTexture)
	{
		SAFE_DELETE(mappedTexture);
	}

	// Kick the job
	SaveTextureJob* saveTexJob = new SaveTextureJob(texelWidth, texelHeight, 4, filepath, imgData);
	g_jobSystem->QueueJob(saveTexJob);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::DxInit()
{
	HWND hwnd = (HWND)g_window->GetWindowContext();

	// Creation Flags
	unsigned int deviceFlags = 0U;

#ifdef DEBUG_DX_DEVICE
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// Setup our Swap Chain
	DXGI_SWAP_CHAIN_DESC swap_desc;
	memset(&swap_desc, 0, sizeof(swap_desc));

	swap_desc.BufferCount = 2;										// number of back buffers
	swap_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;		// how swap chain is to be used
	swap_desc.OutputWindow = hwnd;									// the window to be copied to on present
	swap_desc.SampleDesc.Count = 1;									// how many multisamples (1 means no multi sampling)
	swap_desc.BufferDesc.Width = g_window->GetClientPixelWidth();	// Drawable space width
	swap_desc.BufferDesc.Height = g_window->GetClientPixelHeight();	// Drawable space height
	swap_desc.Windowed = TRUE;										// windowed/full-screen mode
	swap_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;		// use 32-bit color
	swap_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swap_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	// Actually Create
	HRESULT hr = ::D3D11CreateDeviceAndSwapChain(nullptr, // Adapter, if nullptr, will use adapter window is primarily on.
		D3D_DRIVER_TYPE_HARDWARE,	// Driver Type - We want to use the GPU (HARDWARE)
		nullptr,					// Software Module - DLL that implements software mode (we do not use)
		deviceFlags,				// device creation options
		nullptr,					// feature level (use default)
		0U,							// number of feature levels to attempt
		D3D11_SDK_VERSION,			// SDK Version to use
		&swap_desc,					// Description of our swap chain
		&m_dxSwapChain,				// Swap Chain we're creating
		&m_dxDevice,				// [out] The device created
		nullptr,					// [out] Feature Level Acquired
		&m_dxContext);				// Context that can issue commands on this pipe.

	ASSERT_OR_DIE(SUCCEEDED(hr), "D3D11CreateDeviceAndSwapChain failed!");

	// Grab the debug object
#ifdef DEBUG_DX_DEVICE
	hr = m_dxDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)(&m_dxDebug));
	ASSERT_OR_DIE(SUCCEEDED(hr), "Couldn't acquire the debug interface!");
#endif
}


//-------------------------------------------------------------------------------------------------
void RenderContext::PostDxInit()
{
	// Only triangle lists for now
	m_dxContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Samplers
	Sampler* pointSampler = new Sampler();
	pointSampler->SetFilterModes(FILTER_MODE_POINT, FILTER_MODE_POINT);
	m_samplers[SAMPLER_MODE_POINT] = pointSampler;

	Sampler* linearSampler = new Sampler();
	linearSampler->SetFilterModes(FILTER_MODE_LINEAR, FILTER_MODE_LINEAR);
	m_samplers[SAMPLER_MODE_LINEAR] = linearSampler;

	// Default color/depth target
	InitDefaultColorAndDepthViews();

	// Model matrix UBO
	UpdateModelMatrixUBO(Matrix4::IDENTITY);
	BindUniformBuffer(CONSTANT_BUFFER_SLOT_MODEL_MATRIX, &m_modelMatrixUBO);

	// Light UBO
	LightBufferData lightData;
	m_lightUBO.CopyToGPU(&lightData, sizeof(LightBufferData));
	BindUniformBuffer(CONSTANT_BUFFER_SLOT_LIGHT, &m_lightUBO);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::InitDefaultColorAndDepthViews()
{
	// Get current back buffer
	ID3D11Texture2D* backbuffer = nullptr;
	m_dxSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backbuffer);
	
	// Get the back buffer bounds
	D3D11_TEXTURE2D_DESC desc;
	backbuffer->GetDesc(&desc);

	// Color target
	if (m_defaultColorTarget == nullptr)
	{
		m_defaultColorTarget = new Texture2D();
	}

	m_defaultColorTarget->CreateWithNoData(desc.Width, desc.Height, TEXTURE_FORMAT_R8G8B8A8_UNORM, TEXTURE_USAGE_SHADER_RESOURCE_BIT | TEXTURE_USAGE_RENDER_TARGET_BIT, GPU_MEMORY_USAGE_GPU);

	// Depth target
	if (m_defaultDepthStencil == nullptr)
	{
		m_defaultDepthStencil = new Texture2D();
	}

	m_defaultDepthStencil->CreateWithNoData(desc.Width, desc.Height, TEXTURE_FORMAT_R24G8_TYPELESS, TEXTURE_USAGE_DEPTH_STENCIL_BIT, GPU_MEMORY_USAGE_GPU);

	// Create default views for both
	m_defaultColorTarget->CreateOrGetColorTargetView();
	m_defaultDepthStencil->CreateOrGetDepthStencilView();

	DX_SAFE_RELEASE(backbuffer);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::BindVertexStream(const VertexBuffer* vbo)
{
	const VertexLayout* layout = vbo->GetVertexLayout();
	ASSERT_OR_DIE(layout != nullptr, "VertexBuffer had null layout!");

	ID3D11Buffer* handle = vbo->GetDxHandle();
	uint32 stride = layout->GetStride();
	uint32 offset = 0U;
	
	m_dxContext->IASetVertexBuffers(0, 1, &handle, &stride, &offset);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::BindIndexStream(const IndexBuffer* ibo)
{
	ID3D11Buffer* handle = nullptr;
	if (ibo != nullptr)
	{
		handle = ibo->GetDxHandle();
	}

	m_dxContext->IASetIndexBuffer(handle, DXGI_FORMAT_R32_UINT, 0);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::UpdateInputLayout(const VertexLayout* vertexLayout)
{
	m_currentShader->CreateInputLayoutForVertexLayout(vertexLayout);
	m_dxContext->IASetInputLayout(m_currentShader->GetDxInputLayout());
	m_lastInputLayout = m_currentShader->GetInputLayout();
}


//-------------------------------------------------------------------------------------------------
RenderContext::RenderContext()
{
}


//-------------------------------------------------------------------------------------------------
RenderContext::~RenderContext()
{
	SAFE_DELETE(m_samplers[SAMPLER_MODE_POINT]);
	SAFE_DELETE(m_samplers[SAMPLER_MODE_LINEAR]);

	SAFE_DELETE(m_defaultColorTarget);
	SAFE_DELETE(m_defaultDepthStencil);

	// Shutdown DirectX
	// DX11 cannot shutdown in full screen
	m_dxSwapChain->SetFullscreenState(FALSE, NULL);

	DX_SAFE_RELEASE(m_dxSwapChain);
	DX_SAFE_RELEASE(m_dxContext);
	DX_SAFE_RELEASE(m_dxDevice);

	if (m_dxDebug != nullptr)
	{
		m_dxDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
		DX_SAFE_RELEASE(m_dxDebug);
	}
}


//-------------------------------------------------------------------------------------------------
ID3D11Device* RenderContext::GetDxDevice()
{
	return m_dxDevice;
}


//-------------------------------------------------------------------------------------------------
ID3D11DeviceContext* RenderContext::GetDxContext()
{
	return m_dxContext;
}


//-------------------------------------------------------------------------------------------------
IDXGISwapChain* RenderContext::GetDxSwapChain()
{
	return m_dxSwapChain;
}


//-------------------------------------------------------------------------------------------------
RenderTargetView* RenderContext::GetDefaultColorTargetView() const
{
	return m_defaultColorTarget->CreateOrGetColorTargetView();
}


//-------------------------------------------------------------------------------------------------
DepthStencilView* RenderContext::GetDefaultDepthStencilView() const
{
	return m_defaultDepthStencil->CreateOrGetDepthStencilView();
}


//-------------------------------------------------------------------------------------------------
bool RenderContext::Event_WindowResize(NamedProperties& args)
{
	m_defaultDepthStencil->Clear();
	m_defaultColorTarget->Clear();

	int clientWidth = args.Get("client-width", 0);
	int clientHeight = args.Get("client-height", 0);

	HRESULT hr = m_dxSwapChain->ResizeBuffers(0, clientWidth, clientHeight, DXGI_FORMAT_UNKNOWN, 0);
	ASSERT_OR_DIE(hr == S_OK, "Couldn't resize back buffers!");

	InitDefaultColorAndDepthViews();

	return false;
}
