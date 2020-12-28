#include "stdafx.h"
#include "Engine.h"
#include "WindowSystem.h"
//-----------------------------------------------------------------------------
::Engine& Globals::Engine() noexcept
{
	static ::Engine engine;
	return engine;
}
//-----------------------------------------------------------------------------
Engine::Engine()
	: m_windowSystem(Globals::WindowSystem())
{
}
//-----------------------------------------------------------------------------
bool Engine::Init() noexcept
{
	if (!m_windowSystem.Init())
		return false;
	return true;
}
//-----------------------------------------------------------------------------
void Engine::Close() noexcept
{
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
}
//-----------------------------------------------------------------------------
void Engine::EndFrame() noexcept
{
}
//-----------------------------------------------------------------------------