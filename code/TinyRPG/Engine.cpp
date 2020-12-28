#include "stdafx.h"
#include "Engine.h"
#include "WindowSystem.h"
#include "RendererSystem.h"
//-----------------------------------------------------------------------------
::Engine& Globals::Engine() noexcept
{
	static ::Engine engine;
	return engine;
}
//-----------------------------------------------------------------------------
Engine::Engine()
	: m_windowSystem(Globals::WindowSystem())
	, m_renderer(Globals::RendererSystem())
{
}
//-----------------------------------------------------------------------------
bool Engine::Init() noexcept
{
	if (!m_windowSystem.Init())
		return false;

	if (!m_renderer.Init())
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
	m_windowSystem.Update();
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