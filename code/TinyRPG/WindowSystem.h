#pragma once

#include "Window.h"

class WindowSystem final
{
public:
	bool Init();
	void Close();

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
	Window m_wnd;
};