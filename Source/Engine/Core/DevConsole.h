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
#include "Engine/Core/ConsoleCommand.h"
#include "Engine/IO/Mouse.h"
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

enum UIInputCursorMode
{
	CURSOR_BLINK,
	CURSOR_SELECTION
};

enum PopUpUIState
{
	POP_UP_HIDDEN,
	POP_UP_AUTOCOMPLETE,
	POP_UP_USAGE
};

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
	void		ProcessKeyUp(unsigned char keyCode);

	void		BeginFrame();
	void		ProcessInput();
	void		Update();
	void		EndFrame();
	void		Render() const;
	
	// Accessors
	bool		IsActive() const { return m_isActive; }
	int			GetBestIndexForMousePosition(const Vector2& mouseCanvasPos);
	std::string GetSelectedInputText() const;
	bool		HasInputSelection() const;

	void		SetIsActive(bool isActive);
	void		AddToLogQueue(const ColoredText& outputText);
	void		AddToMessageQueue(const ColoredText& outputText, float lifetimeSeconds);
	void		MoveCursor(int valueToAddToCursor);
	void		SetCursor(int valueToBeSetTo);
	void		SetCursorToEnd();
	void		ClearInputField();
	void		StartSelection(int startIndex);
	void		SetSelectEndIndex(int endIndex);
	void		ResetInputSelection();
	void		ClearLog();


public:
	//-----Public Data-----

	static const Rgba DEFAULT_CONSOLE_LOG_COLOR;
	static constexpr float DEFAULT_PRINT_LIFETIME_SECONDS = 0.f;


private:
	//-----Private Methods-----

	DevConsole();
	~DevConsole();

	void RegisterEngineCommands();

	void HandleTilde();
	void HandleEnter();
	void HandleBackSpace();
	void HandleDelete();
	void HandleEscape();
	void HandleUpArrow();
	void HandleDownArrow();
	void HandleLeftArrow();
	void HandleRightArrow();
	void HandleTab();
	void AddCharacterToInputBuffer(const char character);
	void AddStringToInputBuffer(const std::string& text);
	void UpdateInputCursorUI();
	void ResetCursorTimer();
	void UpdateAutoCompleteUI();
	void DeleteSelection();
	void CopyInputIntoClipboard();
	void PasteFromClipboard();


private:
	//-----Private Data-----

	bool								m_isActive = false;
	int									m_cursorIndex = 0;
	int									m_historyIndex = 0;
	std::vector<std::string>			m_commandHistory;
	ThreadSafeQueue<ColoredText>		m_logQueue;
	ThreadSafeQueue<std::pair<ColoredText, float>> m_messageQueue;
	std::vector<FrameTimer>				m_messageTimers;
	int									m_autocompleteIndex = 0;
	PopUpUIState						m_popUpState = POP_UP_HIDDEN;

	// Rendering - active
	Canvas*								m_canvas = nullptr;
	UIPanel*							m_activePanel = nullptr;
	UIPanel*							m_inputPanel = nullptr;
	UIText*								m_inputFieldText = nullptr;
	UIScrollView*						m_logScrollView = nullptr;
	UIImage*							m_inputCursor = nullptr;
	UIPanel*							m_popUpPanel = nullptr;
	UIImage*							m_popUpImage = nullptr;
	UIText*								m_popUpText = nullptr;
	UIText*								m_fpsText = nullptr;

	// Rendering - inactive
	UIPanel*							m_inactivePanel = nullptr;
	UIScrollView*						m_messageScrollView = nullptr;

	FrameTimer							m_cursorTimer;
	bool								m_showInputCursor = false;
	float								m_cursorInterval = 0.5f; // seconds per cursor state (shown or hidden)
	float								m_defaultCursorWidth = -1.f;

	// Text Select
	int									m_selectionStartIndex = -1;
	int									m_selectionEndIndex = -1;
	bool								m_shiftHeld = false;
	bool								m_ctrlHeld = false;

	FrameTimer							m_fpsUpdateTimer;

	// Mouse State
	bool								m_wasMouseShown = false;
	bool								m_wasMouseLocked = false;
	CursorMode							m_prevMouseCursorMode = CURSORMODE_ABSOLUTE;

	std::map<StringID, const ConsoleCommand*>	m_commands;

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
void ConsoleLogf(const Rgba &color, char const *format, ...);
void ConsoleLogf(char const *format, ...);
void ConsoleLogf(const std::string& text);
void ConsoleLogf(const Rgba& color, const std::string& text);
void ConsoleLogWarningf(char const *format, ...); // Orange Font
void ConsoleLogErrorf(char const *format, ...); // Red Font

void ConsolePrintf(char const* format, ...);
void ConsolePrintf(const Rgba& color, float lifetimeSeconds, char const* format, ...);
void ConsoleErrorf(char const* format, ...);
void ConsoleErrorf(float lifetimeSeconds, char const* format, ...);
void ConsoleWarningf(char const* format, ...);
void ConsoleWarningf(float lifetimeSeconds, char const* format, ...);
