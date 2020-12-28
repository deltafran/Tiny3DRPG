#pragma once

class Engine final
{
public:
	bool Init() noexcept;
	void Close() noexcept;

	void Update() noexcept;
	void BeginFrame() noexcept;
	void EndFrame() noexcept;
};