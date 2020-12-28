#pragma once

class Engine final
{
public:
	Engine();

	bool Init() noexcept;
	void Close() noexcept;

	void Update() noexcept;
	void BeginFrame() noexcept;
	void EndFrame() noexcept;

private:
	Engine(const Engine&) = delete;
	Engine(Engine&&) = delete;
	Engine& operator=(const Engine&) = delete;
	Engine& operator=(Engine&&) = delete;

	WindowSystem& m_windowSystem;
};