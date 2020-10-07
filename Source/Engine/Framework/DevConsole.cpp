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
#include "Engine/Render/Camera/Camera.h"
#include "Engine/Render/Core/RenderContext.h"
#include "Engine/Render/Material.h"
#include "Engine/Render/Shader.h"
#include "Engine/Render/Texture/Texture2D.h"
#include "Engine/UI/Canvas.h"
#include "Engine/UI/Panel.h"
#include "Engine/UI/UIText.h"
#include "Engine/Utility/NamedProperties.h"
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
DevConsole::DevConsole()
{
	m_canvas = new Canvas();
	m_canvas->Initialize(g_renderContext->GetDefaultRenderTarget(), Vector2(1000.f * g_window->GetClientAspect(), 1000.f), SCREEN_MATCH_WIDTH_OR_HEIGHT);

	// TODO: Remove these when ResourceManager is going
	Shader* shader = new Shader();
	shader->CreateFromFile("Data/Shader/test.shader");
	shader->SetBlend(BLEND_PRESET_ALPHA);

	// Texture
	Image image(IntVector2(2));
	//image.LoadFromFile("Data/Image/test.png");

	Texture2D* texture = new Texture2D();
	texture->CreateFromImage(image);
	ShaderResourceView* textureView = texture->CreateOrGetShaderResourceView();

	// Combine into default material
	Material* material = new Material();
	material->SetShader(shader);
	material->SetAlbedoTextureView(textureView);

	Font* font = g_fontLoader->LoadFont("Data/Font/bitwise.regular.ttf", 0);

	const float kFontHeight = 25.f; // Canvas units
	m_backPanel = new Panel(m_canvas, material);
	m_backPanel->m_transform.SetAnchors(AnchorPreset::TOP_LEFT);
	m_backPanel->m_transform.SetPivot(Vector2(0.f, 1.0f));
	m_backPanel->m_transform.SetPosition(Vector2::ZERO);
	m_backPanel->m_transform.SetDimensions(m_canvas->GetResolution() - Vector2(0.f, kFontHeight));

	m_inputPanel = new Panel(m_canvas, material);
	m_inputPanel->m_transform.SetAnchors(AnchorPreset::BOTTOM_LEFT);
	m_inputPanel->m_transform.SetPivot(Vector2::ZERO);
	m_inputPanel->m_transform.SetPosition(Vector2::ZERO);
	m_inputPanel->m_transform.SetDimensions(Vector2(m_canvas->GetResolution().x, kFontHeight));

	m_inputFieldText = new UIText(m_canvas);
	m_inputFieldText->SetFont(font, shader);
	m_inputFieldText->SetText("The quick brown fox jump over the lazy dog!", Rgba::RED);
	m_inputFieldText->m_transform.SetAnchors(AnchorPreset::BOTTOM_LEFT);
	m_inputFieldText->m_transform.SetPivot(Vector2::ZERO);
	m_inputFieldText->m_transform.SetPosition(Vector2::ZERO);
	m_inputFieldText->m_transform.SetDimensions(Vector2(m_canvas->GetResolution().x, kFontHeight));

	m_canvas->AddChild(m_backPanel);	
	m_canvas->AddChild(m_inputPanel);
	m_inputPanel->AddChild(m_inputFieldText);
}


//-------------------------------------------------------------------------------------------------
DevConsole::~DevConsole()
{
	SAFE_DELETE(m_canvas);
}


//-------------------------------------------------------------------------------------------------
void DevConsole::HandleTilde()
{
	m_isActive = !m_isActive;
}


//-------------------------------------------------------------------------------------------------
void DevConsole::HandleEnter()
{
	if (m_inputBuffer.size() > 0)
	{
		// Print the command to the console log

		// Add the command to history


		// Parse the command arguments from the command name
		// Let the event parse the individual commands, as each 
		// command may expect/treat arguments as different types
		// This also preserves the order of the arguments
		//NamedProperties args;
		//size_t firstSpaceIndex = m_inputBuffer.find_first_of(' ');

		//std::string eventName;
		//std::string argString;

		//if (firstSpaceIndex < m_inputBuffer.size() - 1)
		//{
		//	args.Set("args", m_inputBuffer.substr());

		//}
		//else
		//{

		//}
		//StringID eventID = ParseCommandLine(m_inputBuffer, args);

		//// Reset input buffer
		//m_inputBuffer.clear();
		//m_cursorPosition = 0;

		//// Run the command
		//FireEvent(eventID, args);
	}
}


//-------------------------------------------------------------------------------------------------
void DevConsole::HandleBackSpace()
{

}


//-------------------------------------------------------------------------------------------------
void DevConsole::HandleDelete()
{

}


//-------------------------------------------------------------------------------------------------
void DevConsole::HandleEscape()
{

}


//-------------------------------------------------------------------------------------------------
void DevConsole::HandleUpArrow()
{

}


//-------------------------------------------------------------------------------------------------
void DevConsole::HandleDownArrow()
{

}

//-------------------------------------------------------------------------------------------------
void DevConsole::HandleLeftArrow()
{
	// Move the cursor to the left, not going beyond right before the first character
	//m_cursorPosition--;
	//m_cursorPosition = ClampInt(m_cursorPosition, 0, static_cast<int>(m_inputBuffer.size()));
}


//-------------------------------------------------------------------------------------------------
void DevConsole::HandleRightArrow()
{
	// Move the cursor to the right, not going pass the very end of the string
	//m_cursorPosition++;
	//m_cursorPosition = ClampInt(m_cursorPosition, 0, static_cast<int>(m_inputBuffer.size()));
}


//-------------------------------------------------------------------------------------------------
void DevConsole::AddCharacterToInputBuffer(unsigned char character)
{

}
