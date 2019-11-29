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

#include "Engine/Framework/Window.h"

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

///--------------------------------------------------------------------------------------------------------------------------------------------------
///                                                           *** C FUNCTIONS ***
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
LRESULT CALLBACK WindowsMessageHandlingProcedure(HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam)
{
	switch (wmMessageCode)
	{
	case WM_CLOSE:
	{
		g_isQuitting = true;
		return 0;
	}

	case WM_KEYDOWN:
	{
		unsigned char asKey = (unsigned char)wParam;
		if (asKey == VK_ESCAPE)
		{
			g_isQuitting = true;
			return 0;
		}
		break;
	}

	case WM_KEYUP:
	{
		break;
	}
	}

	return DefWindowProc(windowHandle, wmMessageCode, wParam, lParam);
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
void Window::Initialize()
{
	WNDCLASSEX wndClassDesc = CreateWindowClassDescription();
	
	const DWORD windowStyleFlags = WS_OVERLAPPEDWINDOW | WS_MAXIMIZE;
	const DWORD windowStyleExFlags = WS_EX_APPWINDOW;

	RECT windowRect = DetermineWindowBounds(1.77777f, windowStyleFlags, windowStyleExFlags);
	memset(&windowRect, 0, sizeof(RECT));

	WCHAR windowTitle[1024];
	MultiByteToWideChar(GetACP(), 0, m_windowTitle.c_str(), -1, windowTitle, sizeof(windowTitle) / sizeof(windowTitle[0]));
	m_windowContext = CreateWindowEx(
		windowStyleExFlags,
		wndClassDesc.lpszClassName,
		windowTitle,
		windowStyleFlags,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		GetModuleHandle(NULL),
		NULL);

	ShowWindow((HWND)m_windowContext, SW_SHOW);
	SetForegroundWindow((HWND)m_windowContext);
	SetFocus((HWND)m_windowContext);
}
