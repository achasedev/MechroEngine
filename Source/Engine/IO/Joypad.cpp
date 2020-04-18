///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: March 20th, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/IO/Joypad.h"
#include "Engine/Math/MathUtils.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <Xinput.h>
#pragma comment( lib, "xinput9_1_0" ) // Link in the xinput.lib static library // #Eiserloh: Xinput 1_4 doesn't work in Windows 7; use 9_1_0 explicitly for broadest compatibility

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
Joypad::Joypad(int joypadIndex)
	: m_joypadIndex(joypadIndex)
{
}


//-------------------------------------------------------------------------------------------------
bool Joypad::IsConnected() const
{
	return m_isConnected;
}


//-------------------------------------------------------------------------------------------------
bool Joypad::IsButtonPressed(JoypadButtonID button) const
{
	return m_buttons[button].m_isPressed;
}


//-------------------------------------------------------------------------------------------------
bool Joypad::WasButtonJustPressed(JoypadButtonID button) const
{
	return m_buttons[button].m_wasJustPressed;
}


//-------------------------------------------------------------------------------------------------
bool Joypad::WasButtonJustReleased(JoypadButtonID button) const
{
	return m_buttons[button].m_wasJustReleased;
}


//-------------------------------------------------------------------------------------------------
bool Joypad::WasStickJustPressed(JoypadStickID stick) const
{
	bool isCurrentlyPressed = m_sticks[stick].m_normalizedCorrectedMagnitude > 0.f;
	bool wasPressedLastFrame = m_sticks[stick].m_lastFrameNormalizedCorrectedMagnitude > 0.f;

	return isCurrentlyPressed && !wasPressedLastFrame;
}


//-------------------------------------------------------------------------------------------------
bool Joypad::WasTriggerJustPulled(JoypadTriggerID trigger) const
{
	const JoypadTriggerState& triggerState = m_triggers[trigger];

	bool isCurrentlyPulled = triggerState.m_valueThisFrame > 0.f;
	bool wasPulledLastFrame = triggerState.m_valueLastFrame > 0.f;
	
	return isCurrentlyPulled && !wasPulledLastFrame;
}


//-------------------------------------------------------------------------------------------------
Vector2 Joypad::GetRawStickPosition(JoypadStickID stick) const
{
	return m_sticks[stick].m_normalizedRawPosition;
}


//-------------------------------------------------------------------------------------------------
Vector2 Joypad::GetCorrectedStickPosition(JoypadStickID stick) const
{
	return m_sticks[stick].m_normalizedCorrectedPosition;
}


//-------------------------------------------------------------------------------------------------
float Joypad::GetStickOrientationDegrees(JoypadStickID stick) const
{
	return m_sticks[stick].m_orientationDegrees;
}


//-------------------------------------------------------------------------------------------------
Vector2 Joypad::GetCardinalStickDirection(JoypadStickID stick) const
{
	return GetNearestCardinalDirection(m_sticks[stick].m_normalizedCorrectedPosition);
}


//-------------------------------------------------------------------------------------------------
float Joypad::GetRawStickMagnitude(JoypadStickID stick) const
{
	return m_sticks[stick].m_normalizedRawMagnitude;
}


//-------------------------------------------------------------------------------------------------
float Joypad::GetCorrectedStickMagnitude(JoypadStickID stick) const
{
	return m_sticks[stick].m_normalizedCorrectedMagnitude;
}


//-------------------------------------------------------------------------------------------------
float Joypad::GetTriggerValue(JoypadTriggerID trigger) const
{
	return m_triggers[trigger].m_valueThisFrame;
}


