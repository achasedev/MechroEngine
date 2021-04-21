///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: December 8th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Event/EventSystem.h"
#include "Engine/Framework/DevConsole.h"
#include "Engine/Framework/Window.h"
#include "Engine/IO/File.h"
#include "Engine/IO/Image.h"
#include "Engine/Job/EngineJobs.h"
#include "Engine/Job/JobSystem.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/OBB2.h"
#include "Engine/Math/Polygon2D.h"
#include "Engine/Math/Polygon3d.h"
#include "Engine/Render/Buffer/UniformBuffer.h"
#include "Engine/Render/Buffer/VertexBuffer.h"
#include "Engine/Render/Camera/Camera.h"
#include "Engine/Render/Core/DrawCall.h"
#include "Engine/Render/Core/DX11Common.h"
#include "Engine/Render/Core/Renderable.h"
#include "Engine/Render/Core/RenderContext.h"
#include "Engine/Render/Material.h"
#include "Engine/Render/Mesh/Mesh.h"
#include "Engine/Render/Mesh/MeshBuilder.h"
#include "Engine/Render/Mesh/Vertex.h"
#include "Engine/Render/Sampler.h"
#include "Engine/Render/Shader.h"
#include "Engine/Render/View/RenderTargetView.h"
#include "Engine/Render/View/ShaderResourceView.h"
#include "Engine/Render/View/DepthStencilTargetView.h"
#include "Engine/Render/Texture/Texture2D.h"
#include "Engine/Resource/ResourceSystem.h"
#include "Engine/Utility/NamedProperties.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
enum UniformSlot
{
	UNIFORM_SLOT_FRAME_TIME = 1,
	UNIFORM_SLOT_CAMERA = 2,
	UNIFORM_SLOT_MODEL_MATRIX = 3
};

struct FrameTimeBufferData
{
	float m_deltaTimeSeconds;
	float m_totalTime;
	float m_padding0;
	float m_padding1;
};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------
RenderContext* g_renderContext = nullptr;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
void SetupMaterial(Texture2D* albedo, Shader* shader, Material& out_material)
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
}


//-------------------------------------------------------------------------------------------------
void RenderContext::EndFrame()
{
	// Swap buffers
	m_dxSwapChain->Present(0, 0);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::BeginCamera(Camera* camera)
{
	m_currentCamera = camera;

	// Render to the camera's target
	RenderTargetView* colorView = camera->GetRenderTargetView();
	ASSERT_OR_DIE(colorView != nullptr, "Beginning camera will null target view!");

	ID3D11RenderTargetView* rtv = colorView->GetDxHandle();
	
	DepthStencilTargetView* depthView = camera->GetDepthStencilTargetView();
	ID3D11DepthStencilView* dsv = (depthView != nullptr ? depthView->GetDxHandle() : nullptr);
	
	m_dxContext->OMSetRenderTargets(1, &rtv, dsv);
	
	// Viewport
	D3D11_VIEWPORT viewport;
	memset(&viewport, 0, sizeof(viewport));
	viewport.TopLeftX = 0U;
	viewport.TopLeftY = 0U;
	viewport.Width = (FLOAT)colorView->GetWidth();
	viewport.Height = (FLOAT)colorView->GetHeight();
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	m_dxContext->RSSetViewports(1, &viewport);

	// Uniform Buffer
	camera->UpdateUBO();
	BindUniformBuffer(UNIFORM_SLOT_CAMERA, camera->GetUniformBuffer());
}


//-------------------------------------------------------------------------------------------------
void RenderContext::EndCamera()
{
	m_dxContext->OMSetRenderTargets(0, nullptr, nullptr);
	m_currentCamera = nullptr;
}


//-------------------------------------------------------------------------------------------------
void RenderContext::ClearScreen(const Rgba& color)
{
	ASSERT_OR_DIE(m_currentCamera != nullptr, "No Camera bound!");

	float colors[4];
	colors[0] = color.GetRedFloat();
	colors[1] = color.GetGreenFloat();
	colors[2] = color.GetBlueFloat();
	colors[3] = color.GetAlphaFloat();

	if (m_currentCamera->GetRenderTargetView() != nullptr)
	{
		ID3D11RenderTargetView* dxRTV = nullptr;
		dxRTV = m_currentCamera->GetRenderTargetView()->GetDxHandle();
		m_dxContext->ClearRenderTargetView(dxRTV, colors);
	}
}


//-------------------------------------------------------------------------------------------------
void RenderContext::ClearDepth(float depth /*= 1.0f*/)
{
	ASSERT_OR_DIE(m_currentCamera != nullptr, "No Camera bound!");

	if (m_currentCamera->GetDepthStencilTargetView() != nullptr)
	{
		ID3D11DepthStencilView* dxView = nullptr;
		dxView = m_currentCamera->GetDepthStencilTargetView()->GetDxHandle();
		m_dxContext->ClearDepthStencilView(dxView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, 0U);
	}
}


//-------------------------------------------------------------------------------------------------
void RenderContext::BindUniformBuffer(uint32 slot, UniformBuffer* ubo)
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

	ASSERT_OR_DIE(material->GetShaderResourceView(SRV_SLOT_ALBEDO) != nullptr, "No albedo texture on material!");

	// Bind Texture + Sampler
	BindShaderResourceView(SRV_SLOT_ALBEDO, material->GetShaderResourceView(SRV_SLOT_ALBEDO));

	// Bind Shader
	BindShader(material->GetShader());
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
		m_dxContext->VSSetShader(shader->GetVertexStage(), 0, 0);
		m_dxContext->PSSetShader(shader->GetFragmentStage(), 0, 0);

		shader->UpdateBlendState();
		float black[] = { 0.f, 0.f, 0.f, 0.f };
		m_dxContext->OMSetBlendState(shader->GetDXBlendState(), black, 0xFFFFFFFF);

		shader->UpdateRasterizerState();
		m_dxContext->RSSetState(shader->GetDXRasterizerState());

		m_currentShader = shader;
	}
}


