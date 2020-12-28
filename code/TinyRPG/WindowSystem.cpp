#include "stdafx.h"
#include "WindowSystem.h"
//-----------------------------------------------------------------------------
::WindowSystem& Globals::WindowSystem() noexcept
{
	static ::WindowSystem wnd;
	return wnd;
}
//-----------------------------------------------------------------------------