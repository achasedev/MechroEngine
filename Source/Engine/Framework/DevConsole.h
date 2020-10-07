///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: October 2nd, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
class Canvas;
class Panel;
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
	void		EndFrame();
	void		Render() const;

	// Accessors
	bool		IsActive() const { return m_isActive; }


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


private:
	//-----Private Data-----

	bool m_isActive = false;

	std::string m_inputBuffer;
	int			m_cursorPosition = 0;

	// Rendering
	Canvas* m_canvas = nullptr;
	Panel* m_backPanel = nullptr;
	Panel* m_inputPanel = nullptr;
	UIText* m_inputFieldText = nullptr;

};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
void ConsolePrintf(const Rgba &color, char const *format, ...);
void ConsolePrintf(char const *format, ...);
void ConsoleWarningf(char const *format, ...); // Orange Font
void ConsoleErrorf(char const *format, ...); // Red Font