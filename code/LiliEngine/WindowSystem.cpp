#include "stdafx.h"
#include "WindowSystem.h"
#include "InputSystem.h"
#include "Vector2.h"
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
	wc.lpszClassName = m_applicationClassName;
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

	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationClassName, m_applicationClassName, Style, posX, posY, screenWidth, screenHeight, nullptr, nullptr, m_hinstance, nullptr);
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

	UnregisterClass(m_applicationClassName, m_hinstance);
	m_hinstance = nullptr;
}
//-----------------------------------------------------------------------------
bool WindowSystem::Update() noexcept
{
	isResize = false;
	MSG msg = {};
	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (msg.message == WM_QUIT)
		return false;

	return true;
}
//-----------------------------------------------------------------------------
LRESULT WindowSystem::MessageHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept
{
	switch (msg)
	{
	case WM_KEYDOWN:
	{
		//m_inputSystem.KeyDown((unsigned int)wparam);
		const int32_t keycode = HIWORD(lparam) & 0x1FF;
		const KeyboardType key = InputSystem::GetKeyFromKeyCode(keycode);
		InputSystem::onKeyDown(key);
		return 0;
	}
	case WM_KEYUP:
	{
		const int32_t keycode = HIWORD(lparam) & 0x1FF;
		const KeyboardType key = InputSystem::GetKeyFromKeyCode(keycode);
		InputSystem::onKeyUp(key);
		//m_inputSystem.KeyUp((unsigned int)wparam);
		return 0;
	}
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_XBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_XBUTTONUP:
	{
		const int x = GET_X_LPARAM(lparam);
		const int y = GET_Y_LPARAM(lparam);

		Vector2 pos(x, y);

		MouseType button = MouseType::MOUSE_BUTTON_LEFT;
		if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP)
			button = MouseType::MOUSE_BUTTON_LEFT;
		else if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP)
			button = MouseType::MOUSE_BUTTON_RIGHT;
		else if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONUP)
			button = MouseType::MOUSE_BUTTON_MIDDLE;
		else if (GET_XBUTTON_WPARAM(wparam) == XBUTTON1)
			button = MouseType::MOUSE_BUTTON_4;
		else
			button = MouseType::MOUSE_BUTTON_5;

		int32_t action = 0;
		if (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN || msg == WM_XBUTTONDOWN)
		{
			action = 1;
			SetCapture(hwnd);
		}
		else
		{
			action = 0;
			ReleaseCapture();
		}

		if (action == 1)
			InputSystem::onMouseDown(button, pos);
		else
			InputSystem::onMouseUp(button, pos);

		if (msg == WM_XBUTTONDOWN || msg == WM_XBUTTONUP)
			return TRUE;

		return 0;
	}
	case WM_MOUSEMOVE:
	{
		const int x = GET_X_LPARAM(lparam);
		const int y = GET_Y_LPARAM(lparam);
		const Vector2 pos(x, y);
		InputSystem::onMouseMove(pos);
		return 0;
	}
	case WM_MOUSEWHEEL:
	{
		const int x = GET_X_LPARAM(lparam);
		const int y = GET_Y_LPARAM(lparam);
		const Vector2 pos(x, y);
		InputSystem::onMouseWheel((float)GET_WHEEL_DELTA_WPARAM(wparam) / (float)WHEEL_DELTA, pos);
		return 0;
	}
	case WM_MOUSEHWHEEL:
	{
		const int x = GET_X_LPARAM(lparam);
		const int y = GET_Y_LPARAM(lparam);
		const Vector2 pos(x, y);
		InputSystem::onMouseWheel((float)GET_WHEEL_DELTA_WPARAM(wparam) / (float)WHEEL_DELTA, pos);
		return 0;
	}
	case WM_SIZE:
		resize(LOWORD(lparam), HIWORD(lparam));
		return 0;
	default:
		return DefWindowProc(hwnd, msg, wparam, lparam);
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
void WindowSystem::resize(int width, int height)
{
	isResize = true;
	m_configuration.windowWidth = width;
	m_configuration.windowHeight = height;
}
//-----------------------------------------------------------------------------