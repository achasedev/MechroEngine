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
class UIElement;

//-------------------------------------------------------------------------------------------------
struct UIMouseInfo
{
	bool	m_leftClicked = false;
	bool	m_leftHeld = false;
	bool	m_leftReleased = false;
	bool	m_rightClicked = false;
	bool	m_rightHeld = false;
	bool	m_rightReleased = false;
	bool	m_thisElementClicked = false; // For hold input, determine if this element was clicked on first

	float	m_mouseWheelDelta = 0.f;

	// All canvas space
	Vector2 m_position = Vector2::ZERO;
	Vector2 m_lastFramePosition = Vector2::ZERO;
	Vector2	m_cursorCanvasDelta = Vector2::ZERO;

	// Where the mouse was on the current hold/drag
	Vector2 m_leftHoldStartPosition = Vector2::ZERO;
	Vector2 m_rightHoldStartPosition = Vector2::ZERO;
	Vector2 m_leftHoldDelta = Vector2::ZERO;
	Vector2 m_rightHoldDelta = Vector2::ZERO;
};

typedef bool(*UIMouseInputHandler)(UIElement* element, const UIMouseInfo& mouseInfo);

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

	void				AddChild(UIElement* child);
	void				RemoveChild(UIElement* child);
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

	template <typename T>
	T* GetAsType() const;

	static UIElement*	CreateUIElementFromXML(const XMLElem& element, Canvas* canvas);


public:
	//-----Public Data------

	RectTransform			m_transform;

	// Mouse handlers
	UIMouseInputHandler		m_onJustHovered = nullptr;
	UIMouseInputHandler		m_onHovered = nullptr;
	UIMouseInputHandler		m_onUnhovered = nullptr;

	UIMouseInputHandler		m_onMouseClick = nullptr;
	UIMouseInputHandler		m_onMouseHold = nullptr;
	UIMouseInputHandler		m_onMouseRelease = nullptr;


protected:
	//-----Protected Data------

	StringID						m_id = INVALID_STRING_ID;
	UIElement*						m_parent = nullptr;
	Canvas*							m_canvas = nullptr;
	std::vector<UIElement*>			m_children;
	uint32							m_layer = 0U;

	static int s_type;

};


//-------------------------------------------------------------------------------------------------
template <typename T>
T* UIElement::GetAsType() const
{
	if (GetType() == T::GetTypeStatic())
	{
		return (T*)this;
	}

	return nullptr;
}

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
