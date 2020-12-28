#include "stdafx.h"
#include "WindowSystem.h"
//-----------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch (umessage)
	{
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		return 0;
	}
	default:
		return Globals::WindowSystem().MessageHandler(hwnd, umessage, wparam, lparam);
	}
}
//-----------------------------------------------------------------------------
::WindowSystem& Globals::WindowSystem() noexcept
{
	static ::WindowSystem system;
	return system;
}
//-----------------------------------------------------------------------------
bool WindowSystem::Init()
{
	return false;
}
//-----------------------------------------------------------------------------
void WindowSystem::Close()
{
}
//-----------------------------------------------------------------------------