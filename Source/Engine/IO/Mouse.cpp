///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 20th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/Window.h"
#include "Engine/IO/Mouse.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
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
static IntVector2 GetClientCenterPosInDesktopCoords()
{
	HWND hwnd = (HWND)g_window->GetWindowContext();

	POINT clientTopLeftDesktopSpace;
	clientTopLeftDesktopSpace.x = 0;
	clientTopLeftDesktopSpace.y = 0;
	ClientToScreen(hwnd, &clientTopLeftDesktopSpace);
	
	IntVector2 clientDimensions = g_window->GetClientDimensions();

	IntVector2 clientCenterDesktopSpace;
	clientCenterDesktopSpace.x = clientTopLeftDesktopSpace.x + clientDimensions.x / 2;
	clientCenterDesktopSpace.y = clientTopLeftDesktopSpace.y + clientDimensions.y / 2;

	return IntVector2(clientCenterDesktopSpace.x, clientCenterDesktopSpace.y);
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
void Mouse::SetCursorPosition(const IntVector2& desktopPosition)
{
	m_currFrameDesktopPos = desktopPosition;
	SetCursorPos(desktopPosition.x, desktopPosition.y);
}


//-------------------------------------------------------------------------------------------------
void Mouse::LockCursorToClient(bool lock)
{
	if (lock)
	{
		HWND hwnd = (HWND)g_window->GetWindowContext();

		RECT clientRect;
		GetClientRect(hwnd, &clientRect);

		POINT offset;
		offset.x = 0;
		offset.y = 0;
		ClientToScreen(hwnd, &offset);

		clientRect.left += offset.x;
		clientRect.right += offset.x;
		clientRect.top += offset.y;
		clientRect.bottom += offset.y;

		// lock the mouse to the bounds
		ClipCursor(&clientRect);
	}
	else
	{
		// Just free the mouse
		ClipCursor(nullptr);
	}

	m_isCursorLocked = lock;
}


//-------------------------------------------------------------------------------------------------
void Mouse::ShowMouseCursor(bool show)
{
	int displayCounter = ShowCursor(show);

	// Repeatedly call ShowCursor to put the display counter above/under 0
	if (show)
	{
		while (displayCounter < 0)
		{
			displayCounter = ShowCursor(show);
		}
	}
	else
	{
		while (displayCounter >= 0)
		{
			displayCounter = ShowCursor(show);
		}
	}

	m_isCursorShown = show;
}


//-------------------------------------------------------------------------------------------------
void Mouse::SetCursorMode(CursorMode mode)
{
	m_cursorMode = mode;

	if (mode == CURSORMODE_RELATIVE)
	{
		// Reset state to avoid first-frame bugs
		m_lastFrameDesktopPos = GetClientCenterPosInDesktopCoords();
		m_currFrameDesktopPos = m_lastFrameDesktopPos;
		SetCursorPos(m_currFrameDesktopPos.x, m_currFrameDesktopPos.y);
	}
}


//-------------------------------------------------------------------------------------------------
IntVector2 Mouse::GetCursorClientPosition() const
{
	POINT position;
	position.x = m_currFrameDesktopPos.x;
	position.y = m_currFrameDesktopPos.y;

	HWND hwnd = (HWND)g_window->GetWindowContext();
	ScreenToClient(hwnd, &position);

	return IntVector2(position.x, position.y);
}


//-------------------------------------------------------------------------------------------------
IntVector2 Mouse::GetCursorClientLastFramePosition() const
{
	POINT position;
	position.x = m_lastFrameDesktopPos.x;
	position.y = m_lastFrameDesktopPos.y;

	HWND hwnd = (HWND)g_window->GetWindowContext();
	ScreenToClient(hwnd, &position);

	return IntVector2(position.x, position.y);
}


//-------------------------------------------------------------------------------------------------
IntVector2 Mouse::GetCursorDesktopPosition() const
{
	return m_currFrameDesktopPos;
}


//-------------------------------------------------------------------------------------------------
IntVector2 Mouse::GetMouseDelta() const
{
	return (m_currFrameDesktopPos - m_lastFrameDesktopPos);
}


//-------------------------------------------------------------------------------------------------
float Mouse::GetMouseWheelDelta() const
{
	return m_currFrameWheel;
}


//-------------------------------------------------------------------------------------------------
bool Mouse::WasButtonJustPressed(MouseButton button) const
{
	return m_buttons[button].m_wasJustPressed;
}


//-------------------------------------------------------------------------------------------------
bool Mouse::WasButtonJustReleased(MouseButton button) const
{
	return m_buttons[button].m_wasJustReleased;
}


//-------------------------------------------------------------------------------------------------
bool Mouse::IsButtonPressed(MouseButton button) const
{
	return m_buttons[button].m_isPressed;
}


//-------------------------------------------------------------------------------------------------
bool Mouse::IsCursorShown() const
{
	return m_isCursorShown;
}


//-------------------------------------------------------------------------------------------------
bool Mouse::IsCursorLocked() const
{
	return m_isCursorLocked;
}


//-------------------------------------------------------------------------------------------------
CursorMode Mouse::GetCursorMode() const
{
	return m_cursorMode;
}


//-------------------------------------------------------------------------------------------------
void Mouse::BeginFrame()
{
	m_currFrameWheel = 0.f;

	// Reset all "just" states before updating current state
	for (uint32 buttonIndex = 0; buttonIndex < NUM_MOUSEBUTTONS; ++buttonIndex)
	{
		m_buttons[buttonIndex].m_wasJustPressed = false;
		m_buttons[buttonIndex].m_wasJustReleased = false;
	}

	// Update the cursor position

	// Absolute Mode - I get mouse position - and I can potentially lock to the screen
	UpdateCursorPositions();

	// Relative mode (lock cursor to center)
	if (m_cursorMode == CURSORMODE_RELATIVE) {
		m_lastFrameDesktopPos = GetClientCenterPosInDesktopCoords();
		SetCursorPos(m_lastFrameDesktopPos.x, m_lastFrameDesktopPos.y);
	}
}


//-------------------------------------------------------------------------------------------------
void Mouse::OnMouseButton(size_t wParam)
{
	// Update all buttons
	UpdateButtonState(MOUSEBUTTON_LEFT, (uint16)wParam, MK_LBUTTON);
	UpdateButtonState(MOUSEBUTTON_RIGHT, (uint16)wParam, MK_RBUTTON);
	UpdateButtonState(MOUSEBUTTON_MIDDLE, (uint16)wParam, MK_MBUTTON);
}


//-------------------------------------------------------------------------------------------------
void Mouse::OnMouseWheel(size_t wParam)
{
	short wheel = GET_WHEEL_DELTA_WPARAM(wParam);
	float wheelDelta = (float)wheel * (1.f / (float)WHEEL_DELTA);

	m_currFrameWheel += wheelDelta;
}


//-------------------------------------------------------------------------------------------------
void Mouse::UpdateCursorPositions()
{
	m_lastFrameDesktopPos = m_currFrameDesktopPos;

	POINT desktopPosition;
	GetCursorPos(&desktopPosition);
	m_currFrameDesktopPos = IntVector2(desktopPosition.x, desktopPosition.y);
}


//-------------------------------------------------------------------------------------------------
void Mouse::UpdateButtonState(MouseButton button, uint16 buttonFlags, uint16 buttonMask)
{
	bool isCurrentlyPressed = ((buttonFlags & buttonMask) == buttonMask);
	bool justChanged = (isCurrentlyPressed != m_buttons[button].m_isPressed);

	// Update the button's current pressed state
	m_buttons[button].m_isPressed = isCurrentlyPressed;

	if (justChanged)
	{
		if (isCurrentlyPressed)
		{
			// Just changed and is pressed, so it was just pressed
			m_buttons[button].m_wasJustPressed = true;
		}
		else
		{
			// Just changed and is not pressed, so it was just released
			m_buttons[button].m_wasJustReleased = true;
		}
	}
}
