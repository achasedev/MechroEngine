///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: October 2nd, 2020
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Event/EventSystem.h"
#include "Engine/Framework/DevConsole.h"
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Framework/Window.h"
#include "Engine/IO/Image.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Render/Camera/Camera.h"
#include "Engine/Render/Core/RenderContext.h"
#include "Engine/Render/Material.h"
#include "Engine/Render/Shader.h"
#include "Engine/Render/Texture/Texture2D.h"
#include "Engine/UI/Canvas.h"
#include "Engine/UI/UIImage.h"
#include "Engine/UI/UIPanel.h"
#include "Engine/UI/UIScrollView.h"
#include "Engine/UI/UIText.h"
#include "Engine/Utility/Assert.h"
#include "Engine/Utility/NamedProperties.h"
#include "Engine/Render/Font/Font.h"
#include "Engine/Render/Font/FontLoader.h"

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
DevConsole* g_devConsole = nullptr;

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
	// Give characters priority
	case WM_CHAR:
	{
		g_devConsole->ProcessCharacter(keyCode);
		return true;
	}
	// For handling arrow keys and delete
	case WM_KEYDOWN:
	{
		g_devConsole->ProcessKeydown(keyCode);
		return true;
	}
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
void ConsolePrintf(const Rgba &color, char const *format, ...)
{
	ASSERT_RETURN(g_devConsole != nullptr, NO_RETURN_VAL, "DevConsole not initialized!");

	// Construct the string
	char textLiteral[VARIABLE_ARG_STACK_LOCAL_TEMP_LENGTH];
	va_list variableArgumentList;
	va_start(variableArgumentList, format);
	vsnprintf_s(textLiteral, VARIABLE_ARG_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList);
	va_end(variableArgumentList);
	textLiteral[VARIABLE_ARG_STACK_LOCAL_TEMP_LENGTH - 1] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	// Add it to the console log
	ColoredText colorText;
	colorText.m_text = std::string(textLiteral);
	colorText.m_color = color;

	g_devConsole->AddToMessageQueue(colorText);
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
	g_window->UnregisterMessageHandler(DevConsoleMessageHandler);
	SAFE_DELETE(g_devConsole);
}


