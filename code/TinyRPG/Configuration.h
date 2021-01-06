#pragma once

#include "WindowConfiguration.h"
#include "RendererConfiguration.h"

struct Configuration
{
	WindowConfiguration window;
	RendererConfiguration renderer;

	std::string logFileName;
};