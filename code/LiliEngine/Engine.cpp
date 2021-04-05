#include "stdafx.h"
#include "Engine.h"
#include "WindowSystem.h"
#include "RendererSystem.h"
#include "Time.h"
//-----------------------------------------------------------------------------
Engine::Engine(Configuration& configuration) noexcept
	: m_windowSystem(configuration.window, m_inputSystem)
	, m_renderer(configuration.renderer)
{
}
//-----------------------------------------------------------------------------
bool Engine::Init() noexcept
{
	GenericPlatformTime::InitTiming();
	if (!m_windowSystem.Init())
		return false;

	if (!m_renderer.Init(
		m_windowSystem.GetWindowInfo(), 
		m_windowSystem.GetConfiguration().windowWidth, 
		m_windowSystem.GetConfiguration().windowHeight))
		return false;

	m_lastTime = GenericPlatformTime::Seconds();
	return true;
}
//-----------------------------------------------------------------------------
void Engine::Close() noexcept
{
	m_renderer.Close();
	m_windowSystem.Close();
}
//-----------------------------------------------------------------------------
void Engine::Update() noexcept
{
	const double nowT = GenericPlatformTime::Seconds();
	m_delta = nowT - m_lastTime;
	m_lastTime = nowT;
	m_currTime = m_currTime + m_delta;

	InputSystem::Reset();

	m_isEnd = !m_windowSystem.Update();

	if (m_windowSystem.IsResize())
	{
		// todo resize window
	}
}
//-----------------------------------------------------------------------------
void Engine::BeginFrame() noexcept
{
	m_renderer.BeginFrame();
}
//-----------------------------------------------------------------------------
void Engine::EndFrame() noexcept
{
	m_renderer.EndFrame();

}
//-----------------------------------------------------------------------------