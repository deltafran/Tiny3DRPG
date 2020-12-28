#pragma once

#include "Configuration.h"

class Application final
{
public:
	Application(const Configuration& config) noexcept;
	~Application() noexcept;

	bool Init() noexcept;

	void Update() noexcept;
	void BeginFrame() noexcept;
	void EndFrame() noexcept;

	bool IsQuit() noexcept;

	void Quit() noexcept;

private:
	Application(const Application&) = delete;
	Application(Application&&) = delete;
	Application& operator=(const Application&) = delete;
	Application& operator=(Application&&) = delete;

	Configuration m_config;

	bool isQuit = false;
};