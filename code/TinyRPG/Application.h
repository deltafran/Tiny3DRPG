#pragma once

#include "Configuration.h"

class Application final
{
public:
	Application() noexcept;
	~Application() noexcept;

	bool Init(const Configuration& config) noexcept;

	void Update() noexcept;
	void BeginFrame() noexcept;
	void EndFrame() noexcept;

	bool IsQuit() noexcept;
	void Quit() noexcept;

	Configuration& GetConfiguration() noexcept;

private:
	Application(const Application&) = delete;
	Application(Application&&) = delete;
	Application& operator=(const Application&) = delete;
	Application& operator=(Application&&) = delete;

	Configuration m_config;
	Engine &m_engine;
	bool isQuit = false;
};