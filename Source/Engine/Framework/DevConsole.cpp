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
#include "Engine/Framework/EngineCommands.h"
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Framework/Window.h"
#include "Engine/IO/Image.h"
#include "Engine/IO/InputSystem.h"
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
#include "Engine/Utility/EngineUtils.h"
#include "Engine/Utility/NamedProperties.h"
#include "Engine/Render/Font/Font.h"
#include "Engine/Time/Clock.h"

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
const Rgba DevConsole::DEFAULT_CONSOLE_LOG_COLOR = Rgba(204, 204, 204, 255);

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
	case WM_KEYUP:
	{
		g_devConsole->ProcessKeyUp(keyCode);
		return true;
	}
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
static void ConsolePrintv(const Rgba& color, char const* format, va_list args)
{
	//ASSERT_RETURN(g_devConsole != nullptr, NO_RETURN_VAL, "DevConsole not initialized!");
	if (g_devConsole != nullptr)
	{
		char textLiteral[VARIABLE_ARG_STACK_LOCAL_TEMP_LENGTH];
		vsnprintf_s(textLiteral, VARIABLE_ARG_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, args);
		textLiteral[VARIABLE_ARG_STACK_LOCAL_TEMP_LENGTH - 1] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

		// Add it to the console log
		ColoredText colorText;
		colorText.m_text = std::string(textLiteral);
		colorText.m_color = color;

		g_devConsole->AddToMessageQueue(colorText);
	}
}


//-------------------------------------------------------------------------------------------------
void ConsolePrintf(const Rgba &color, char const *format, ...)
{
	va_list variableArgumentList;
	va_start(variableArgumentList, format);
	ConsolePrintv(color, format, variableArgumentList);
	va_end(variableArgumentList);
}


//-------------------------------------------------------------------------------------------------
void ConsolePrintf(char const *format, ...)
{
	// Construct the string
	va_list variableArgumentList;
	va_start(variableArgumentList, format);
	ConsolePrintv(DevConsole::DEFAULT_CONSOLE_LOG_COLOR, format, variableArgumentList);
	va_end(variableArgumentList);
}


//-------------------------------------------------------------------------------------------------
void ConsolePrintf(const Rgba& color, const std::string& text)
{
	ConsolePrintf(color, text.c_str());
}


//-------------------------------------------------------------------------------------------------
void ConsolePrintf(const std::string& text)
{
	ConsolePrintf(DevConsole::DEFAULT_CONSOLE_LOG_COLOR, text.c_str());
}


//-------------------------------------------------------------------------------------------------
void ConsoleWarningf(char const *format, ...)
{
	va_list variableArgumentList;
	va_start(variableArgumentList, format);
	ConsolePrintv(Rgba::ORANGE, format, variableArgumentList);
	va_end(variableArgumentList);
}


//-------------------------------------------------------------------------------------------------
void ConsoleErrorf(char const *format, ...)
{
	va_list variableArgumentList;
	va_start(variableArgumentList, format);
	ConsolePrintv(Rgba::RED, format, variableArgumentList);
	va_end(variableArgumentList);
}


//-------------------------------------------------------------------------------------------------
static bool OnMouseClick_InputField(UIElement* element, const UIMouseInfo& mouseInfo)
{
	UNUSED(element);

	g_devConsole->ResetInputSelection();
	int startIndex = g_devConsole->GetBestIndexForMousePosition(mouseInfo.m_position);
	g_devConsole->StartSelection(startIndex);

	return true;
}


//-------------------------------------------------------------------------------------------------
static bool OnMouseHold_InputField(UIElement* element, const UIMouseInfo& mouseInfo)
{
	UNUSED(element);
	int endIndex = g_devConsole->GetBestIndexForMousePosition(mouseInfo.m_position);
	g_devConsole->SetSelectEndIndex(endIndex);

	return true;
}


