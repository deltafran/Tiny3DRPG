#pragma once

#include "WindowConfiguration.h"

class WindowSystem final
{
public:
	bool Init();
	void Close();

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
	// Window m_wnd; TODO: в будущем выделить в отдельный класс, сейчас же так
};