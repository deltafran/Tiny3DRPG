#pragma once

class InputSystem final
{
public:
	InputSystem() noexcept;

	void KeyDown(unsigned int);
	void KeyUp(unsigned int);

	bool IsKeyDown(unsigned int);
private:
	bool m_keys[256];
};