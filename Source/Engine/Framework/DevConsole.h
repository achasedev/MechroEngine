///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: October 2nd, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/DataStructures/ColoredText.h"
#include "Engine/DataStructures/ThreadSafeQueue.h"
#include "Engine/Time/FrameTimer.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Canvas;
class Shader;
class Texture2D;
class UIImage;
class UIPanel;
class UIScrollView;
class UIText;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class DevConsole
{
public:
	//-----Public Methods-----

	static void	Initialize();
	static void	Shutdown();

	// These need to be separate functions as some keys share keycodes and are only differentiated 
	// by the messages WM_CHAR or WM_KEYDOWN codes
	void		ProcessCharacter(unsigned char keyCode);
	void		ProcessKeydown(unsigned char keyCode);

	void		BeginFrame();
	void		ProcessInput();
	void		Update();
	void		EndFrame();
	void		Render() const;

	void		SetIsActive(bool isActive);
	void		AddToMessageQueue(const ColoredText& outputText);

	// Accessors
	bool		IsActive() const { return m_isActive; }


public:
	//-----Public Data-----

	static const Rgba DEFAULT_CONSOLE_LOG_COLOR;


private:
	//-----Private Methods-----

	DevConsole();
	~DevConsole();

	void HandleTilde();
	void HandleEnter();
	void HandleBackSpace();
	void HandleDelete();
	void HandleEscape();
	void HandleUpArrow();
	void HandleDownArrow();
	void HandleLeftArrow();
	void HandleRightArrow();
	void AddCharacterToInputBuffer(unsigned char character);
	void UpdateCursorElementPosition();
	void ResetCursorTimer();
	void MoveCursor(int valueToAddToCursor);
	void SetCursor(int valueToBeSetTo);
	void ClearInputField();
	void UpdateAutoCompleteElements();


private:
	//-----Private Data-----

	bool							m_isActive = false;
	int								m_cursorPosition = 0;
	int								m_historyIndex = 0;
	std::vector<std::string>		m_commandHistory;
	ThreadSafeQueue<ColoredText>	m_outputQueue;

	// Rendering
	Canvas*			m_canvas = nullptr;
	UIPanel*		m_backPanel = nullptr;
	UIPanel*		m_inputPanel = nullptr;
	UIText*			m_inputFieldText = nullptr;
	UIScrollView*	m_logScrollView = nullptr;
	UIImage*		m_inputCursor = nullptr;
	UIImage*		m_autocompleteImage = nullptr;
	UIText*			m_autocompleteText = nullptr;

	FrameTimer		m_cursorTimer;
	bool			m_showInputCursor = false;
	float			m_cursorInterval = 0.25f; // 0.5 seconds per state (shown or hidden)

	// TODO: Resource System
	Shader*			m_shader = nullptr;

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
void ConsolePrintf(const Rgba &color, char const *format, ...);
void ConsolePrintf(char const *format, ...);
void ConsoleWarningf(char const *format, ...); // Orange Font
void ConsoleErrorf(char const *format, ...); // Red Font