///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 15th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Render/DX11Common.h"
#include "Engine/Render/RenderContext.h"
#include "Engine/Render/Sampler.h"
#include "Engine/Core/EngineCommon.h"

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

//-------------------------------------------------------------------------------------------------
// Currently sets mips to point
static D3D11_FILTER GetDxSamplerFilter(SamplerFilterMode minFilter, SamplerFilterMode magFilter)
{
	if (minFilter == FILTER_MODE_POINT)
	{
		switch (magFilter) 
		{
		case FILTER_MODE_POINT:  return D3D11_FILTER_MIN_MAG_MIP_POINT; break;
		case FILTER_MODE_LINEAR: return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT; break;
		default:
			break;
		};
	}
	else
	{
		// minFilter is linear
		switch (magFilter)
		{
		case FILTER_MODE_POINT:  return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT; break;
		case FILTER_MODE_LINEAR: return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT; break;
		default:
			break;
		};
	}

	// Bad combination?
	ERROR_AND_DIE("Invalid min/mag filter combination!");
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
Sampler::~Sampler()
{
	DX_SAFE_RELEASE(m_dxHandle);
}


//-------------------------------------------------------------------------------------------------
void Sampler::CreateOrUpdate()
{
	if (!m_isDirty)
	{
		return;
	}

	DX_SAFE_RELEASE(m_dxHandle);

	D3D11_SAMPLER_DESC desc;
	memset(&desc, 0, sizeof(D3D11_SAMPLER_DESC));

	desc.Filter = GetDxSamplerFilter(m_minFilter, m_magFilter);
	desc.MaxAnisotropy = 1U;

	desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

	desc.MinLOD = -FLT_MAX;
	desc.MaxLOD = FLT_MAX;
	desc.MipLODBias = 0.0f;

	desc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	ID3D11Device* dxDevice = g_renderContext->GetDxDevice();
	dxDevice->CreateSamplerState(&desc, &m_dxHandle);

	m_isDirty = false;
	ASSERT_OR_DIE(m_dxHandle != nullptr, "Couldn't create sampler state!");
}
