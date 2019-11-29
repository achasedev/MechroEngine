#pragma once
#include <string>

class Window
{
public:

	void Initialize();


private:

	void* m_windowContext;
	std::string m_windowTitle;

};

extern bool g_isQuitting;