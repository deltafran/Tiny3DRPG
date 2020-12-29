#pragma once

#include "WindowConfiguration.h"

class WindowSystem final
{
public:
	bool Init();
	void Close();
	void Update();

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

	HWND GetHWND();

private:
	// Window m_wnd; TODO: в будущем выделить в отдельный класс, сейчас же так

	LPCWSTR m_applicationName = L"Tiny3DRPG";
	HINSTANCE m_hinstance;
	HWND m_hwnd;
};