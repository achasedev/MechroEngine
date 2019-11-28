#pragma once

class Window
{
public:

	Window() {}

	void Initialize();

private:

	void* m_windowContext;

};

extern bool g_isQuitting;