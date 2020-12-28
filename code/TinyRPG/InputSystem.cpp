#include "stdafx.h"
#include "InputSystem.h"
//-----------------------------------------------------------------------------
::InputSystem& Globals::InputSystem() noexcept
{
	static ::InputSystem system;
	return system;
}
//-----------------------------------------------------------------------------
InputSystem::InputSystem() noexcept
{
	for (int i = 0; i < 256; i++)
		m_keys[i] = false;
}
//-----------------------------------------------------------------------------
void InputSystem::KeyDown(unsigned int input)
{
	m_keys[input] = true;
}
//-----------------------------------------------------------------------------
void InputSystem::KeyUp(unsigned int input)
{
	m_keys[input] = false;
}
//-----------------------------------------------------------------------------
bool InputSystem::IsKeyDown(unsigned int key)
{
	return m_keys[key];
}
//-----------------------------------------------------------------------------