#pragma once

class GameApplication final
{
public:
	GameApplication(Configuration &configuration) noexcept;

	void StartGame() noexcept;
private:
	GameApplication() = delete;
	GameApplication(const GameApplication&) = delete;
	GameApplication(GameApplication&&) = delete;
	GameApplication operator=(const GameApplication&) = delete;
	GameApplication operator=(GameApplication&&) = delete;

	bool init() noexcept;
	void update() noexcept;
	void draw() noexcept;
	void close() noexcept;
	bool isEnd() const noexcept { return m_isEnd || m_engine.IsEnd(); }

	Configuration& m_configuration;
	Engine m_engine;
	bool m_isEnd = false;

	VulkanRHI* m_vulkanRHI;
};