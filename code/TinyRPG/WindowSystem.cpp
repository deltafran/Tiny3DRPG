#include "stdafx.h"
#include "WindowSystem.h"
#include "InputSystem.h"
#include "Application.h"
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
	auto& wndconfig = Globals::Application().GetConfiguration().window;

	//=========================================================================
	// InitializeWindows

	m_hinstance = GetModuleHandle(NULL);

	m_applicationName = L"Tiny3DRPG";

	WNDCLASSEX wc;	
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = m_hinstance;
	wc.hIcon         = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm       = wc.hIcon;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = m_applicationName;
	wc.cbSize        = sizeof(WNDCLASSEX);

	RegisterClassEx(&wc);

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	int posX, posY;
	if (wndconfig.fullscreen)
	{
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		posX = posY = 0;
	}
	else
	{
		screenWidth = wndconfig.windowWidth;
		screenHeight = wndconfig.windowHeight;

		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
	}

	DWORD Style = (wndconfig.fullscreen ? WS_POPUP : WS_SYSMENU | WS_MINIMIZEBOX | WS_CAPTION);

	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName, Style, posX, posY, screenWidth, screenHeight, NULL, NULL, m_hinstance, NULL);

	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	//ShowCursor(false);

	return true;
}
//-----------------------------------------------------------------------------
void WindowSystem::Close()
{
	auto& wndconfig = Globals::Application().GetConfiguration().window;

	ShowCursor(true);

	if (wndconfig.fullscreen)
		ChangeDisplaySettings(NULL, 0);

	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;
}
//-----------------------------------------------------------------------------
void WindowSystem::Update()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (msg.message == WM_QUIT)
	{
		Globals::Application().Quit();
	}
}
//-----------------------------------------------------------------------------
LRESULT WindowSystem::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	auto& input = Globals::InputSystem();
	switch (umsg)
	{
	case WM_KEYDOWN:
	{
		input.KeyDown((unsigned int)wparam);
		return 0;
	}
	case WM_KEYUP:
	{
		input.KeyUp((unsigned int)wparam);
		return 0;
	}
	default:
		return DefWindowProc(hwnd, umsg, wparam, lparam);
	}
}
//-----------------------------------------------------------------------------