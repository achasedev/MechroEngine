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
#include "Engine/Utility/EngineUtils.h"
#include "Engine/Utility/NamedProperties.h"
#include "Engine/Render/Font/Font.h"
#include "Engine/Render/Font/FontLoader.h"
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
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
static void ConsolePrintv(const Rgba& color, char const* format, va_list args)
{
	ASSERT_RETURN(g_devConsole != nullptr, NO_RETURN_VAL, "DevConsole not initialized!");

	char textLiteral[VARIABLE_ARG_STACK_LOCAL_TEMP_LENGTH];
	vsnprintf_s(textLiteral, VARIABLE_ARG_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, args);
	textLiteral[VARIABLE_ARG_STACK_LOCAL_TEMP_LENGTH - 1] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	// Add it to the console log
	ColoredText colorText;
	colorText.m_text = std::string(textLiteral);
	colorText.m_color = color;

	g_devConsole->AddToMessageQueue(colorText);
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


static bool AddTwoNumbers(NamedProperties& args)
{
	std::string argsText = args.Get("args", "");
	std::vector<std::string> tokens;
	Tokenize(argsText, ' ', tokens);

	float a = StringToFloat(tokens[0]);
	float b = StringToFloat(tokens[1]);

	ConsolePrintf("%.2f + %.2f = %.2f", a, b, a + b);

	return true;
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
	m_popupImage = m_canvas->FindElementAsType<UIImage>(SID("popup_image"));
	m_popupText = m_canvas->FindElementAsType<UIText>(SID("popup_text"));
	m_popupText->SetRenderMode(ELEMENT_RENDER_NONE);
	m_fpsText = m_canvas->FindElementAsType<UIText>(SID("fps_text"));

	m_inputFieldText->SetShader(m_shader);
	m_inputCursor->SetShader(m_shader);
	m_popupText->SetShader(m_shader);
	m_logScrollView->GetScrollTextElement()->SetShader(m_shader);
	m_fpsText->SetShader(m_shader);

	m_inputFieldText->m_onMouseClick = OnMouseClick_InputField;
	m_inputFieldText->m_onMouseHold = OnMouseHold_InputField;
	m_inputFieldText->m_onMouseRelease = OnMouseRelease_InputField;
	m_inputCursor->m_onMouseClick = PassThroughMouseInput;
	m_inputCursor->m_onMouseRelease = PassThroughMouseInput;
	m_inputFieldText->SetText(">");
	m_defaultCursorWidth = m_inputCursor->m_transform.GetWidth();
	SetCursor(0);

	m_fpsUpdateTimer.SetInterval(0.5f);

	g_eventSystem->SubscribeEventCallbackFunction("add", AddTwoNumbers);
	g_eventSystem->SubscribeEventCallbackFunction("adder", AddTwoNumbers);
	g_eventSystem->SubscribeEventCallbackFunction("adding", AddTwoNumbers);
	g_eventSystem->SubscribeEventCallbackFunction("adda", AddTwoNumbers);
	g_eventSystem->SubscribeEventCallbackFunction("addt", AddTwoNumbers);
	g_eventSystem->SubscribeEventCallbackFunction("addg", AddTwoNumbers);
	g_eventSystem->SubscribeEventCallbackFunction("addfdsa", AddTwoNumbers);
	g_eventSystem->SubscribeEventCallbackFunction("add0", AddTwoNumbers);
	g_eventSystem->SubscribeEventCallbackFunction("add-", AddTwoNumbers);
}


//-------------------------------------------------------------------------------------------------
DevConsole::~DevConsole()
{
	m_fpsText = nullptr;
	m_popupText = nullptr;
	m_popupImage = nullptr;
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
	if (m_inputFieldText->IsInFocus())
	{
		std::string input = m_inputFieldText->GetText();
		TrimWhitespace(input);

		// Trim the ">"
		std::string command = input.substr(1);
		TrimWhitespace(command);

		if (command.size() > 0)
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
				ConsoleWarningf("Unknown command: %s", commandID.c_str());
			}

			ClearInputField();
		}
	}
}


