///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 28th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "Engine/Event/EventSystem.h"
#include "Engine/Framework/EngineCommon.h"
#include "Engine/Framework/Window.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Utility/NamedProperties.h"

///-------e-------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------
Window* g_window = nullptr;

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
LRESULT CALLBACK WindowsMessageHandlingProcedure(HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam)
{
	//  Give the custom handlers a chance to run first; 
	if (g_window == nullptr)
		return DefWindowProc(windowHandle, wmMessageCode, wParam, lParam);

	const std::vector<WindowsMessageHandler>& handlers = g_window->GetHandlers();

	bool msgConsumed = false;
	for (int i = 0; i < static_cast<int>(handlers.size()); ++i)
	{
		bool handlerConsumedMsg = handlers[i](wmMessageCode, wParam, lParam);
		msgConsumed = msgConsumed || handlerConsumedMsg;
	}
	
	if (!msgConsumed)
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
	//wndClassDesc.hbrBackground = (HBRUSH)COLOW_WINDOW;
	RegisterClassEx(&wndClassDesc);

	return wndClassDesc;
}


//-------------------------------------------------------------------------------------------------
void DetermineWindowAndClientBounds(float clientAspect, const DWORD windowStyleFlags, const DWORD windowStyleExFlags, RECT& out_windowRect, RECT& out_clientRect)
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

	out_clientRect.left = (int)clientMarginX;
	out_clientRect.right = out_clientRect.left + (int)clientWidth;
	out_clientRect.top = (int)clientMarginY;
	out_clientRect.bottom = out_clientRect.top + (int)clientHeight;

	// Calculate the outer dimensions of the physical window, including frame et. al.
	out_windowRect = out_clientRect;
	AdjustWindowRectEx(&out_windowRect, windowStyleFlags, FALSE, windowStyleExFlags);
}


//-------------------------------------------------------------------------------------------------
static bool WindowMessageHandler(unsigned int msg, size_t wParam, size_t lParam)
{
	UNUSED(wParam);
	UNUSED(lParam);

	switch (msg)
	{
	case WM_SIZE: // Window was resized
	{
		g_window->ResizeWindowToWindowsRect();
		return true;
	}
	}

	return false;
}


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS IMPLEMENTATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
Window::Window(float aspect, const char* windowTitle)
{
	WNDCLASSEX wndClassDesc = CreateWindowClassDescription();

	const DWORD windowStyleFlags = WS_OVERLAPPEDWINDOW;
	const DWORD windowStyleExFlags = WS_EX_APPWINDOW;

	RECT windowRect, clientRect;
	DetermineWindowAndClientBounds(aspect, windowStyleFlags, windowStyleExFlags, windowRect, clientRect);

	WCHAR titleBuffer[1024];
	MultiByteToWideChar(GetACP(), 0, windowTitle, -1, titleBuffer, sizeof(titleBuffer) / sizeof(titleBuffer[0]));

	m_hwnd = CreateWindowEx(
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

	ShowWindow((HWND)m_hwnd, SW_SHOW);
	SetForegroundWindow((HWND)m_hwnd);
	SetFocus((HWND)m_hwnd);

	// Set members
	m_windowPixelBounds.mins.x = static_cast<float>(windowRect.left);
	m_windowPixelBounds.maxs.x = static_cast<float>(windowRect.right);
	m_windowPixelBounds.mins.y = static_cast<float>(windowRect.top); // Windows' screen coordinates (0,0) is top left
	m_windowPixelBounds.maxs.y = static_cast<float>(windowRect.bottom);

	m_clientPixelBounds.mins.x = static_cast<float>(clientRect.left);
	m_clientPixelBounds.maxs.x = static_cast<float>(clientRect.right);
	m_clientPixelBounds.mins.y = static_cast<float>(clientRect.top);
	m_clientPixelBounds.maxs.y = static_cast<float>(clientRect.bottom);

	m_windowTitle = windowTitle;
}


//-------------------------------------------------------------------------------------------------
Window::~Window()
{
	ASSERT_RECOVERABLE(m_messageHandlers.size() == 0, "Not all windows message handlers were unsubscribed!");
	m_messageHandlers.clear();
}


//-------------------------------------------------------------------------------------------------
void Window::Initialize(float aspect, const char* windowTitle)
{
	g_window = new Window(aspect, windowTitle);
	g_window->RegisterMessageHandler(WindowMessageHandler);

	// Hack to get around Visual Studio constantly sending the program to bottom on start up...
	g_eventSystem->SubscribeEventCallbackObjectMethod("bring_to_front", &Window::BringWindowToFront, *g_window);
	QueueDelayedEvent("bring_to_front", 0.25f);
}


//-------------------------------------------------------------------------------------------------
void Window::Shutdown()
{
	g_eventSystem->UnsubscribeEventCallbackObjectMethod("bring_to_front", &Window::BringWindowToFront, *g_window);
	g_window->UnregisterMessageHandler(WindowMessageHandler);
	SAFE_DELETE(g_window);
}


//-------------------------------------------------------------------------------------------------
void Window::ResizeWindowToWindowsRect()
{
	RECT clientRect;
	GetClientRect((HWND)m_hwnd, &clientRect);
	ClientToScreen((HWND)m_hwnd, (POINT *)&clientRect);
	ClientToScreen((HWND)m_hwnd, (POINT *)&clientRect + 1);
	m_clientPixelBounds.mins.x = static_cast<float>(clientRect.left);
	m_clientPixelBounds.maxs.x = static_cast<float>(clientRect.right);
	m_clientPixelBounds.mins.y = static_cast<float>(clientRect.top);
	m_clientPixelBounds.maxs.y = static_cast<float>(clientRect.bottom);

	RECT windowRect;
	GetWindowRect((HWND)m_hwnd, &windowRect);
	m_windowPixelBounds.mins.x = static_cast<float>(windowRect.left);
	m_windowPixelBounds.maxs.x = static_cast<float>(windowRect.right);
	m_windowPixelBounds.mins.y = static_cast<float>(windowRect.top); // Windows' screen coordinates (0,0) is top left
	m_windowPixelBounds.maxs.y = static_cast<float>(windowRect.bottom);

	NamedProperties args;
	args.Set("client-bounds", m_clientPixelBounds);
	args.Set("client-aspect", GetClientAspect());
	args.Set("client-width", static_cast<int>(m_clientPixelBounds.GetWidth()));
	args.Set("client-height", static_cast<int>(m_clientPixelBounds.GetHeight()));

	FireEvent("window-resize", args);
}


//-------------------------------------------------------------------------------------------------
bool Window::BringWindowToFront(NamedProperties& args)
{
	UNUSED(args);
	BringWindowToTop((HWND)m_hwnd);

	return true;
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
