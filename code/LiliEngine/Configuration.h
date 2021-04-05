#pragma once

#include "WindowConfiguration.h"
#include "RendererConfiguration.h"

class Configuration final
{
public:
	Configuration() = default;

	WindowConfiguration window;
	RendererConfiguration renderer;
	std::string logFileName;

private:
	Configuration(const Configuration&) = delete;
	Configuration(Configuration&&) = delete;
	Configuration operator=(const Configuration&) = delete;
	Configuration operator=(Configuration&&) = delete;
};