//-------------------------------------------------------------------------------------------------
static bool OnMouseRelease_InputField(UIElement* element, const UIMouseInfo& mouseInfo)
{
	UNUSED(element);

	// Final update for accuracy
	int currIndex = g_devConsole->GetBestIndexForMousePosition(mouseInfo.m_position);
	g_devConsole->SetSelectEndIndex(currIndex);

	if (!g_devConsole->HasInputSelection())
	{
		g_devConsole->ResetInputSelection();
		g_devConsole->SetCursor(currIndex);
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
		case VK_TAB:						break; // Do nothing
		case 1: // Ctrl + a
			// Select the entire line
			StartSelection(0);
			SetSelectEndIndex(static_cast<int>(m_inputFieldText->GetText().size()) - 1); // -1 here since there's still a '>'
			break;
		case 3: // Ctrl + c
			CopyInputIntoClipboard();
			break;
		case 22: // Ctrl + v
			PasteFromClipboard();
			break;
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
		case VK_TAB:	HandleTab();		break;
		case VK_SHIFT:
			m_shiftHeld = true;
			break;
		case VK_CONTROL:
			m_ctrlHeld = true;
			break;
		default:
			break;
		}
	}
}


//-------------------------------------------------------------------------------------------------
void DevConsole::ProcessKeyUp(unsigned char keyCode)
{
	// Listen for keyups even when not active in case the 
	// console was closed before the key up
	switch (keyCode)
	{
	case VK_SHIFT:
		m_shiftHeld = false;
		break;
	case VK_CONTROL:
		m_ctrlHeld = false;
		break;
	default:
		break;
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
	if (!HasInputSelection() && m_cursorTimer.CheckAndDecrementAll())
	{
		m_showInputCursor = !m_showInputCursor;

		UIElementRenderMode mode = (m_showInputCursor ? ELEMENT_RENDER_ALL : ELEMENT_RENDER_NONE);
		m_inputCursor->SetRenderMode(mode);
	}

	// Cursor gets moved from many functions, easiest to just update its position constantly
	// also it's cheap, so why not
	UpdateInputCursorUI();

	// Update the message logs with messages in the queue
	ColoredText text;
	while (m_outputQueue.Dequeue(text)) // returns false when empty
	{
		m_logScrollView->AddTextToScroll(text.m_text, text.m_color);
	}

	// Update FPS display
	if (m_fpsUpdateTimer.CheckAndDecrementAll())
	{
		Clock* masterClock = Clock::GetMasterClock();
		float deltaSeconds = masterClock->GetDeltaSeconds();
		float fps = (deltaSeconds > 0.f ? (1.0f / deltaSeconds) : 0.f);

		std::string fpsText = Stringf("FPS: %.2f", fps);
		std::string frameText = (deltaSeconds > 0.01f ? Stringf("%.2f ms", deltaSeconds) : Stringf("%.2f us", deltaSeconds * 1000.f));

		m_fpsText->SetLine(0, fpsText);
		m_fpsText->SetLine(1, frameText);
	}

	// UI update
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

	Mouse& mouse = InputSystem::GetMouse();

	if (m_isActive)
	{
		// Save off the previous mouse state to restore it on exit
		m_wasMouseShown = mouse.IsCursorShown();
		m_wasMouseLocked = mouse.IsCursorLocked();
		m_prevMouseCursorMode = mouse.GetCursorMode();

		// Activate the mouse
		mouse.ShowMouseCursor(true);
		mouse.LockCursorToClient(false);
		mouse.SetCursorMode(CURSORMODE_ABSOLUTE);
	}
	else
	{
		// Restore the mouse state
		mouse.ShowMouseCursor(m_wasMouseShown);
		mouse.LockCursorToClient(m_wasMouseLocked);
		mouse.SetCursorMode(m_prevMouseCursorMode);
	}
}


//-------------------------------------------------------------------------------------------------
void DevConsole::AddToMessageQueue(const ColoredText& outputText)
{
	m_outputQueue.Enqueue(outputText);
}


//-------------------------------------------------------------------------------------------------
int DevConsole::GetBestIndexForMousePosition(const Vector2& mouseCanvasPos)
{
	std::string inputText = m_inputFieldText->GetText();

	float startX = m_inputFieldText->GetCanvasBounds().m_alignedBounds.mins.x;
	float bestDiff = -1;
	float prevDiff = -1;
	int bestIndex = -1;

	for (int index = 0; index < (int)inputText.size(); ++index)
	{
		std::string testString = inputText.substr(0, index + 1);
		Vector2 dimensions = m_inputFieldText->GetTextCanvasDimensions(testString);

		float xPos = startX + dimensions.x;
		float diff = Abs(xPos - mouseCanvasPos.x);

		if (bestIndex == -1 || diff < bestDiff)
		{
			bestDiff = diff;
			bestIndex = index;
			prevDiff = diff;
		}
		else
		{
			// Once we aren't getting any better, it will forever only get worse so early out
			break;
		}
	}

	return bestIndex;
}


//-------------------------------------------------------------------------------------------------
std::string DevConsole::GetSelectedInputText() const
{
	std::string inputText = m_inputFieldText->GetText();

	if (inputText.size() == 0)
	{
		return "";
	}

	// Get rid of '>'
	inputText = inputText.substr(1);

	ASSERT_OR_DIE(m_selectionStartIndex > -1, "Bad selection start index!");
	ASSERT_OR_DIE(m_selectionStartIndex <= (int)inputText.size(), "Bad selection start index!");
	ASSERT_OR_DIE(m_selectionEndIndex > -1, "Bad selection end index!");
	ASSERT_OR_DIE(m_selectionEndIndex <= (int)inputText.size(), "Bad selection end index!");
	
	if (m_selectionStartIndex == m_selectionEndIndex)
	{
		return "";
	}

	int lowerIndex = Min(m_selectionStartIndex, m_selectionEndIndex);
	int upperIndex = Max(m_selectionStartIndex, m_selectionEndIndex);
	int numChars = upperIndex - lowerIndex;

	return inputText.substr(lowerIndex, numChars);
}


//-------------------------------------------------------------------------------------------------
bool DevConsole::HasInputSelection() const
{
	return (m_selectionStartIndex >= 0) && (m_selectionEndIndex >= 0) && (m_selectionStartIndex != m_selectionEndIndex);
}


//-------------------------------------------------------------------------------------------------
DevConsole::DevConsole()
{
	m_canvas = new Canvas();
	m_canvas->InitializeFromXML("Data/Engine/Console_Layout.canvas");

	m_backPanel = m_canvas->FindElementAsType<UIPanel>(SID("background_panel"));
	m_inputPanel = m_canvas->FindElementAsType<UIPanel>(SID("input_panel"));
	m_inputFieldText = m_canvas->FindElementAsType<UIText>(SID("input_text"));
	m_logScrollView = m_canvas->FindElementAsType<UIScrollView>(SID("log_scrollview"));
	m_inputCursor = m_canvas->FindElementAsType<UIImage>(SID("input_cursor_image"));
	m_popUpImage = m_canvas->FindElementAsType<UIImage>(SID("popup_image"));
	m_popUpText = m_canvas->FindElementAsType<UIText>(SID("popup_text"));
	m_popUpPanel = m_canvas->FindElementAsType<UIPanel>(SID("popup_panel"));
	m_popUpPanel->SetRenderMode(ELEMENT_RENDER_NONE);
	m_fpsText = m_canvas->FindElementAsType<UIText>(SID("fps_text"));

	m_inputFieldText->m_onMouseClick = OnMouseClick_InputField;
	m_inputFieldText->m_onMouseHold = OnMouseHold_InputField;
	m_inputFieldText->m_onMouseRelease = OnMouseRelease_InputField;
	m_inputCursor->m_onMouseClick = PassThroughMouseInput;
	m_inputCursor->m_onMouseRelease = PassThroughMouseInput;
	m_inputFieldText->SetText(">");
	m_defaultCursorWidth = m_inputCursor->m_transform.GetWidth();
	SetCursor(0);

	m_fpsUpdateTimer.SetInterval(0.5f);

	RegisterEngineCommands();
}


//-------------------------------------------------------------------------------------------------
DevConsole::~DevConsole()
{
	m_fpsText = nullptr;
	m_popUpText = nullptr;
	m_popUpImage = nullptr;
	m_popUpPanel = nullptr;
	m_logScrollView = nullptr;
	m_backPanel = nullptr;
	m_inputPanel = nullptr;
	m_inputFieldText = nullptr;

	SAFE_DELETE(m_canvas);
}


//-------------------------------------------------------------------------------------------------
void DevConsole::RegisterEngineCommands()
{
	ConsoleCommand::Register(SID("clear"),	"Clears the log",	"clear <NO_PARAMS>",									Command_ClearLog,	true);
	ConsoleCommand::Register(SID("add"),	"Adds two numbers", "add (first:float) (second:float)",						Command_Add,		true);
	ConsoleCommand::Register(SID("help"),	"Prints out available console commands", "help (type:string:OPTIONAL)",		Command_Help,		true);
}	


//-------------------------------------------------------------------------------------------------
void DevConsole::HandleTilde()
{
	SetIsActive(!m_isActive);
}


//-------------------------------------------------------------------------------------------------
void DevConsole::HandleEnter()
{
	if (m_inputFieldText->IsInFocus())
	{
		std::string input = m_inputFieldText->GetText();

		// Trim the '>' and whitespace
		std::string commandLine = input.substr(1);
		TrimWhitespace(commandLine);

		if (commandLine.size() > 0)
		{
			// Print the command to the console log
			m_logScrollView->AddTextToScroll(input);

			m_commandHistory.push_back(commandLine);
			m_historyIndex = (int)m_commandHistory.size();

			ConsoleCommand::Run(commandLine);

			ClearInputField();
		}
	}
}


//-------------------------------------------------------------------------------------------------
void DevConsole::HandleBackSpace()
{
	if (m_inputFieldText->IsInFocus())
	{
		if (HasInputSelection())
		{
			DeleteSelection();
		}
		else
		{
			std::string inputText = m_inputFieldText->GetText();

			if (inputText.size() > 1)
			{
				inputText.erase(inputText.begin() + m_cursorIndex);
				m_inputFieldText->SetText(inputText);
				MoveCursor(-1);
			}
		}

		UpdateAutoCompleteUI();
	}
}


//-------------------------------------------------------------------------------------------------
void DevConsole::HandleDelete()
{
	if (m_inputFieldText->IsInFocus())
	{
		if (HasInputSelection())
		{
			DeleteSelection();
		}
		else
		{
			// Erase in front of the cursor
			std::string inputText = m_inputFieldText->GetText();

			if (m_cursorIndex < (int)inputText.size() - 1)
			{
				if (inputText.size() > 1)
				{
					inputText.erase(inputText.begin() + m_cursorIndex + 1);
					m_inputFieldText->SetText(inputText);
				}
			}
		}

		UpdateAutoCompleteUI();
	}
}


//-------------------------------------------------------------------------------------------------
void DevConsole::HandleEscape()
{
	if (m_inputFieldText->IsInFocus())
	{
		ClearInputField();
	}
}


//-------------------------------------------------------------------------------------------------
void DevConsole::HandleUpArrow()
{
	if (m_inputFieldText->IsInFocus())
	{
		if (m_popUpState == POP_UP_AUTOCOMPLETE)
		{
			m_popUpText->SetColor(m_autocompleteIndex, DEFAULT_CONSOLE_LOG_COLOR);
			m_autocompleteIndex = Max(0, m_autocompleteIndex - 1);
			m_popUpText->SetColor(m_autocompleteIndex, Rgba::YELLOW);
		}
		else if ((m_popUpState == POP_UP_HIDDEN) && (m_commandHistory.size() > 0))
		{
			// Allow history scrolling
			m_historyIndex = Max(m_historyIndex - 1, 0);

			std::string command = m_commandHistory[m_historyIndex];
			m_inputFieldText->SetText(">" + command);
			m_cursorIndex = (int)command.size();
		}
	}
}


//-------------------------------------------------------------------------------------------------
void DevConsole::HandleDownArrow()
{
	if (m_inputFieldText->IsInFocus())
	{
		if (m_popUpState == POP_UP_AUTOCOMPLETE)
		{
			int numLines = (int)m_popUpText->GetNumLines();

			m_popUpText->SetColor(m_autocompleteIndex, DEFAULT_CONSOLE_LOG_COLOR);
			m_autocompleteIndex = Min(m_autocompleteIndex + 1, numLines - 1);
			m_popUpText->SetColor(m_autocompleteIndex, Rgba::YELLOW);
		}
		else if ((m_popUpState == POP_UP_HIDDEN) && (m_commandHistory.size() > 0))
		{
			m_historyIndex = Min(m_historyIndex + 1, (int)m_commandHistory.size());

			std::string command = ">";
			if (m_historyIndex < (int)m_commandHistory.size())
			{
				command += m_commandHistory[m_historyIndex];
			}

			m_inputFieldText->SetText(command);
			m_cursorIndex = (int)command.size();
		}
	}
}


//-------------------------------------------------------------------------------------------------
void DevConsole::HandleLeftArrow()
{
	if (m_inputFieldText->IsInFocus())
	{
		if (m_shiftHeld)
		{
			if (!HasInputSelection() && m_cursorIndex > 0)
			{
				// Start a new selection
				StartSelection(m_cursorIndex);
			}

			if (m_ctrlHeld)
			{
				std::string inputText = m_inputFieldText->GetText();
				inputText = inputText.substr(1); // Remove '>'

				// Select the rest of the current token to the left, 
				// or if already at the end of the token select the space to the left + next token
				int endIndex = Max(m_cursorIndex - 1, 0);

				// Walk over white space
				while (endIndex > 0 && inputText[endIndex] == ' ')
				{
					endIndex--;
				}

				// Walk over text
				while (endIndex > 0 && inputText[endIndex] != ' ')
				{
					endIndex--;
				}

				// If we stopped above by finding a whitespace, we actually need to select
				// the character in front of the white space, but don't do this if we just selected *everything* to the left
				if (endIndex > 0)
				{
					endIndex++;
				}

				SetSelectEndIndex(endIndex);
			}
			else
			{
				// Just move the end of the selection normally
				SetSelectEndIndex(m_cursorIndex - 1);
			}
		}	
		else if (HasInputSelection())
		{
			// Break the selection, move the cursor to the left side of the selection
			SetCursor(Min(m_selectionStartIndex, m_selectionEndIndex));
			ResetInputSelection();
		}
		else
		{
			// Just move cursor normally
			MoveCursor(-1);
		}
	}
}


//-------------------------------------------------------------------------------------------------
void DevConsole::HandleRightArrow()
{
	if (m_inputFieldText->IsInFocus())
	{
		std::string inputText = m_inputFieldText->GetText();
		inputText = inputText.substr(1); // Remove '>'

		if (m_shiftHeld)
		{
			if (!HasInputSelection() && m_cursorIndex < (int)inputText.size())
			{
				StartSelection(m_cursorIndex);
			}

			if (m_ctrlHeld)
			{
				// Select the rest of the current token to the right + whitespace, 
				// or if already at the end of the token select the space to the right + next token
				int endIndex = Min(m_cursorIndex + 1, (int)inputText.size());

				// Walk over text
				while (endIndex < (int)inputText.size() && inputText[endIndex] != ' ')
				{
					endIndex++;
				}

				// Walk over whitespace
				while (endIndex < (int)inputText.size() && inputText[endIndex] == ' ')
				{
					endIndex++;
				}

				SetSelectEndIndex(endIndex);
			}
			else
			{
				// Just move the selection per normal
				SetSelectEndIndex(m_cursorIndex + 1);
			}
		}
		else if (HasInputSelection())
		{
			// Break the selection, place the cursor on the right side of the selection
			SetCursor(Max(m_selectionStartIndex, m_selectionEndIndex));
			ResetInputSelection();
		}
		else
		{
			// Move the cursor normally
			MoveCursor(1);
		}
	}
}


//-------------------------------------------------------------------------------------------------
void DevConsole::HandleTab()
{
	if (m_inputFieldText->IsInFocus())
	{
		if (m_popUpState == POP_UP_AUTOCOMPLETE)
		{
			std::string selectedCommandLine = m_popUpText->GetText(m_autocompleteIndex);
			std::string selectedCommand = selectedCommandLine.substr(0, selectedCommandLine.find_first_of(' '));

			m_inputFieldText->SetText(selectedCommand);
			SetCursorToEnd();
			ResetInputSelection();
			UpdateAutoCompleteUI();
		}
	}
}


//-------------------------------------------------------------------------------------------------
void DevConsole::AddCharacterToInputBuffer(const char character)
{
	std::string text;
	text += character;
	AddStringToInputBuffer(text);
}


//-------------------------------------------------------------------------------------------------
void DevConsole::AddStringToInputBuffer(const std::string& text)
{
	if (m_inputFieldText->IsInFocus())
	{
		if (HasInputSelection())
		{
			DeleteSelection();
		}

		std::string inputText = m_inputFieldText->GetText();

		if (m_cursorIndex == (int)inputText.size())
		{
			inputText += text;
		}
		else
		{
			inputText.insert(m_cursorIndex + 1, text);
		}

		m_inputFieldText->SetText(inputText);

		MoveCursor((int)text.size());
		UpdateAutoCompleteUI();
	}
}


//-------------------------------------------------------------------------------------------------
void DevConsole::UpdateInputCursorUI()
{
	if (HasInputSelection())
	{
		std::string text = m_inputFieldText->GetText();

		int lowerIndex = Min(m_selectionStartIndex, m_selectionEndIndex) + 1;
		int upperIndex = Max(m_selectionStartIndex, m_selectionEndIndex) + 1;

		AABB2 lowerCharBounds = m_inputFieldText->GetCharacterLocalBounds(0U, Min(lowerIndex, (int)text.size() - 1));
		AABB2 upperCharBounds = m_inputFieldText->GetCharacterLocalBounds(0U, Min(upperIndex, (int)text.size() - 1));

		// To select the last character, check if our index goes off the end
		float xPos = lowerCharBounds.mins.x;
		float width = (upperIndex == (int)text.size() ? upperCharBounds.maxs.x - lowerCharBounds.mins.x : upperCharBounds.mins.x - lowerCharBounds.mins.x);

		m_inputCursor->m_transform.SetXPosition(xPos);
		m_inputCursor->m_transform.SetWidth(width);
		m_showInputCursor = true;
		m_inputCursor->SetRenderMode(ELEMENT_RENDER_ALL);
	}
	else
	{
		if (!m_inputFieldText->IsInFocus())
		{
			m_inputCursor->SetRenderMode(ELEMENT_RENDER_NONE);
			m_showInputCursor = false;
		}

		// Only update the position when the cursor is shown
		if (m_showInputCursor)
		{
			std::string text = m_inputFieldText->GetText();

			// Find the text the cursor would be placed after
			// + 1 since the ">" isn't counted for the cursor position
			text = text.substr(0, m_cursorIndex + 1);

			Vector2 canvasDimensions = m_inputFieldText->GetTextCanvasDimensions(text);
			m_inputCursor->m_transform.SetXPosition(canvasDimensions.x);
		}
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
	SetCursor(m_cursorIndex + valueToAddToCursor);
}


void DevConsole::SetCursor(int valueToBeSetTo)
{
	// Keep the cursor in bounds
	std::string inputText = m_inputFieldText->GetText();
	int finalValue = Clamp(valueToBeSetTo, 0, (int)inputText.size() - 1);
	m_cursorIndex = finalValue;

	// Reset the cursor and force show it
	m_showInputCursor = true;
	m_inputCursor->SetRenderMode(ELEMENT_RENDER_ALL);
	m_inputCursor->m_transform.SetWidth(m_defaultCursorWidth);
	ResetCursorTimer();

	// Don't reset input selection, as the cursor shows during an empty but in-progress selection
}


//-------------------------------------------------------------------------------------------------
void DevConsole::SetCursorToEnd()
{
	int index = static_cast<int>(m_inputFieldText->GetText().size());
	SetCursor(index);
}


//-------------------------------------------------------------------------------------------------
void DevConsole::ClearInputField()
{
	m_inputFieldText->SetText(">");
	SetCursor(0);
	UpdateAutoCompleteUI();
}


//-------------------------------------------------------------------------------------------------
void DevConsole::StartSelection(int startIndex)
{
	m_selectionStartIndex = startIndex;
}


//-------------------------------------------------------------------------------------------------
void DevConsole::SetSelectEndIndex(int endIndex)
{
	std::string inputText = m_inputFieldText->GetText();
	m_selectionEndIndex = Clamp(endIndex, 0, (int)inputText.size() - 1);

	// Keep the cursor updated
	SetCursor(m_selectionEndIndex);
}


//-------------------------------------------------------------------------------------------------
void DevConsole::ResetInputSelection()
{
	m_selectionStartIndex = -1;
	m_selectionEndIndex = -1;
}


//-------------------------------------------------------------------------------------------------
void DevConsole::ClearLog()
{
	m_logScrollView->GetScrollTextElement()->ClearText();
}

bool SortStuff(const ConsoleCommand* first, const ConsoleCommand* second)
{
	return first->GetID().ToString() < second->GetID().ToString();
}

//-------------------------------------------------------------------------------------------------
void DevConsole::UpdateAutoCompleteUI()
{
	std::string inputText = m_inputFieldText->GetText();
	inputText = inputText.substr(1);

	if (inputText.size() > 0)
	{
		std::vector<const ConsoleCommand*> commands;
		std::string typedCommandName = inputText.substr(0, inputText.find_first_of(' '));
		ConsoleCommand::GetAllCommandsWithIDPrefix(typedCommandName.c_str(), commands);

		// Alphabetical order
		std::sort(commands.begin(), commands.end(), SortStuff);

		if (commands.size() > 0)
		{
			// If there's only one matching event name and we already typed it out, show the usage for that command
			if (commands.size() == 1 && commands[0]->GetID() == SID(typedCommandName))
			{
				m_popUpText->ClearText();
				m_popUpText->SetText(">" + commands[0]->GetUsage(), Rgba::WHITE);
				m_popUpPanel->SetRenderMode(ELEMENT_RENDER_ALL);
				m_popUpState = POP_UP_USAGE;
			}
			else
			{
				// Fill the popup text window with the command ids + descriptions
				for (int lineIndex = 0; lineIndex < static_cast<int>(commands.size()); ++lineIndex)
				{
					// Set where the cursor starts as yellow
					Rgba lineColor = Rgba::WHITE;

					if (lineIndex == 0)
					{
						lineColor = Rgba::YELLOW;
					}

					std::string lineText = commands[lineIndex]->GetIDWithDescription();
					m_popUpText->SetLine(lineIndex, lineText, lineColor);
				}

				// Yellow is where the selection cursor is
				m_popUpPanel->SetRenderMode(ELEMENT_RENDER_ALL);
				m_popUpState = POP_UP_AUTOCOMPLETE;

				float totalHeight = m_popUpText->GetTotalLinesHeight();
				m_popUpPanel->m_transform.SetHeight(totalHeight);
			}
		}
		else
		{
			m_popUpText->ClearText();
			m_popUpPanel->SetRenderMode(ELEMENT_RENDER_NONE);
			m_popUpState = POP_UP_HIDDEN;
		}
	}
	else
	{
		m_popUpText->ClearText();
		m_popUpPanel->SetRenderMode(ELEMENT_RENDER_NONE);
		m_popUpState = POP_UP_HIDDEN;
	}

	m_autocompleteIndex = 0;
}


//-------------------------------------------------------------------------------------------------
void DevConsole::DeleteSelection()
{
	std::string inputText = m_inputFieldText->GetText();

	int lowerIndex = Min(m_selectionStartIndex, m_selectionEndIndex) + 1;
	int upperIndex = Max(m_selectionStartIndex, m_selectionEndIndex) + 1;
	int numSelectedChars = upperIndex - lowerIndex;

	std::string remainingText;

	// Keep before the selection
	if (lowerIndex > 0)
	{
		remainingText += inputText.substr(0, lowerIndex);
	}

	// Keep after the selection
	if (m_selectionEndIndex < (int)inputText.size())
	{
		remainingText += inputText.substr(lowerIndex + numSelectedChars);
	}

	// Update
	m_inputFieldText->SetText(remainingText);
	ResetInputSelection();
	SetCursor(lowerIndex - 1); // -1 to undo the +1 above
}


//-------------------------------------------------------------------------------------------------
void DevConsole::CopyInputIntoClipboard()
{
	if (m_inputFieldText->IsInFocus() && HasInputSelection())
	{
		std::string inputSelection = GetSelectedInputText();

		HGLOBAL textMem = GlobalAlloc(GMEM_MOVEABLE, inputSelection.size() + 1);

		memcpy(GlobalLock(textMem), inputSelection.c_str(), inputSelection.size() + 1);
		GlobalUnlock(textMem);

		OpenClipboard(NULL);
		EmptyClipboard();
		SetClipboardData(CF_TEXT, textMem);
		CloseClipboard();
	}
}


//-------------------------------------------------------------------------------------------------
void DevConsole::PasteFromClipboard()
{
	if (m_inputFieldText->IsInFocus())
	{
		std::string inputText = m_inputFieldText->GetText();

		OpenClipboard(NULL);
		HGLOBAL textMem = GetClipboardData(CF_TEXT);

		if (textMem != NULL)
		{
			GlobalLock(textMem);

			const char* text = static_cast<const char*>(textMem);
			AddStringToInputBuffer(text);

			GlobalUnlock(textMem);
		}

		CloseClipboard();
	}
}
