#include "stdafx.h"
#include "Application.h"
#include "Engine.h"
//-----------------------------------------------------------------------------
::Application& Globals::Application() noexcept
{
	static ::Application app;
	return app;
}
//-----------------------------------------------------------------------------
Application::Application() noexcept
	: m_engine(Globals::Engine())
{
}
//-----------------------------------------------------------------------------
Application::~Application() noexcept
{
	m_engine.Close();
}
//-----------------------------------------------------------------------------
bool Application::Init(const Configuration& config) noexcept
{
	m_config = config;

	if (!m_engine.Init())
		return false;

	return true;
}
//-----------------------------------------------------------------------------
void Application::Update() noexcept
{
	if (isQuit) return;

	m_engine.Update();
}
//-----------------------------------------------------------------------------
void Application::BeginFrame() noexcept
{
	if (isQuit) return;

	m_engine.BeginFrame();
}
//-----------------------------------------------------------------------------
void Application::EndFrame() noexcept
{
	if (isQuit) return;
	
	m_engine.EndFrame();
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
Configuration& Application::GetConfiguration() noexcept
{
	return m_config;
}
//-----------------------------------------------------------------------------