#pragma once

#include "WindowConfiguration.h"
#include "WindowInfo.h"

class InputSystem;

class WindowSystem final
{
public:
	WindowSystem(WindowConfiguration& configuration, InputSystem& inputSystem) noexcept;

	bool Init() noexcept;
	void Close() noexcept;
	bool Update() noexcept;

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM) noexcept;

	WindowInfo GetWindowInfo() noexcept;

	const WindowConfiguration& GetConfiguration() const noexcept { return m_configuration; }

	bool IsResize() const noexcept { return isResize; }

private:
	WindowSystem() = delete;
	WindowSystem(const WindowSystem&) = delete;
	WindowSystem(WindowSystem&&) = delete;
	WindowSystem& operator=(const WindowSystem&) = delete;
	WindowSystem& operator=(WindowSystem&&) = delete;

	void resize(int width, int height);

	WindowConfiguration& m_configuration;
	InputSystem& m_inputSystem;
	LPCWSTR m_applicationClassName = L"Tiny3DRPG";
	HINSTANCE m_hinstance = nullptr;
	HWND m_hwnd = nullptr;
	bool isResize = false;
};