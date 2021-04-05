#pragma once

#include "Configuration.h"
#include "WindowSystem.h"
#include "InputSystem.h"
#include "RendererSystem.h"


class Engine final
{
public:
	Engine(Configuration& configuration) noexcept;

	bool Init() noexcept;
	void Close() noexcept;

	void Update() noexcept;
	void BeginFrame() noexcept;
	void EndFrame() noexcept;

	bool IsEnd() const noexcept { return m_isEnd; }

private:
	Engine() = delete;
	Engine(const Engine&) = delete;
	Engine(Engine&&) = delete;
	Engine& operator=(const Engine&) = delete;
	Engine& operator=(Engine&&) = delete;

	InputSystem m_inputSystem;
	WindowSystem m_windowSystem;
	RendererSystem m_renderer;

	bool m_isEnd = false;
};