//-------------------------------------------------------------------------------------------------
void RenderContext::BindShaderResourceView(uint32 slot, ShaderResourceView* view)
{
	// TODO: Default the view to a sensible texture if null
	ASSERT_OR_DIE(view != nullptr, "Null TextureView!");

	BindSampler(0U, view->GetSampler());

	ID3D11ShaderResourceView* dxViewHandle = view->GetDxHandle();
	m_dxContext->PSSetShaderResources(slot, 1U, &dxViewHandle);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::BindSampler(uint32 slot, Sampler* sampler)
{
	if (sampler == nullptr)
	{
		sampler = m_samplers[m_samplerMode];
	}
	else
	{
		int x = 4;
		x = 5;
	}

	sampler->CreateOrUpdate();

	ID3D11SamplerState* handle = sampler->GetDxSamplerState();
	m_dxContext->PSSetSamplers(slot, 1U, &handle);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::UpdateModelMatrixUBO(const Matrix44& modelMatrix)
{
	m_modelMatrixUBO.CopyToGPU(&modelMatrix, sizeof(Matrix44));
}


//-------------------------------------------------------------------------------------------------
void RenderContext::DrawMesh(Mesh& mesh)
{
	DrawMeshWithMaterial(mesh, nullptr);
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
			ConsoleErrorf("Draw attempted with a null mesh!");
		}
	}
}


