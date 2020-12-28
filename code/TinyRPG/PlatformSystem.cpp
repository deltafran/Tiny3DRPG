#include "stdafx.h"
#include "PlatformSystem.h"
//-----------------------------------------------------------------------------
::PlatformSystem& Globals::PlatformSystem() noexcept
{
	static ::PlatformSystem system;
	return system;
}
//-----------------------------------------------------------------------------