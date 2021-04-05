#include "stdafx.h"
#include "WindowSystem.h"
#include "InputSystem.h"
//-----------------------------------------------------------------------------
static WindowSystem* currentWindowSystem = nullptr;
//-----------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam) noexcept
{
	if (!currentWindowSystem)
		return DefWindowProc(hwnd, umessage, wparam, lparam);

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
		return currentWindowSystem->MessageHandler(hwnd, umessage, wparam, lparam);
	}
}
//-----------------------------------------------------------------------------
WindowSystem::WindowSystem(WindowConfiguration& configuration, InputSystem& inputSystem) noexcept
	: m_configuration(configuration)
	, m_inputSystem(inputSystem)
{
}
//-----------------------------------------------------------------------------
bool WindowSystem::Init() noexcept
{
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
	if (m_configuration.fullscreen)
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

		m_configuration.windowWidth = screenWidth;
		m_configuration.windowHeight = screenHeight;
	}
	else
	{
		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
	}

	screenWidth = m_configuration.windowWidth;
	screenHeight = m_configuration.windowHeight;

	DWORD Style = (m_configuration.fullscreen ? WS_POPUP : WS_SYSMENU | WS_MINIMIZEBOX | WS_CAPTION);

	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName, Style, posX, posY, screenWidth, screenHeight, nullptr, nullptr, m_hinstance, nullptr);
	if (!m_hwnd)
		return false;

	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	//ShowCursor(false);

	currentWindowSystem = this;

	return true;
}
//-----------------------------------------------------------------------------
void WindowSystem::Close() noexcept
{
	ShowCursor(true);

	if (m_configuration.fullscreen)
		ChangeDisplaySettings(nullptr, 0);

	DestroyWindow(m_hwnd);
	m_hwnd = nullptr;

	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = nullptr;
}
//-----------------------------------------------------------------------------
bool WindowSystem::Update() noexcept
{
	MSG msg = {};
	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (msg.message == WM_QUIT)
	{
		return false;
	}
	return true;
}
//-----------------------------------------------------------------------------
LRESULT WindowSystem::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) noexcept
{
	switch (umsg)
	{
	case WM_KEYDOWN:
	{
		m_inputSystem.KeyDown((unsigned int)wparam);
		return 0;
	}
	case WM_KEYUP:
	{
		m_inputSystem.KeyUp((unsigned int)wparam);
		return 0;
	}
	default:
		return DefWindowProc(hwnd, umsg, wparam, lparam);
	}
}
//-----------------------------------------------------------------------------
WindowInfo WindowSystem::GetWindowInfo() noexcept
{
	WindowInfo info;
	info.hwnd = m_hwnd;
	return info;
}
//-----------------------------------------------------------------------------