#include "stdafx.h"
#include "InputSystem.h"
//-----------------------------------------------------------------------------
void InputSystem::KeyDown(unsigned int input) noexcept
{
	m_keys[input] = true;
}
//-----------------------------------------------------------------------------
void InputSystem::KeyUp(unsigned int input) noexcept
{
	m_keys[input] = false;
}
//-----------------------------------------------------------------------------
bool InputSystem::IsKeyDown(unsigned int key) noexcept
{
	return m_keys[key];
}
//-----------------------------------------------------------------------------