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

	virtual void		Render();

	virtual void		AddChild(UIElement* child);
	virtual void		InitializeFromXML(const XMLElem& element);

	UIElement*			GetParent() const { return m_parent; }
	StringID			GetID() const { return m_id; }
	UIElement*			GetChildByID(StringID id);
	Canvas*				GetCanvas() const { return m_canvas; }
	virtual void*		GetType() const { return &s_type; }
	bool				IsCanvas() const;

	void				SetID(StringID id);
	void				SetID(const std::string& name);
	void				SetID(const char* name);

	OBB2				CalculateFinalBounds() const;
	Matrix44			CalculateModelMatrix() const;
	Matrix44			CalculateModelMatrix(const OBB2& finalBounds) const;

	static UIElement*	CreateUIElementFromXML(const XMLElem& element, Canvas* canvas);


public:
	//-----Public Data------

	RectTransform			m_transform;


protected:
	//-----Protected Data------

	StringID						m_id = INVALID_STRING_ID;
	UIElement*						m_parent = nullptr;
	Canvas*							m_canvas = nullptr;
	std::vector<UIElement*>			m_children;

	static int s_type;

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
