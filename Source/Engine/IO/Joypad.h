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
#include "Engine/IO/KeyButtonState.h"
#include "Engine/Math/Vector2.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
struct JoypadStickState
{
	Vector2 m_normalizedRawPosition = Vector2::ZERO;
	Vector2 m_normalizedCorrectedPosition = Vector2::ZERO;

	float m_normalizedRawMagnitude = 0.f;
	float m_normalizedCorrectedMagnitude = 0.f;

	float m_orientationDegrees = 0.f;
	float m_lastFrameNormalizedCorrectedMagnitude = 0.f;
};

struct JoypadTriggerState
{
	float m_valueThisFrame = 0.f;
	float m_valueLastFrame = 0.f;
};

enum JoypadButtonID
{
	JOYPAD_BUTTON_ERROR = -1,
	JOYPAD_BUTTON_A,
	JOYPAD_BUTTON_B,
	JOYPAD_BUTTON_X,
	JOYPAD_BUTTON_Y,
	JOYPAD_BUTTON_DPAD_UP,
	JOYPAD_BUTTON_DPAD_DOWN,
	JOYPAD_BUTTON_DPAD_LEFT,
	JOYPAD_BUTTON_DPAD_RIGHT,
	JOYPAD_BUTTON_THUMBCLICK_LEFT,
	JOYPAD_BUTTON_THUMBCLICK_RIGHT,
	JOYPAD_BUTTON_LB,
	JOYPAD_BUTTON_RB,
	JOYPAD_BUTTON_START,
	JOYPAD_BUTTON_BACK,
	NUM_JOYPAD_BUTTONS
};

enum JoypadStickID
{
	JOYPAD_STICK_ERROR = -1,
	JOYPAD_STICK_LEFT,
	JOYPAD_STICK_RIGHT,
	NUM_JOYPAD_STICKS
};

enum JoypadTriggerID
{
	JOYPAD_TRIGGER_ERROR = -1,
	JOYPAD_TRIGGER_LEFT,
	JOYPAD_TRIGGER_RIGHT,
	NUM_JOYPAD_TRIGGERS
};

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class Joypad
{
	friend class InputSystem;

public:
	//-----Public Methods-----

	bool	IsConnected() const;
	bool	IsButtonPressed(JoypadButtonID button) const;
	bool	WasButtonJustPressed(JoypadButtonID button) const;
	bool	WasButtonJustReleased(JoypadButtonID button) const;
	bool	WasStickJustPressed(JoypadStickID stick) const;
	bool	WasTriggerJustPulled(JoypadTriggerID trigger) const;

	Vector2 GetRawStickPosition(JoypadStickID stick) const;
	Vector2 GetCorrectedStickPosition(JoypadStickID stick) const;
	Vector2 GetCardinalStickDirection(JoypadStickID stick) const;
	float	GetStickOrientationDegrees(JoypadStickID stick) const;
	float	GetRawStickMagnitude(JoypadStickID stick) const;
	float	GetCorrectedStickMagnitude(JoypadStickID stick) const;
	float	GetTriggerValue(JoypadTriggerID trigger) const;


private:
	//-----Private Methods-----

	Joypad(int joypadIndex = -1);
	~Joypad() {}
	Joypad(const Joypad& copy) = delete;

	void Update();

	void UpdateButtonState(JoypadButtonID button, uint16 buttonFlags, uint16 buttonMask);
	void UpdateStickState(JoypadStickID stick, short rawX, short rawY);
	void UpdateTriggerState(JoypadTriggerID trigger, uint8 triggerValue);
	void ResetButtonStates();
	void ResetStickStates();
	void ResetTriggerStates();


private:
	//-----Private Data-----

	int					m_joypadIndex = -1;
	bool				m_isConnected = false;
	KeyButtonState		m_buttons[NUM_JOYPAD_BUTTONS];
	JoypadStickState	m_sticks[NUM_JOYPAD_STICKS];
	JoypadTriggerState	m_triggers[NUM_JOYPAD_TRIGGERS];

	static constexpr float DEADZONE_INNER_FRACTION = 0.3f;
	static constexpr float DEADZONE_OUTER_FRACTION = 0.9f;
	static constexpr float MIN_STICK_RAW_VALUE = -32768.f;
	static constexpr float MAX_STICK_RAW_VALUE = 32767.f;

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