//-------------------------------------------------------------------------------------------------
void DevConsole::HandleBackSpace()
{
	if (m_inputFieldText->IsInFocus() && m_cursorPosition > 0)
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
				inputText.erase(inputText.begin() + m_cursorPosition);
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

			if (m_cursorPosition < (int)inputText.size() - 1)
			{
				if (inputText.size() > 1)
				{
					inputText.erase(inputText.begin() + m_cursorPosition + 1);
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
		if (m_autocompleteShown)
		{
			m_popupText->SetColor(m_autocompleteIndex, DEFAULT_CONSOLE_LOG_COLOR);
			m_autocompleteIndex = Max(0, m_autocompleteIndex - 1);
			m_popupText->SetColor(m_autocompleteIndex, Rgba::YELLOW);
		}
		else if (m_commandHistory.size() > 0)
		{
			m_historyIndex = Max(m_historyIndex - 1, 0);

			std::string command = m_commandHistory[m_historyIndex];
			m_inputFieldText->SetText(">" + command);
			m_cursorPosition = (int)command.size();
		}
	}
}


//-------------------------------------------------------------------------------------------------
void DevConsole::HandleDownArrow()
{
	if (m_inputFieldText->IsInFocus())
	{
		if (m_autocompleteShown)
		{
			int numLines = (int)m_popupText->GetNumLines();

			m_popupText->SetColor(m_autocompleteIndex, DEFAULT_CONSOLE_LOG_COLOR);
			m_autocompleteIndex = Min(m_autocompleteIndex + 1, numLines - 1);
			m_popupText->SetColor(m_autocompleteIndex, Rgba::YELLOW);
		}
		else if (m_commandHistory.size() > 0)
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
}


//-------------------------------------------------------------------------------------------------
void DevConsole::HandleLeftArrow()
{
	if (m_inputFieldText->IsInFocus())
	{
		ResetInputSelection();
		MoveCursor(-1);
	}
}


//-------------------------------------------------------------------------------------------------
void DevConsole::HandleRightArrow()
{
	if (m_inputFieldText->IsInFocus())
	{
		ResetInputSelection();
		MoveCursor(1);
	}
}


//-------------------------------------------------------------------------------------------------
void DevConsole::HandleTab()
{
	if (m_inputFieldText->IsInFocus())
	{
		if (m_autocompleteShown)
		{
			std::string selectedCommand = m_popupText->GetText(m_autocompleteIndex);
			m_inputFieldText->SetText(selectedCommand);
			SetCursorToEnd();
			ResetInputSelection();
			UpdateAutoCompleteUI();
		}
	}
}


//-------------------------------------------------------------------------------------------------
void DevConsole::AddCharacterToInputBuffer(unsigned char character)
{
	if (m_inputFieldText->IsInFocus())
	{
		if (HasInputSelection())
		{
			DeleteSelection();
		}

		std::string inputText = m_inputFieldText->GetText();
		inputText.insert(inputText.begin() + m_cursorPosition + 1, character);
		m_inputFieldText->SetText(inputText);

		MoveCursor(1);
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
			text = text.substr(0, m_cursorPosition + 1);

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
	m_inputCursor->m_transform.SetWidth(m_defaultCursorWidth);
	ResetCursorTimer();

	// Don't reset input selection, as the cursor shows during an empty but in-progress selection
}


//-------------------------------------------------------------------------------------------------
void DevConsole::SetCursorToEnd()
{
	int index = m_inputFieldText->GetText().size();
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
	m_isSelecting = true;
}


//-------------------------------------------------------------------------------------------------
void DevConsole::SetSelectEndIndex(int endIndex)
{
	m_selectionEndIndex = endIndex;

	// Keep a blinking cursor if the selection is empty
	if (m_selectionStartIndex == m_selectionEndIndex)
	{
		SetCursor(m_selectionStartIndex);
	}
}


//-------------------------------------------------------------------------------------------------
void DevConsole::ResetInputSelection()
{
	m_selectionStartIndex = -1;
	m_selectionEndIndex = -1;
	m_isSelecting = false;
}


//-------------------------------------------------------------------------------------------------
void DevConsole::UpdateAutoCompleteUI()
{
	std::string inputText = m_inputFieldText->GetText();
	inputText = inputText.substr(1);

	if (inputText.size() > 0)
	{
		std::vector<std::string> eventNames;
		g_eventSystem->GetAllEventNamesThatStartWithPrefix(inputText.c_str(), eventNames);
		std::sort(eventNames.begin(), eventNames.end());

		if (eventNames.size() > 0)
		{
			// If there's only one matching event name and we already typed it out, hide the auto complete
			if (eventNames.size() == 1 && eventNames[0] == inputText)
			{
				m_popupText->ClearText();
				m_popupText->SetRenderMode(ELEMENT_RENDER_NONE);
				m_autocompleteShown = false;
			}
			else
			{
				// Add an ">" to all the lines
				for (size_t nameIndex = 0; nameIndex < eventNames.size(); ++nameIndex)
				{
					std::string& currName = eventNames[nameIndex];
					currName = ">" + currName;
				}

				m_popupText->SetLines(eventNames, DEFAULT_CONSOLE_LOG_COLOR);
				m_popupText->SetColor(0, Rgba::YELLOW);
				m_popupText->SetRenderMode(ELEMENT_RENDER_ALL);
				m_autocompleteShown = true;

				float totalHeight = m_popupText->GetTotalLinesHeight();
				m_popupText->m_transform.SetHeight(totalHeight);
			}
		}
		else
		{
			m_popupText->ClearText();
			m_popupText->SetRenderMode(ELEMENT_RENDER_NONE);
			m_autocompleteShown = false;
		}
	}
	else
	{
		m_popupText->ClearText();
		m_popupText->SetRenderMode(ELEMENT_RENDER_NONE);
		m_autocompleteShown = false;
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
