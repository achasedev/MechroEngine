///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 4th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Event/EventSystem.h"
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Render/Camera/Camera.h"
#include "Engine/UI/Canvas.h"
#include "Engine/UI/UIElement.h"
#include "Engine/Utility/NamedProperties.h"

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
Canvas::Canvas()
	: UIElement(nullptr)
{
}

//-------------------------------------------------------------------------------------------------
Canvas::~Canvas()
{
	g_eventSystem->UnsubscribeEventCallbackObjectMethod("ortho-resize", &Canvas::Event_OrthoResize, *this);
}


//-------------------------------------------------------------------------------------------------
void Canvas::Initialize(Camera* orthoCamera, const Vector2& resolution, ScreenMatchMode mode, float widthHeightBlend /*= 1.0f*/)
{
	m_orthoCamera = orthoCamera;
	m_matchMode = mode;
	m_widthOrHeightBlend = widthHeightBlend;
	m_resolution = Vector2(resolution.x, resolution.y);
	m_transform.SetPosition(orthoCamera->GetOrthoBounds().GetTopLeft());
	m_transform.SetDimensions(resolution.x, resolution.y);
	m_transform.SetPivot(Vector2(0.f, 1.0f));

	UpdateScaleForCurrentMatchMode();

	g_eventSystem->SubscribeEventCallbackObjectMethod("ortho-resize", &Canvas::Event_OrthoResize, *this);
}


//-------------------------------------------------------------------------------------------------
void Canvas::Render() const
{
	UIElement::Render();
}


//-------------------------------------------------------------------------------------------------
void Canvas::SetOrthoCamera(Camera* orthoCamera)
{
	m_orthoCamera = orthoCamera;
}


//-------------------------------------------------------------------------------------------------
void Canvas::SetScreenMatchMode(ScreenMatchMode mode, float widthHeightBlend /*= 1.0f*/)
{
	m_matchMode = mode;
	m_widthOrHeightBlend = widthHeightBlend;

	UpdateScaleForCurrentMatchMode();
}


//-------------------------------------------------------------------------------------------------
void Canvas::SetResolution(float width, float height)
{
	m_resolution = Vector2(width, height);
	m_transform.SetDimensions(width, height);

	UpdateScaleForCurrentMatchMode();
}


//-------------------------------------------------------------------------------------------------
bool Canvas::Event_OrthoResize(NamedProperties& args)
{
	UNUSED(args);

	AABB2 orthoBounds = args.Get("ortho-bounds", m_orthoCamera->GetOrthoBounds());
	m_transform.SetPosition(orthoBounds.GetTopLeft());

	UpdateScaleForCurrentMatchMode();

	return false;
}


//-------------------------------------------------------------------------------------------------
void Canvas::UpdateScaleForCurrentMatchMode()
{
	AABB2 orthoBounds = m_orthoCamera->GetOrthoBounds();
	Vector2 orthoDimensions = orthoBounds.GetDimensions();

	float widthScalar = orthoDimensions.x / m_resolution.x;
	float heightScalar = orthoDimensions.y / m_resolution.y;

	float newScale = 1.0f;
	switch (m_matchMode)
	{
	case SCREEN_MATCH_WIDTH_OR_HEIGHT:
		newScale = (heightScalar * m_widthOrHeightBlend) + (widthScalar * (1.0f - m_widthOrHeightBlend));
		break;
	case SCREEN_MATCH_EXPAND_TO_FILL:
		newScale = Max(widthScalar, heightScalar);
		break;
	case SCREEN_MATCH_SHRINK_TO_FIT:
		newScale = Min(widthScalar, heightScalar);
		break;
	default:
		break;
	}
	
	m_transform.SetScale(newScale);
}
