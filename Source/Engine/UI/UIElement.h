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
	float	m_deltaSeconds = 0.f;

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
typedef bool(*UIKeyboardInputHandler)(UIElement* element, unsigned char character);

//-------------------------------------------------------------------------------------------------
enum UIElementRenderMode
{
	ELEMENT_RENDER_ALL,
	ELEMENT_RENDER_SELF,
	ELEMENT_RENDER_CHILDREN,
	ELEMENT_RENDER_NONE
};

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
	RTTI_BASE_CLASS(UIElement);

	UIElement(Canvas* canvas, const StringID& id);
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
	bool				IsCanvas() const;
	bool				IsInFocus() const;
	bool				ShouldRenderSelf() const;
	bool				ShouldRenderChildren() const;

	template <typename T>
	T*					GetFirstChildOfType();

	void				SetID(StringID id);
	void				SetID(const std::string& name);
	void				SetID(const char* name);
	void				SetRenderMode(UIElementRenderMode mode) { m_renderMode = mode; }

	OBB2				GetCanvasBounds() const;
	Matrix44			CreateModelMatrix() const;
	Matrix44			CreateModelMatrix(const OBB2& finalBounds) const;
	
	static UIElement*	CreateUIElementFromXML(const XMLElem& element, UIElement* parent, Canvas* canvas);


public:
	//-----Public Data------

	RectTransform			m_transform;

	// Mouse handlers
	UIMouseInputHandler		m_onJustHovered = nullptr;
	UIMouseInputHandler		m_onHover = nullptr;
	UIMouseInputHandler		m_onUnhovered = nullptr;
	UIMouseInputHandler		m_onMouseClick = nullptr;
	UIMouseInputHandler		m_onMouseHold = nullptr;
	UIMouseInputHandler		m_onMouseRelease = nullptr;

	// Keyboard
	UIKeyboardInputHandler	m_onKeyDown = nullptr;


protected:
	//-----Protected Data------

	StringID						m_id;
	UIElement*						m_parent = nullptr;
	Canvas*							m_canvas = nullptr;
	std::vector<UIElement*>			m_children;
	uint32							m_layer = 0U;
	UIElementRenderMode				m_renderMode = ELEMENT_RENDER_ALL;

};


//-------------------------------------------------------------------------------------------------
template <typename T>
T* UIElement::GetFirstChildOfType()
{
	T* childOfType = nullptr;
	for (size_t childIndex = 0; childIndex < m_children.size(); ++childIndex)
	{
		if (m_children[childIndex]->IsOfType<T>())
		{
#ifdef DISABLE_ASSERTS
			childOfType = m_children[childIndex]->GetAsType<T>();
			break;
#else
			ASSERT_RETURN(childOfType == nullptr, childOfType, "Duplicate children of type when calling GetFirstChildOfType()!");
			childOfType = m_children[childIndex]->GetAsType<T>();
#endif
		}
	}

	return childOfType;
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

bool IsXMLElemForUIText(const XMLElem& element);
bool IsXMLElemForUIImage(const XMLElem& element);

bool PassThroughMouseInput(UIElement* element, const UIMouseInfo& info);
