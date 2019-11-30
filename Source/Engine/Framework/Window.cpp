///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 28th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** INCLUDES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Framework/Window.h"
#include "Engine/Math/MathUtils.h"

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** DEFINES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                              *** TYPES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** STRUCTS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                        *** GLOBALS AND STATICS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------
Window* Window::s_instance = nullptr;

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                           *** C FUNCTIONS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
LRESULT CALLBACK WindowsMessageHandlingProcedure(HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam)
{
	//  Give the custom handlers a chance to run first; 
	Window* window = Window::GetInstance();
	if (!window)
		return 0;

	const std::vector<WindowsMessageHandler>& handlers = window->GetHandlers();

	bool returnDefaultProc = true;
	for (int i = 0; i < static_cast<int>(handlers.size()); ++i)
	{
		// If any return true then do default windows behavior
		returnDefaultProc = handlers[i](wmMessageCode, wParam, lParam) && returnDefaultProc;
	}
	

	if (returnDefaultProc)
	{
		return DefWindowProc(windowHandle, wmMessageCode, wParam, lParam);
	}
	
	return 0;
}


//-------------------------------------------------------------------------------------------------
WNDCLASSEX CreateWindowClassDescription()
{
	WNDCLASSEX wndClassDesc;
	memset(&wndClassDesc, 0, sizeof(wndClassDesc));

	wndClassDesc.cbSize = sizeof(wndClassDesc);
	wndClassDesc.style = CS_OWNDC;
	wndClassDesc.lpfnWndProc = WindowsMessageHandlingProcedure;
	wndClassDesc.hInstance = GetModuleHandle(NULL);
	wndClassDesc.hIcon = NULL;
	wndClassDesc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClassDesc.lpszClassName = TEXT("Window Class");

	RegisterClassEx(&wndClassDesc);

	return wndClassDesc;
}


//-------------------------------------------------------------------------------------------------
RECT DetermineWindowBounds(float clientAspect, const DWORD windowStyleFlags, const DWORD windowStyleExFlags)
{
	RECT desktopRect;
	HWND desktopWindowHandle = GetDesktopWindow();
	GetClientRect(desktopWindowHandle, &desktopRect);
	float desktopWidth = (float)(desktopRect.right - desktopRect.left);
	float desktopHeight = (float)(desktopRect.bottom - desktopRect.top);
	float desktopAspect = desktopWidth / desktopHeight;

	// Calculate maximum client size (as some % of desktop size)
	constexpr float maxClientFractionOfDesktop = 0.90f;
	float clientWidth = desktopWidth * maxClientFractionOfDesktop;
	float clientHeight = desktopHeight * maxClientFractionOfDesktop;
	if (clientAspect > desktopAspect)
	{
		// Client window has a wider aspect than desktop; shrink client height to match its width
		clientHeight = clientWidth / clientAspect;
	}
	else
	{
		// Client window has a taller aspect than desktop; shrink client width to match its height
		clientWidth = clientHeight * clientAspect;
	}

	// Calculate client rect bounds by centering the client area
	float clientMarginX = 0.5f * (desktopWidth - clientWidth);
	float clientMarginY = 0.5f * (desktopHeight - clientHeight);

	RECT clientRect;
	clientRect.left = (int)clientMarginX;
	clientRect.right = clientRect.left + (int)clientWidth;
	clientRect.top = (int)clientMarginY;
	clientRect.bottom = clientRect.top + (int)clientHeight;

	// Calculate the outer dimensions of the physical window, including frame et. al.
	RECT windowRect = clientRect;
	AdjustWindowRectEx(&windowRect, windowStyleFlags, FALSE, windowStyleExFlags);

	return windowRect;
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                             *** CLASSES ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
Window::Window(float aspect, const char* windowTitle)
{
	WNDCLASSEX wndClassDesc = CreateWindowClassDescription();

	const DWORD windowStyleFlags = WS_OVERLAPPEDWINDOW | WS_MAXIMIZE;
	const DWORD windowStyleExFlags = WS_EX_APPWINDOW;

	RECT windowRect = DetermineWindowBounds(aspect, windowStyleFlags, windowStyleExFlags);

	WCHAR titleBuffer[1024];
	MultiByteToWideChar(GetACP(), 0, windowTitle, -1, titleBuffer, sizeof(titleBuffer) / sizeof(titleBuffer[0]));

	SetLastError(0);
	HINSTANCE hInstance = GetModuleHandle(NULL);
	m_windowContext = CreateWindowEx(
		windowStyleExFlags,
		wndClassDesc.lpszClassName,
		titleBuffer,
		windowStyleFlags,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		GetModuleHandle(NULL),
		NULL);

	DWORD error = GetLastError();
	ShowWindow((HWND)m_windowContext, SW_SHOW);
	SetForegroundWindow((HWND)m_windowContext);
	SetFocus((HWND)m_windowContext);

	// Set members
	m_pixelBounds.mins.x = static_cast<float>(windowRect.left);
	m_pixelBounds.maxs.x = static_cast<float>(windowRect.right);
	m_pixelBounds.mins.y = static_cast<float>(windowRect.top); // Windows' screen coordinates (0,0) is top left
	m_pixelBounds.maxs.y = static_cast<float>(windowRect.bottom);
	m_windowTitle = windowTitle;
}


//-------------------------------------------------------------------------------------------------
Window::~Window()
{
	m_messageHandlers.clear();
}


//-------------------------------------------------------------------------------------------------
void Window::Initialize(float aspect, const char* windowTitle)
{
	s_instance = new Window(aspect, windowTitle);
}


//-------------------------------------------------------------------------------------------------
void Window::ShutDown()
{
	delete s_instance;
	s_instance = nullptr;
}


//-------------------------------------------------------------------------------------------------
void Window::SetWindowPixelBounds(const AABB2& newBounds)
{
	SetWindowPos((HWND)m_windowContext,
		NULL,
		RoundToNearestInt(newBounds.mins.x),
		RoundToNearestInt(newBounds.mins.x),
		RoundToNearestInt(newBounds.mins.x),
		RoundToNearestInt(newBounds.mins.x),
		NULL);

	m_pixelBounds = newBounds;
}


//-------------------------------------------------------------------------------------------------
void Window::RegisterMessageHandler(WindowsMessageHandler handler)
{
	// Check for duplicates
	bool alreadyExists = false;
	int cbCount = (int)m_messageHandlers.size();

	for (int i = 0; i < cbCount; ++i)
	{
		if (m_messageHandlers[i] == handler)
		{
			alreadyExists = true;
		}
	}

	ASSERT_RECOVERABLE(!alreadyExists, "Window::RegisterMessageHandler() received duplicate message handler!");

	if (!alreadyExists)
	{
		m_messageHandlers.push_back(handler);
	}
}


//-------------------------------------------------------------------------------------------------
void Window::UnregisterMessageHandler(WindowsMessageHandler handler)
{
	int cbCount = (int)m_messageHandlers.size();

	for (int i = 0; i < cbCount; ++i)
	{
		if (m_messageHandlers[i] == handler)
		{
			m_messageHandlers.erase(m_messageHandlers.begin() + i);
			break;
		}
	}
}
