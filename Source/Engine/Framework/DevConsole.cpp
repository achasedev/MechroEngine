///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: October 2nd, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Framework/DevConsole.h"
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Framework/Window.h"
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

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
static bool DevConsoleMessageHandler(unsigned int msg, size_t wparam, size_t lparam)
{
	UNUSED(lparam);
	// Get the keycode of the key pressed
	unsigned char keyCode = (unsigned char)wparam;

	switch (msg)
	{
	case WM_CHAR:
	{
		g_devConsole->ProcessCharacterCode(keyCode);
		return false;
	}
	// For handling arrow keys and delete
	case WM_KEYDOWN:
	{
		g_devConsole->ProcessKeydownCode(keyCode);
		return false;
	}
	}
	return true;
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
void DevConsole::Initialize()
{
	ASSERT_OR_DIE(g_devConsole == nullptr, "Double initializing DevConsole!");
	g_devConsole = new DevConsole();
	
	ASSERT_OR_DIE(g_window != nullptr, "DevConsole cannot register its event handler, the window doesn't exist!");
	g_window->RegisterMessageHandler(DevConsoleMessageHandler);
}


//-------------------------------------------------------------------------------------------------
void DevConsole::Shutdown()
{
	SAFE_DELETE(g_devConsole);
}


//-------------------------------------------------------------------------------------------------
void DevConsole::ProcessKeydown(unsigned char keyCode)
{
	if (m_isActive)
	{
		separate this into two functions because keycodes don't work'
		switch (keyCode)
		{
		case '`':		HandleTilde();		break;
		case VK_RETURN: HandleEnter();		break;
		case VK_BACK:	HandleBackSpace();	break;
		case VK_ESCAPE: HandleEscape();		break;
		case VK_DELETE: HandleDelete();		break;
		case VK_LEFT:	HandleLeftArrow();	break;
		case VK_RIGHT:	HandleRightArrow(); break;
		case VK_UP:		HandleUpArrow();	break;
		case VK_DOWN:	HandleDownArrow();	break;
		default:
			// Regular input, so add to the input field
			AddCharacterToInputBuffer(keyCode);
			break;
		}
	}
	else if (keyCode == '`')
	{
		// To enable the DevConsole
		HandleTilde();
	}
}


//-------------------------------------------------------------------------------------------------
void DevConsole::BeginFrame()
{

}


//-------------------------------------------------------------------------------------------------
void DevConsole::HandleTilde()
{
	m_isActive = !m_isActive;
}


//-------------------------------------------------------------------------------------------------
void DevConsole::HandleLeftArrow()
{
	// Move the cursor to the left, not going beyond right before the first character
	m_cursorPosition--;
	m_cursorPosition = ClampInt(m_cursorPosition, 0, static_cast<int>(m_inputBuffer.size()));
}


//-------------------------------------------------------------------------------------------------
void DevConsole::HandleRightArrow()
{
	// Move the cursor to the right, not going pass the very end of the string
	m_cursorPosition++;
	m_cursorPosition = ClampInt(m_cursorPosition, 0, static_cast<int>(m_inputBuffer.size()));
}
