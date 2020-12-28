#include "stdafx.h"
#include "InputSystem.h"
//-----------------------------------------------------------------------------
::InputSystem& Globals::InputSystem() noexcept
{
	static ::InputSystem system;
	return system;
}
//-----------------------------------------------------------------------------