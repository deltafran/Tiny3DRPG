#include "stdafx.h"
#include "RendererSystem.h"
//-----------------------------------------------------------------------------
::RendererSystem& Globals::RendererSystem() noexcept
{
	static ::RendererSystem system;
	return system;
}
//-----------------------------------------------------------------------------