//-------------------------------------------------------------------------------------------------
void RenderContext::Draw(const DrawCall& drawCall)
{
	BindMaterial(drawCall.GetMaterial());
	
	Mesh* mesh = drawCall.GetMesh();
	ASSERT_OR_DIE(mesh != nullptr, "Attempting to draw null mesh!");

	BindVertexStream(mesh->GetVertexBuffer());
	BindIndexStream(mesh->GetIndexBuffer());
	UpdateInputLayout(mesh->GetVertexLayout());
	UpdateModelMatrixUBO(drawCall.GetModelMatrix());

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

	m_dxContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	DrawVertexArray(vertices.data(), (uint32)vertices.size(), nullptr, 0U, material);
	m_dxContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
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

	m_dxContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	DrawVertexArray(vertices.data(), (uint32)vertices.size(), nullptr, 0U, &material);
	m_dxContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::DrawLine2D(const Vector2& start, const Vector2& end, Material* material, const Rgba& color /*= Rgba::WHITE*/)
{
	std::vector<Vertex3D_PCU> vertices;

	Vector3 start3D = Vector3(start, 0.f);
	Vector3 end3D = Vector3(end, 0.f);

	vertices.push_back(Vertex3D_PCU(start3D, color, Vector2::ZERO));
	vertices.push_back(Vertex3D_PCU(end3D, color, Vector2::ZERO));

	m_dxContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	DrawVertexArray(vertices.data(), (uint32)vertices.size(), nullptr, 0U, material);
	m_dxContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::DrawLine3D(const Vector3& start, const Vector3& end, const Rgba& color /*= Rgba::WHITE*/, Shader* shader /*= nullptr*/)
{
	std::vector<Vertex3D_PCU> vertices;

	vertices.push_back(Vertex3D_PCU(start, color, Vector2::ZERO));
	vertices.push_back(Vertex3D_PCU(end, color, Vector2::ZERO));

	Material material;
	SetupMaterial(nullptr, shader, material);

	m_dxContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	DrawVertexArray(vertices.data(), (uint32)vertices.size(), nullptr, 0U, &material);
	m_dxContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::DrawWirePolygon2D(const Polygon2D& polygon, Material* material, const Rgba& color /*= Rgba::WHITE*/)
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

	m_dxContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	DrawVertexArray(vertices.data(), (uint32)vertices.size(), nullptr, 0U, material);
	m_dxContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::DrawWirePolygon3D(const Polygon3d& polygon, const Rgba& color /*= Rgba::WHITE*/, Shader* shader /*= nullptr*/)
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

	m_dxContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	DrawVertexArray(vertices.data(), (uint32)vertices.size(), nullptr, 0U, &material);
	m_dxContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
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

	m_dxContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	DrawVertexArray(vertices.data(), (uint32)vertices.size(), nullptr, 0U, material);
	m_dxContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


//-------------------------------------------------------------------------------------------------
void RenderContext::DrawPlane3(const Plane3& plane, const Rgba& color /*= Rgba::WHITE*/, Shader* shader /*= nullptr*/)
{
	MeshBuilder mb;
	mb.BeginBuilding(true);

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

	m_dxContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	DrawVertexArray(vertices.data(), vertices.size(), nullptr, 0, &material);
	m_dxContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
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
			mappedTexture->CreateWithNoData(texture->GetWidth(), texture->GetHeight(), 4U, TEXTURE_USAGE_NO_BIND, GPU_MEMORY_USAGE_STAGING);

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
	UpdateModelMatrixUBO(Matrix44::IDENTITY);
	BindUniformBuffer(UNIFORM_SLOT_MODEL_MATRIX, &m_modelMatrixUBO);
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

	m_defaultColorTarget->CreateFromDxTexture2D(backbuffer);

	// Depth target
	if (m_defaultDepthStencilTarget == nullptr)
	{
		m_defaultDepthStencilTarget = new Texture2D();
	}

	m_defaultDepthStencilTarget->CreateWithNoData(desc.Width, desc.Height, 0, TEXTURE_USAGE_DEPTH_STENCIL_TARGET_BIT, GPU_MEMORY_USAGE_GPU); // 0 here is ignored for depth stencils, it's always R24G8_TYPELESS

	// Create default views for both
	m_defaultColorTarget->CreateOrGetColorTargetView();
	m_defaultDepthStencilTarget->CreateOrGetDepthStencilTargetView();

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
	// Don't rebind if it's the same layout as previous draw
	if (m_currVertexLayout != vertexLayout)
	{
		m_currentShader->CreateInputLayoutForVertexLayout(vertexLayout);
		m_dxContext->IASetInputLayout(m_currentShader->GetInputLayout());
		m_currVertexLayout = vertexLayout;
	}
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
	SAFE_DELETE(m_defaultDepthStencilTarget);

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
RenderTargetView* RenderContext::GetDefaultRenderTargetView() const
{
	return m_defaultColorTarget->CreateOrGetColorTargetView();
}


//-------------------------------------------------------------------------------------------------
DepthStencilTargetView* RenderContext::GetDefaultDepthStencilTargetView() const
{
	return m_defaultDepthStencilTarget->CreateOrGetDepthStencilTargetView();
}


//-------------------------------------------------------------------------------------------------
bool RenderContext::Event_WindowResize(NamedProperties& args)
{
	m_defaultDepthStencilTarget->Clear();
	m_defaultColorTarget->Clear();

	int clientWidth = args.Get("client-width", 0);
	int clientHeight = args.Get("client-height", 0);

	HRESULT hr = m_dxSwapChain->ResizeBuffers(0, clientWidth, clientHeight, DXGI_FORMAT_UNKNOWN, 0);
	ASSERT_OR_DIE(hr == S_OK, "Couldn't resize back buffers!");

	InitDefaultColorAndDepthViews();

	return false;
}
