///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: April 3rd, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Math/OBB2.h"
#include "Engine/Render/Core/Renderable.h"
#include "Engine/UI/RectTransform.h"
#include "Engine/Utility/StringID.h"
#include "Engine/Utility/XMLUtils.h"
#include <vector>

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Canvas;

//-------------------------------------------------------------------------------------------------
struct UIMouseInput
{
	float	m_mouseWheelDelta = 0.f;
	Vector2 m_canvasCursorStartPos;
	Vector2 m_canvasCursorPosition;
};

typedef bool(*UIMouseInputHandler)(const UIMouseInput& mouseInput);

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class UIElement
{
public:
	//-----Public Methods-----

	UIElement(Canvas* canvas);
	virtual ~UIElement();

	virtual void		Update();
	virtual void		Render();

	virtual void		AddChild(UIElement* child);
	void				SetLayer(uint32 layer);
	virtual void		InitializeFromXML(const XMLElem& element);

	UIElement*			GetParent() const { return m_parent; }
	StringID			GetID() const { return m_id; }
	uint32				GetLayer() const { return m_layer; }
	UIElement*			GetChildByID(StringID id);
	Canvas*				GetCanvas() const { return m_canvas; }
	virtual void*		GetType() const { return &s_type; }
	bool				IsCanvas() const;

	void				SetID(StringID id);
	void				SetID(const std::string& name);
	void				SetID(const char* name);

	OBB2				GetCanvasBounds() const;
	Matrix44			CreateModelMatrix() const;
	Matrix44			CreateModelMatrix(const OBB2& finalBounds) const;

	static UIElement*	CreateUIElementFromXML(const XMLElem& element, Canvas* canvas);


public:
	//-----Public Data------

	RectTransform			m_transform;

	// Mouse handlers
	UIMouseInputHandler				m_onJustHovered = nullptr;
	UIMouseInputHandler				m_onHovered = nullptr;
	UIMouseInputHandler				m_onUnhovered = nullptr;

	UIMouseInputHandler				m_onLeftClick = nullptr;
	UIMouseInputHandler				m_onRightClick = nullptr;
	UIMouseInputHandler				m_onLeftRelease = nullptr;
	UIMouseInputHandler				m_onRightRelease = nullptr;

	UIMouseInputHandler				m_onLeftHoldClickedOn = nullptr;
	UIMouseInputHandler				m_onRightHeldClickedOn = nullptr;
	UIMouseInputHandler				m_onLeftHoldClickedOff = nullptr;
	UIMouseInputHandler				m_onRightHeldClickedOff = nullptr;


protected:
	//-----Protected Data------

	StringID						m_id = INVALID_STRING_ID;
	UIElement*						m_parent = nullptr;
	Canvas*							m_canvas = nullptr;
	std::vector<UIElement*>			m_children;
	uint32							m_layer = 0U;

	static int s_type;

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
