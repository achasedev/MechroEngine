///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 20th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/Window.h"
#include "Engine/IO/InputSystem.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------
InputSystem* InputSystem::s_instance = nullptr;
const uint8	InputSystem::KEYBOARD_ESCAPE		= VK_ESCAPE;
const uint8 InputSystem::KEYBOARD_SPACEBAR		= VK_SPACE;
const uint8	InputSystem::KEYBOARD_F1			= VK_F1;
const uint8	InputSystem::KEYBOARD_F2			= VK_F2;
const uint8	InputSystem::KEYBOARD_F3			= VK_F3;
const uint8	InputSystem::KEYBOARD_F4			= VK_F4;
const uint8	InputSystem::KEYBOARD_F5			= VK_F5;
const uint8	InputSystem::KEYBOARD_F6			= VK_F6;
const uint8	InputSystem::KEYBOARD_F7			= VK_F7;
const uint8	InputSystem::KEYBOARD_F8			= VK_F8;
const uint8	InputSystem::KEYBOARD_F9			= VK_F9;
const uint8	InputSystem::KEYBOARD_F10			= VK_F10;
const uint8	InputSystem::KEYBOARD_LEFT_ARROW	= VK_LEFT;
const uint8	InputSystem::KEYBOARD_UP_ARROW		= VK_UP;
const uint8	InputSystem::KEYBOARD_DOWN_ARROW	= VK_DOWN;
const uint8	InputSystem::KEYBOARD_RIGHT_ARROW	= VK_RIGHT;
const uint8 InputSystem::KEYBOARD_TILDE			= VK_OEM_3;
const uint8	InputSystem::KEYBOARD_SHIFT			= VK_SHIFT;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
static bool InputMessageHandler(unsigned int msg, size_t wparam, size_t lparam)
{
	UNUSED(lparam);
	InputSystem* inputSystem = InputSystem::GetInstance();

	// Process the message and pass to the input system if a key was pressed
	unsigned char keyCode = (unsigned char)wparam;
	switch (msg)
	{
	case WM_KEYDOWN:
	{
		inputSystem->OnKeyPressed(keyCode);
		break;
	}
	case WM_KEYUP:
	{
		inputSystem->OnKeyReleased(keyCode);
		break;
	}
	// Mouse input - all handled the same way
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
		InputSystem::GetMouse().OnMouseButton(wparam);
		break;
	case WM_MOUSEWHEEL:
		InputSystem::GetMouse().OnMouseWheel(wparam);
		break;
	}

	return false;
}

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
InputSystem::InputSystem()
{
	for (int i = 0; i < NUM_JOYPADS; i++)
	{
		m_joypads[i] = Joypad(i);
	}
}


//-------------------------------------------------------------------------------------------------
void InputSystem::ResetJustKeyStates()
{
	for (int i = 0; i < NUM_KEYS; i++)
	{
		m_keyboardStates[i].m_wasJustPressed = false;
		m_keyboardStates[i].m_wasJustReleased = false;
	}
}


//-------------------------------------------------------------------------------------------------
void InputSystem::UpdateJoypads()
{
	for (int i = 0; i < NUM_JOYPADS; i++)
	{
		m_joypads[i].Update();
	}
}


//-------------------------------------------------------------------------------------------------
void InputSystem::Initialize()
{
	ASSERT_OR_DIE(s_instance == nullptr, "Initializing InputSystem twice!");
	s_instance = new InputSystem();

	Window::GetInstance()->RegisterMessageHandler(InputMessageHandler);
}


//-------------------------------------------------------------------------------------------------
void InputSystem::Shutdown()
{
	SAFE_DELETE_POINTER(s_instance);
}


//-------------------------------------------------------------------------------------------------
void InputSystem::BeginFrame()
{
	m_mouse.BeginFrame();
	ResetJustKeyStates();
	UpdateJoypads();
}


//-------------------------------------------------------------------------------------------------
void InputSystem::EndFrame()
{
}


//-------------------------------------------------------------------------------------------------
void InputSystem::OnKeyPressed(uint32 keyCode)
{
	if (!m_keyboardStates[keyCode].m_isPressed)
	{
		m_keyboardStates[keyCode].m_wasJustPressed = true;
	}

	m_keyboardStates[keyCode].m_isPressed = true;
}


//-------------------------------------------------------------------------------------------------
void InputSystem::OnKeyReleased(uint32 keyCode)
{
	m_keyboardStates[keyCode].m_isPressed = false;
	m_keyboardStates[keyCode].m_wasJustReleased = true;
}


//-------------------------------------------------------------------------------------------------
bool InputSystem::IsKeyPressed(uint32 keyCode)
{
	return m_keyboardStates[keyCode].m_isPressed;
}


//-------------------------------------------------------------------------------------------------
bool InputSystem::WasKeyJustPressed(uint32 keyCode)
{
	return m_keyboardStates[keyCode].m_wasJustPressed;
}


//-------------------------------------------------------------------------------------------------
bool InputSystem::WasKeyJustReleased(uint32 keyCode)
{
	return m_keyboardStates[keyCode].m_wasJustReleased;
}

