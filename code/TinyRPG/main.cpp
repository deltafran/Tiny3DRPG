#include "stdafx.h"
#include "GameApplication.h"
//-----------------------------------------------------------------------------
#pragma comment(lib, "LiliEngine.lib")
#pragma comment(lib, "3rdparty.lib")
#if LILI_VULKAN
#	pragma comment(lib, "vulkan-1.lib")
#endif
//-----------------------------------------------------------------------------
int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
	Configuration configuration;
	configuration.logFileName = "TinyRPG.log";

	GameApplication game(configuration);
	game.StartGame();
	return 0;
}
//-----------------------------------------------------------------------------