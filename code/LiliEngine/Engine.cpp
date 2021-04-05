#include "stdafx.h"
#include "Engine.h"
#include "WindowSystem.h"
#include "RendererSystem.h"
//-----------------------------------------------------------------------------
Engine::Engine(Configuration& configuration) noexcept
	: m_windowSystem(configuration.window, m_inputSystem)
	, m_renderer(configuration.renderer)
{
}
//-----------------------------------------------------------------------------
bool Engine::Init() noexcept
{
	if (!m_windowSystem.Init())
		return false;

	if (!m_renderer.Init(
		m_windowSystem.GetWindowInfo(), 
		m_windowSystem.GetConfiguration().windowWidth, 
		m_windowSystem.GetConfiguration().windowHeight))
		return false;

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
	m_isEnd = !m_windowSystem.Update();
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