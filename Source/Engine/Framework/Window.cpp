
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif	// Always #define this before #including <windows.h>
#include <windows.h>	
#include "Engine/Framework/Window.h"

//-------------------------------------------------------------------------------------------------
LRESULT CALLBACK WindowsMessageHandlingProcedure(HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam)
{
	switch (wmMessageCode)
	{
		// App close requested via "X" button, or right-click "Close Window" on task bar, or "Close" from system menu, or Alt-F4
	case WM_CLOSE:
	{
		g_isQuitting = true;
		return 0; // "Consumes" this message (tells Windows "okay, we handled it")
	}

	// Raw physical keyboard "key-was-just-depressed" event (case-insensitive, not translated)
	case WM_KEYDOWN:
	{
		unsigned char asKey = (unsigned char)wParam;
		if (asKey == VK_ESCAPE)
		{
			g_isQuitting = true;
			return 0; // "Consumes" this message (tells Windows "okay, we handled it")
		}
		break;
	}

	// Raw physical keyboard "key-was-just-released" event (case-insensitive, not translated)
	case WM_KEYUP:
	{
		//			unsigned char asKey = (unsigned char) wParam;
		break;
	}
	}

	// Send back to Windows any unhandled/unconsumed messages we want other apps to see (e.g. play/pause in music apps, etc.)
	return DefWindowProc(windowHandle, wmMessageCode, wParam, lParam);
}

//-------------------------------------------------------------------------------------------------
WNDCLASSEX CreateWindowClassDescription()
{
	WNDCLASSEX wndClassDesc;
	memset(&wndClassDesc, 0, sizeof(wndClassDesc));

	wndClassDesc.cbSize = sizeof(wndClassDesc);
	wndClassDesc.style = CS_OWNDC; // Redraw on move, request own Display Context
	wndClassDesc.lpfnWndProc = WindowsMessageHandlingProcedure;
	wndClassDesc.hInstance = GetModuleHandle(NULL);
	wndClassDesc.hIcon = NULL;
	wndClassDesc.hCursor = NULL;
	wndClassDesc.lpszClassName = TEXT("Window Class");

	RegisterClassEx(&wndClassDesc);

	return wndClassDesc;
}

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

//-------------------------------------------------------------------------------------------------
void Window::Initialize()
{
	WNDCLASSEX wndClassDesc = CreateWindowClassDescription();
	
	const DWORD windowStyleFlags = WS_OVERLAPPEDWINDOW; // | WS_MAXIMIZE;
	const DWORD windowStyleExFlags = WS_EX_APPWINDOW;

	RECT windowRect = DetermineWindowBounds(1.77777f, windowStyleFlags, windowStyleExFlags);

	WCHAR windowTitle[1024];
	MultiByteToWideChar(GetACP(), 0, "Just a dummy name", -1, windowTitle, sizeof(windowTitle) / sizeof(windowTitle[0]));
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

	HCURSOR cursor = LoadCursor(NULL, IDC_ARROW);
	SetCursor(cursor);
}
