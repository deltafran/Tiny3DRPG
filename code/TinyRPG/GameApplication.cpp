#include "stdafx.h"
#include "GameApplication.h"
//-----------------------------------------------------------------------------
GameApplication::GameApplication(Configuration &configuration) noexcept
	: m_configuration(configuration)
	, m_engine(configuration)
{
}
//-----------------------------------------------------------------------------
void GameApplication::StartGame() noexcept
{
	if (init())
	{
		while (!isEnd())
		{
			m_engine.Update();
			update();
			m_engine.BeginFrame();
			draw();
			m_engine.EndFrame();
		}
		close();
		m_engine.Close();
		Log::Close();
	}
}
//-----------------------------------------------------------------------------
bool GameApplication::init() noexcept
{
	if (!m_configuration.logFileName.empty())
	{
		if (!Log::Open(m_configuration.logFileName))
			return false;
	}

	Log::Message("Start Lili Engine");

	if (!m_engine.Init())
		return false;

	return true;
}
//-----------------------------------------------------------------------------
void GameApplication::update() noexcept
{	
}
//-----------------------------------------------------------------------------
void GameApplication::draw() noexcept
{
}
//-----------------------------------------------------------------------------
void GameApplication::close() noexcept
{
	
}
//-----------------------------------------------------------------------------