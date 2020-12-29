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
	//=========================================================================

	//------------------------------------------------------------------------------
	// Register class
	m_hinstance = GetModuleHandle(nullptr);

	WNDCLASSEX wc    = {};
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = m_hinstance;
	wc.hIcon         = LoadIcon(nullptr, IDI_WINLOGO);
	wc.hIconSm       = wc.hIcon;
	wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
	wc.lpszMenuName  = nullptr;
	wc.lpszClassName = m_applicationName;
	if (!RegisterClassEx(&wc))
		return false;


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

		wndconfig.windowWidth = screenWidth;
		wndconfig.windowHeight = screenHeight;
	}
	else
	{
		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
	}

	screenWidth = wndconfig.windowWidth;
	screenHeight = wndconfig.windowHeight;

	DWORD Style = (wndconfig.fullscreen ? WS_POPUP : WS_SYSMENU | WS_MINIMIZEBOX | WS_CAPTION);

	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName, Style, posX, posY, screenWidth, screenHeight, NULL, NULL, m_hinstance, NULL);

	if (!m_hwnd)
		return false;

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

	CoUninitialize();
}
//-----------------------------------------------------------------------------
void WindowSystem::Update()
{
	MSG msg = {};
	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
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
HWND WindowSystem::GetHWND()
{
	return m_hwnd;
}
//-----------------------------------------------------------------------------