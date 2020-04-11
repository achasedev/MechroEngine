///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 3rd, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/UI/Canvas.h"
#include "Engine/UI/UIElement.h"

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
UIElement::UIElement(Canvas* canvas)
	: m_transform(RectTransform(canvas))
	, m_canvas(canvas)
{
}


//-------------------------------------------------------------------------------------------------
UIElement::~UIElement()
{
	uint32 numChildren = (uint32)m_children.size();
	for (uint32 childIndex = 0; childIndex < numChildren; ++childIndex)
	{
		SAFE_DELETE_POINTER(m_children[childIndex]);
	}

	m_children.clear();
}


//-------------------------------------------------------------------------------------------------
void UIElement::Render() const
{
	// Parent should already have rendered themselves
	// Now render the children on top
	uint32 numChildren = (uint32)m_children.size();
	for (uint32 childIndex = 0; childIndex < numChildren; ++childIndex)
	{
		m_children[childIndex]->Render();
	}
}


//-------------------------------------------------------------------------------------------------
void UIElement::AddChild(UIElement* child)
{
	m_children.push_back(child);
	child->m_parent = this;
	child->m_transform.SetParentTransform(&m_transform);
}


//-------------------------------------------------------------------------------------------------
void UIElement::SetCanvas(Canvas* canvas)
{
	m_canvas = canvas;
}


//-------------------------------------------------------------------------------------------------
OBB2 UIElement::CalculateFinalBounds() const
{
	return m_transform.GetBounds();
}


//-------------------------------------------------------------------------------------------------
Matrix44 UIElement::CalculateModelMatrix() const
{
	OBB2 finalBounds = CalculateFinalBounds();

	// Account for pivot:
	// - Translate in normalized space
	// - Apply translation/rotation/scale (now rotating about correct point)
	// - Translate back in new scaled space to "undo" the translation to line up the rotation pivot

	Vector3 translation = Vector3(finalBounds.alignedBounds.GetBottomLeft(), 0.f);
	Vector3 rotation = Vector3(0.f, 0.f, finalBounds.orientationDegrees);
	Vector3 scale = Vector3(finalBounds.alignedBounds.GetWidth(), finalBounds.alignedBounds.GetHeight(), 1.0f);
	Vector3 pivotTranslation = Vector3(m_transform.GetPivot(), 0.f);
	Vector3 postRotateCorrection = Vector3(pivotTranslation.x * scale.x, pivotTranslation.y * scale.y, 0.f);

	return Matrix44::MakeTranslation(postRotateCorrection) * Matrix44::MakeModelMatrix(translation, rotation, scale) * Matrix44::MakeTranslation(-1.0f * pivotTranslation);
}
