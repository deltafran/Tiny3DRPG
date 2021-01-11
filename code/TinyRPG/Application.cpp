#include "stdafx.h"
#include "Application.h"
#include "Engine.h"
#include "Log.h"
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
	Log::Close();
}
//-----------------------------------------------------------------------------
bool Application::Init(const Configuration& config) noexcept
{
	m_config = config;

	HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
	if (FAILED(hr))
		return false;

	if (!m_config.logFileName.empty())
	{
		if (!Log::Open(m_config.logFileName))
			return false;
	}

	Log::Error("hello");

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