//-------------------------------------------------------------------------------------------------
void Joypad::Update()
{
	// Fetch the XInput information
	XINPUT_STATE joypadState;
	memset(&joypadState, 0, sizeof(joypadState));

	DWORD errorStatus = XInputGetState(m_joypadIndex, &joypadState);

	if (errorStatus == ERROR_SUCCESS)
	{
		// Is connected, so get its input
		m_isConnected = true;

		// Update all of the buttons
		uint16 buttonFlags = (uint16) joypadState.Gamepad.wButtons;

		UpdateButtonState(JOYPAD_BUTTON_A,					buttonFlags, XINPUT_GAMEPAD_A);
		UpdateButtonState(JOYPAD_BUTTON_B,					buttonFlags, XINPUT_GAMEPAD_B);
		UpdateButtonState(JOYPAD_BUTTON_X,					buttonFlags, XINPUT_GAMEPAD_X);
		UpdateButtonState(JOYPAD_BUTTON_Y,					buttonFlags, XINPUT_GAMEPAD_Y);
		UpdateButtonState(JOYPAD_BUTTON_DPAD_UP,			buttonFlags, XINPUT_GAMEPAD_DPAD_UP);
		UpdateButtonState(JOYPAD_BUTTON_DPAD_DOWN,			buttonFlags, XINPUT_GAMEPAD_DPAD_DOWN);
		UpdateButtonState(JOYPAD_BUTTON_DPAD_LEFT,			buttonFlags, XINPUT_GAMEPAD_DPAD_LEFT);
		UpdateButtonState(JOYPAD_BUTTON_DPAD_RIGHT,			buttonFlags, XINPUT_GAMEPAD_DPAD_RIGHT);
		UpdateButtonState(JOYPAD_BUTTON_THUMBCLICK_LEFT,	buttonFlags, XINPUT_GAMEPAD_LEFT_THUMB);
		UpdateButtonState(JOYPAD_BUTTON_THUMBCLICK_RIGHT,	buttonFlags, XINPUT_GAMEPAD_RIGHT_THUMB);
		UpdateButtonState(JOYPAD_BUTTON_LB,					buttonFlags, XINPUT_GAMEPAD_LEFT_SHOULDER);
		UpdateButtonState(JOYPAD_BUTTON_RB,					buttonFlags, XINPUT_GAMEPAD_RIGHT_SHOULDER);
		UpdateButtonState(JOYPAD_BUTTON_START,				buttonFlags, XINPUT_GAMEPAD_START);
		UpdateButtonState(JOYPAD_BUTTON_BACK,				buttonFlags, XINPUT_GAMEPAD_BACK);

		// Update the sticks
		UpdateStickState(JOYPAD_STICK_LEFT, joypadState.Gamepad.sThumbLX, joypadState.Gamepad.sThumbLY);
		UpdateStickState(JOYPAD_STICK_RIGHT, joypadState.Gamepad.sThumbRX, joypadState.Gamepad.sThumbRY);

		// Update the triggers
		UpdateTriggerState(JOYPAD_TRIGGER_LEFT, joypadState.Gamepad.bLeftTrigger);
		UpdateTriggerState(JOYPAD_TRIGGER_RIGHT, joypadState.Gamepad.bRightTrigger);
	}
	else if (errorStatus == ERROR_DEVICE_NOT_CONNECTED)
	{
		m_isConnected = false;
		ResetButtonStates();
		ResetStickStates();
		ResetTriggerStates();
	}
}


//-------------------------------------------------------------------------------------------------
void Joypad::UpdateButtonState(JoypadButtonID button, uint16 buttonFlags, uint16 buttonMask)
{
	KeyButtonState& currButton = m_buttons[button];

	bool isCurrentlyPressed = ((buttonFlags & buttonMask) == buttonMask);
	bool justChanged = (isCurrentlyPressed != currButton.m_isPressed);

	currButton.m_isPressed = isCurrentlyPressed;

	// Set the 'just' data members to false before starting
	currButton.m_wasJustPressed = false;
	currButton.m_wasJustReleased = false;

	if (justChanged)
	{
		if (isCurrentlyPressed)
		{
			// Just changed and is pressed, so it was just pressed
			currButton.m_wasJustPressed = true;
		}
		else
		{
			// Just changed and is not pressed, so it was just released
			currButton.m_wasJustReleased = true;
		}
	}
}


