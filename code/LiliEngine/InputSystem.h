#pragma once

class InputSystem final
{
public:
	InputSystem() noexcept = default;

	void KeyDown(unsigned int) noexcept;
	void KeyUp(unsigned int) noexcept;

	bool IsKeyDown(unsigned int) noexcept;
private:
	InputSystem(const InputSystem&) = delete;
	InputSystem(InputSystem&&) = delete;
	InputSystem& operator=(const InputSystem&) = delete;
	InputSystem& operator=(InputSystem&&) = delete;

	bool m_keys[256] = { false };
};