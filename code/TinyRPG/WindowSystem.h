#pragma once

#include "WindowConfiguration.h"

class WindowSystem final
{
public:
	bool Init();
	void Close();
	void Update();

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
	// Window m_wnd; TODO: в будущем выделить в отдельный класс, сейчас же так

	LPCWSTR m_applicationName;
	HINSTANCE m_hinstance;
	HWND m_hwnd;
};