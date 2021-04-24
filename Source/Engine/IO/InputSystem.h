///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 20th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/IO/Joypad.h"
#include "Engine/IO/Mouse.h"

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
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class InputSystem
{
public:
	//-----Public Methods-----

	static void			Initialize();
	static void			Shutdown();

	void				BeginFrame();
	void				EndFrame();

	void				OnKeyPressed(uint32 keyCode);
	void				OnKeyReleased(uint32 keyCode);

	bool				IsKeyPressed(uint32 keyCode);
	bool				WasKeyJustPressed(uint32 keyCode);
	bool				WasKeyJustReleased(uint32 keyCode);

	Joypad&				GetJoypad(uint32 joypadIndex) { return m_joypads[joypadIndex]; }
	static Mouse&		GetMouse() { return g_inputSystem->m_mouse; }
	static Joypad&		GetPlayerOneJoypad() { return g_inputSystem->m_joypads[0]; }


private:
	//-----Private Methods-----

	InputSystem();
	~InputSystem() {}
	InputSystem(const InputSystem& copy) = delete;

	void ResetJustKeyStates();
	void UpdateJoypads();


public:
	//-----Public Data-----

	static const uint32 NUM_KEYS = 256;
	static const uint32 NUM_JOYPADS = 4;

	static const uint8	KEYBOARD_ESCAPE;
	static const uint8	KEYBOARD_SPACEBAR;
	static const uint8	KEYBOARD_CONTROL;
	static const uint8	KEYBOARD_UP_ARROW;
	static const uint8	KEYBOARD_DOWN_ARROW;
	static const uint8	KEYBOARD_LEFT_ARROW;
	static const uint8	KEYBOARD_RIGHT_ARROW;
	static const uint8	KEYBOARD_F1;
	static const uint8	KEYBOARD_F2;
	static const uint8  KEYBOARD_F3;
	static const uint8  KEYBOARD_F4;
	static const uint8  KEYBOARD_F5;
	static const uint8  KEYBOARD_F6;
	static const uint8  KEYBOARD_F7;
	static const uint8  KEYBOARD_F8;
	static const uint8  KEYBOARD_F9;
	static const uint8  KEYBOARD_F10;
	static const uint8  KEYBOARD_TILDE;
	static const uint8	KEYBOARD_SHIFT;


private:
	//-----Private Data-----

	Mouse m_mouse;
	KeyButtonState m_keyboardStates[NUM_KEYS];
	Joypad m_joypads[NUM_JOYPADS];

};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
