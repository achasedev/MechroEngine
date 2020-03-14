///--------------------------------------------------------------------------------------------------------------------------------------------------
/// Author: Andrew Chase
/// Date Created: November 29th, 2019
/// Description: 
///--------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// INCLUDES
///--------------------------------------------------------------------------------------------------------------------------------------------------
#include "Engine/Math/AABB2.h"
#include <string>
#include <vector>

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// DEFINES
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// ENUMS, TYPEDEFS, STRUCTS, FORWARD DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

// Returns true when the handler consumes the message
typedef bool(*WindowsMessageHandler)(unsigned int msg, size_t wparam, size_t lparam);

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// GLOBALS AND STATICS
///--------------------------------------------------------------------------------------------------------------------------------------------------

///--------------------------------------------------------------------------------------------------------------------------------------------------
/// CLASS DECLARATIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class Window
{
public:
	//-----Public Methods-----

	static void	Initialize(float aspect, const char* windowTitle);
	static void ShutDown();
	static Window* GetInstance() { return s_instance; }

	void	SetWindowPixelBounds(const AABB2& newBounds);
	void	RegisterMessageHandler(WindowsMessageHandler handler);
	void	UnregisterMessageHandler(WindowsMessageHandler handler);

	AABB2	GetPixelBounds() const			{ return m_windowPixelBounds; }
	Vector2 GetPixelDimensions() const		{ return m_windowPixelBounds.GetDimensions(); }
	int		GetWindowPixelWidth() const		{ return static_cast<int>(m_windowPixelBounds.GetWidth()); }
	int		GetWindowPixelHeight() const	{ return static_cast<int>(m_windowPixelBounds.GetHeight()); }
	int		GetClientPixelWidth() const		{ return static_cast<int>(m_clientPixelBounds.GetWidth()); }
	int		GetClientPixelHeight() const	{ return static_cast<int>(m_clientPixelBounds.GetHeight()); }
	float	GetClientAspect() const			{ return static_cast<float>(m_clientPixelBounds.GetWidth()) / static_cast<float>(m_clientPixelBounds.GetHeight()); }
	void*	GetWindowContext() const		{ return m_hwnd; }
	const std::vector<WindowsMessageHandler>& GetHandlers() const { return m_messageHandlers; }


private:
	//-----Private Methods-----

	Window(float aspect, const char* windowTitle);
	~Window();
	Window(const Window& copy) = delete;


private:
	//-----Private Data-----

	void*		m_hwnd = nullptr;
	std::string m_windowTitle;
	AABB2		m_windowPixelBounds;
	AABB2		m_clientPixelBounds;

	// Handlers for windows messages
	std::vector<WindowsMessageHandler> m_messageHandlers;

	// Singleton
	static Window* s_instance;
};


///--------------------------------------------------------------------------------------------------------------------------------------------------
/// C FUNCTIONS
///--------------------------------------------------------------------------------------------------------------------------------------------------