//-------------------------------------------------------------------------------------------------
void Joypad::UpdateStickState(JoypadStickID stick, short rawX, short rawY)
{
	JoypadStickState& currStick = m_sticks[stick];

	// Before recalculating magnitude, save off the previous magnitude
	currStick.m_lastFrameNormalizedCorrectedMagnitude = currStick.m_normalizedCorrectedMagnitude;

	// Represent the raw input as a float between -1.f and 1.f
	float normalizedRawX = RangeMapFloat(static_cast<float>(rawX), MIN_STICK_RAW_VALUE, MAX_STICK_RAW_VALUE, -1.f, 1.f);
	float normalizedRawY = RangeMapFloat(static_cast<float>(rawY), MIN_STICK_RAW_VALUE, MAX_STICK_RAW_VALUE, -1.f, 1.f);

	currStick.m_normalizedRawPosition = Vector2(normalizedRawX, normalizedRawY);
	currStick.m_normalizedRawMagnitude = currStick.m_normalizedRawPosition.GetLength();
	currStick.m_orientationDegrees = currStick.m_normalizedRawPosition.GetOrientationDegrees();

	// Remove deadzones to create a corrected input
	float correctedMagnitude = RangeMapFloat(currStick.m_normalizedRawMagnitude, DEADZONE_INNER_FRACTION, DEADZONE_OUTER_FRACTION, 0.f, 1.f);
	currStick.m_normalizedCorrectedMagnitude = Clamp(correctedMagnitude, 0.f, 1.0f);

	// Calculate the correct X and Y values
	float normalizedCorrectedX = currStick.m_normalizedCorrectedMagnitude * CosDegrees(currStick.m_orientationDegrees);
	float normalizedCorrectedY = currStick.m_normalizedCorrectedMagnitude * SinDegrees(currStick.m_orientationDegrees);

	// Set the corrected (x, y)
	currStick.m_normalizedCorrectedPosition = Vector2(normalizedCorrectedX, normalizedCorrectedY);
}


//-------------------------------------------------------------------------------------------------
void Joypad::UpdateTriggerState(JoypadTriggerID trigger, uint8 triggerValue)
{
	JoypadTriggerState& triggerState = m_triggers[trigger];
	triggerState.m_valueLastFrame = triggerState.m_valueThisFrame;
	triggerState.m_valueThisFrame = static_cast<float>(triggerValue) * (1.f / 255.f);
}


//-------------------------------------------------------------------------------------------------
void Joypad::ResetButtonStates()
{
	for (int i = 0; i < NUM_JOYPAD_BUTTONS; i++)
	{
		KeyButtonState& currbutton = m_buttons[i];
		currbutton.m_isPressed = false;
		currbutton.m_wasJustPressed = false;
		currbutton.m_wasJustReleased = false;
	}
}


//-------------------------------------------------------------------------------------------------
void Joypad::ResetStickStates()
{
	for (int i = 0; i < NUM_JOYPAD_STICKS; i++)
	{
		JoypadStickState& currStick = m_sticks[i];

		currStick.m_normalizedRawPosition = Vector2::ZERO;
		currStick.m_normalizedCorrectedPosition = Vector2::ZERO;
		currStick.m_normalizedRawMagnitude = 0.f;
		currStick.m_normalizedCorrectedMagnitude = 0.f;
		currStick.m_orientationDegrees = 0.f;
	}
}


//-------------------------------------------------------------------------------------------------
void Joypad::ResetTriggerStates()
{
	for (int i = 0; i < NUM_JOYPAD_TRIGGERS; i++)
	{
		m_triggers[i].m_valueLastFrame = 0.f;
		m_triggers[i].m_valueThisFrame = 0.f;
	}
}