//-------------------------------------------------------------------------------------------------
void DevConsole::ProcessCharacter(unsigned char keyCode)
{
	if (m_isActive)
	{
		switch (keyCode)
		{
		case '`':		HandleTilde();		break;
		case VK_RETURN: HandleEnter();		break;
		case VK_BACK:	HandleBackSpace();	break;
		case VK_ESCAPE: HandleEscape();		break;
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
void DevConsole::ProcessKeydown(unsigned char keyCode)
{
	if (m_isActive)
	{
		switch (keyCode)
		{
		case VK_DELETE: HandleDelete();		break;
		case VK_LEFT:	HandleLeftArrow();	break;
		case VK_RIGHT:	HandleRightArrow(); break;
		case VK_UP:		HandleUpArrow();	break;
		case VK_DOWN:	HandleDownArrow();	break;
		default:
			break;
		}
	}
}


//-------------------------------------------------------------------------------------------------
void DevConsole::BeginFrame()
{
}


//-------------------------------------------------------------------------------------------------
void DevConsole::ProcessInput()
{
	m_canvas->ProcessMouseInput();
}


//-------------------------------------------------------------------------------------------------
void DevConsole::Update()
{
	if (m_cursorTimer.CheckAndDecrementAll())
	{
		m_showInputCursor = !m_showInputCursor;

		UIElementRenderMode mode = (m_showInputCursor ? ELEMENT_RENDER_ALL : ELEMENT_RENDER_NONE);
		m_inputCursor->SetRenderMode(mode);
	}

	// Cursor gets moved from many functions, easiest to just update its position constantly
	// also it's cheap, so why not
	UpdateCursorElementPosition();

	// Update the message logs with messages in the queue
	ColoredText text;
	while (m_outputQueue.Dequeue(text)) // returns false when empty
	{
		m_logScrollView->AddTextToScroll(text.m_text, text.m_color);
	}

	m_canvas->Update();
}


//-------------------------------------------------------------------------------------------------
void DevConsole::EndFrame()
{

}


//-------------------------------------------------------------------------------------------------
void DevConsole::Render() const
{
	if (m_isActive)
	{
		Camera camera;
		camera.SetRenderTarget(m_canvas->GetOutputTexture(), false);
		camera.SetProjection(CAMERA_PROJECTION_ORTHOGRAPHIC, m_canvas->GenerateOrthoMatrix());

		g_renderContext->BeginCamera(&camera);
		m_canvas->Render();
		g_renderContext->EndCamera();
	}
}


//-------------------------------------------------------------------------------------------------
void DevConsole::SetIsActive(bool isActive)
{
	m_isActive = isActive;

	m_canvas->SetElementInFocus(m_inputFieldText);
}


//-------------------------------------------------------------------------------------------------
void DevConsole::AddToMessageQueue(const ColoredText& outputText)
{
	m_outputQueue.Enqueue(outputText);
}


//-------------------------------------------------------------------------------------------------
DevConsole::DevConsole()
{
	// TODO: Remove these when ResourceManager is going
	m_shader = new Shader();
	m_shader->CreateFromFile("Data/Shader/font.shader");
	m_shader->SetBlend(BLEND_PRESET_ALPHA);

	m_canvas = new Canvas();
	m_canvas->InitializeFromXML("Data/Engine/Console_Layout.canvas");

	m_backPanel = m_canvas->FindElementAsType<UIPanel>(SID("background_panel"));
	m_inputPanel = m_canvas->FindElementAsType<UIPanel>(SID("input_panel"));
	m_inputFieldText = m_canvas->FindElementAsType<UIText>(SID("input_text"));
	m_logScrollView = m_canvas->FindElementAsType<UIScrollView>(SID("log_scrollview"));
	m_inputCursor = m_canvas->FindElementAsType<UIImage>(SID("input_cursor_image"));

	m_inputFieldText->SetShader(m_shader);
	m_inputCursor->SetShader(m_shader);

	m_logScrollView->GetScrollTextElement()->SetShader(m_shader);

	m_inputFieldText->SetText(">");
	SetCursor(0);
}


//-------------------------------------------------------------------------------------------------
DevConsole::~DevConsole()
{
	m_logScrollView = nullptr;
	m_backPanel = nullptr;
	m_inputPanel = nullptr;
	m_inputFieldText = nullptr;
	
	SAFE_DELETE(m_shader);
	SAFE_DELETE(m_canvas);
}


//-------------------------------------------------------------------------------------------------
void DevConsole::HandleTilde()
{
	SetIsActive(!m_isActive);
}


//-------------------------------------------------------------------------------------------------
void DevConsole::HandleEnter()
{
	std::string input = m_inputFieldText->GetText();
	TrimWhitespace(input);

	// Trim the ">"
	std::string command = input.substr(1);

	if (input.size() > 0)
	{
		// Print the command to the console log
		m_logScrollView->AddTextToScroll(input);

		m_commandHistory.push_back(command);
		m_historyIndex = (int)m_commandHistory.size();

		// Parse the command arguments from the command name
		// Let the event parse the individual commands, as each 
		// command may expect/treat arguments as different types
		// This also preserves the order of the arguments
		NamedProperties args;
		size_t firstSpaceIndex = command.find_first_of(' ');
		std::string commandID = command;

		// If there's arguments
		if (firstSpaceIndex != std::string::npos)
		{
			std::string argsText = command.substr(firstSpaceIndex);
			commandID = command.substr(0, firstSpaceIndex);
			TrimWhitespace(argsText);

			args.Set("args", argsText);
		}

		// Run the command
		bool hasSubscribers = FireEvent(commandID, args);

		if (!hasSubscribers)
		{

		}

		ClearInputField();
	}
}


//-------------------------------------------------------------------------------------------------
void DevConsole::HandleBackSpace()
{
	if (m_cursorPosition > 0)
	{
		std::string inputText = m_inputFieldText->GetText();

		if (inputText.size() > 1)
		{
			inputText.erase(inputText.begin() + m_cursorPosition);
			m_inputFieldText->SetText(inputText);

			MoveCursor(-1);
		}
	}
}


//-------------------------------------------------------------------------------------------------
void DevConsole::HandleDelete()
{
	std::string inputText = m_inputFieldText->GetText();
	if (m_cursorPosition < (int)inputText.size() - 1)
	{
		if (inputText.size() > 1)
		{
			inputText.erase(inputText.begin() + m_cursorPosition + 1);
			m_inputFieldText->SetText(inputText);
		}
	}	
}


//-------------------------------------------------------------------------------------------------
void DevConsole::HandleEscape()
{
	ClearInputField();
}


//-------------------------------------------------------------------------------------------------
void DevConsole::HandleUpArrow()
{
	if (m_commandHistory.size() > 0)
	{
		m_historyIndex = Max(m_historyIndex - 1, 0);

		std::string command = m_commandHistory[m_historyIndex];
		m_inputFieldText->SetText(">" + command);
		m_cursorPosition = (int)command.size();
	}
}


//-------------------------------------------------------------------------------------------------
void DevConsole::HandleDownArrow()
{
	if (m_commandHistory.size() > 0)
	{
		m_historyIndex = Min(m_historyIndex + 1, (int)m_commandHistory.size());

		std::string command = ">";
		if (m_historyIndex < (int)m_commandHistory.size())
		{
			command += m_commandHistory[m_historyIndex];
		}

		m_inputFieldText->SetText(command);
		m_cursorPosition = (int)command.size();
	}
}


//-------------------------------------------------------------------------------------------------
void DevConsole::HandleLeftArrow()
{
	MoveCursor(-1);
}


//-------------------------------------------------------------------------------------------------
void DevConsole::HandleRightArrow()
{
	MoveCursor(1);
}


//-------------------------------------------------------------------------------------------------
void DevConsole::AddCharacterToInputBuffer(unsigned char character)
{
	if (m_inputFieldText->IsInFocus())
	{
		std::string inputText = m_inputFieldText->GetText();
		inputText.insert(inputText.begin() + m_cursorPosition + 1, character);
		m_inputFieldText->SetText(inputText);

		MoveCursor(1);
	}
}


//-------------------------------------------------------------------------------------------------
void DevConsole::UpdateCursorElementPosition()
{
	// Only update the position when the cursor is shown
	if (m_showInputCursor)
	{
		std::string text = m_inputFieldText->GetText();

		// Find the text the cursor would be placed after
		// + 1 since the ">" isn't counted for the cursor position
		text = text.substr(0, m_cursorPosition + 1);

		Vector2 canvasDimensions = m_inputFieldText->GetTextCanvasDimensions(text);
		m_inputCursor->m_transform.SetXPosition(canvasDimensions.x);
	}
}


//-------------------------------------------------------------------------------------------------
void DevConsole::ResetCursorTimer()
{
	m_cursorTimer.SetInterval(m_cursorInterval);
}


//-------------------------------------------------------------------------------------------------
void DevConsole::MoveCursor(int valueToAddToCursor)
{
	SetCursor(m_cursorPosition + valueToAddToCursor);
}


void DevConsole::SetCursor(int valueToBeSetTo)
{
	// Keep the cursor in bounds
	std::string inputText = m_inputFieldText->GetText();
	int finalValue = Clamp(valueToBeSetTo, 0, (int)inputText.size() - 1);
	m_cursorPosition = finalValue;

	// Reset the cursor and force show it
	m_showInputCursor = true;
	m_inputCursor->SetRenderMode(ELEMENT_RENDER_ALL);
	ResetCursorTimer();
}


//-------------------------------------------------------------------------------------------------
void DevConsole::ClearInputField()
{
	m_inputFieldText->SetText(">");
	SetCursor(0);
}
