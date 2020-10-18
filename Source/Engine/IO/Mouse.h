///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 20th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/EngineCommon.h"
#include "Engine/IO/KeyButtonState.h"
#include "Engine/Math/IntVector2.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
enum MouseButton
{
	MOUSEBUTTON_LEFT,
	MOUSEBUTTON_RIGHT,
	MOUSEBUTTON_MIDDLE,
	NUM_MOUSEBUTTONS
};

enum CursorMode
{
	CURSORMODE_ABSOLUTE, // Mouse position updates every frame
	CURSORMODE_RELATIVE, // Mouse position locked to client center
	NUM_CURSORMODE
};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class Mouse
{
	friend class InputSystem;

public:
	//-----Public Methods-----

	void		SetCursorPosition(const IntVector2& position);
	void		LockCursorToClient(bool lock);
	void		ShowMouseCursor(bool show);
	void		SetCursorMode(CursorMode mode);

	IntVector2	GetCursorClientPosition() const;
	IntVector2  GetCursorClientLastFramePosition() const;
	IntVector2	GetCursorDesktopPosition() const;
	IntVector2	GetMouseDelta() const;
	float		GetMouseWheelDelta() const;
	bool		WasButtonJustPressed(MouseButton button) const;
	bool		WasButtonJustReleased(MouseButton button) const;
	bool		IsButtonPressed(MouseButton button) const;
	bool		IsCursorShown() const;
	bool		IsCursorLocked() const;
	CursorMode	GetCursorMode() const;

	void		OnMouseButton(size_t wParam);
	void		OnMouseWheel(size_t wParam);


private:
	//-----Private Methods-----

	Mouse() {}
	~Mouse() {}
	Mouse(const Mouse& copy) = delete;

	void BeginFrame();
	void EndFrame();
	void UpdateCursorPositions();
	void UpdateButtonState(MouseButton button, uint16 buttonFlags, uint16 buttonMask);


private:
	//-----Private Data-----
	
	// (0,0) is top left
	IntVector2 m_lastFrameDesktopPos = IntVector2::ZERO;
	IntVector2 m_currFrameDesktopPos = IntVector2::ZERO;
	CursorMode m_cursorMode = CURSORMODE_ABSOLUTE;

	bool m_isCursorShown = true;
	bool m_isCursorLocked = false;
	float m_currFrameWheel = 0.f;

	KeyButtonState m_buttons[NUM_MOUSEBUTTONS];

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
