#include "stdafx.h"
#include "Application.h"
#include "WindowSystem.h"
//-----------------------------------------------------------------------------
::Application& Globals::Application(const Configuration& config) noexcept
{
	static ::Application app(config);
	return app;
}
//-----------------------------------------------------------------------------
Application::Application(const Configuration& config) noexcept
	: m_config(config)
{
}
//-----------------------------------------------------------------------------
Application::~Application() noexcept
{
}
//-----------------------------------------------------------------------------
bool Application::Init() noexcept
{
	WindowSystem& windowSystem = Globals::WindowSystem();

	return true;
}
//-----------------------------------------------------------------------------
void Application::Update() noexcept
{
	if (isQuit) return;
}
//-----------------------------------------------------------------------------
void Application::BeginFrame() noexcept
{
	if (isQuit) return;
}
//-----------------------------------------------------------------------------
void Application::EndFrame() noexcept
{
	if (isQuit) return;
}
//-----------------------------------------------------------------------------
bool Application::IsQuit() noexcept
{
	return isQuit;
}
//-----------------------------------------------------------------------------
void Application::Quit() noexcept
{
	isQuit = true;
}
//-----------------------------------------------------------------------------