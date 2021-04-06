#include "stdafx.h"
#include "GameApplication.h"
#include "02_Triangle.h"
#include "03_Triangle2.h"
#include "04_UniformBuffer.h"
#include "05_LoadMesh.h"

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

	//Triangle game(configuration);
	Triangle2 game(configuration);
	//UniformBuffer game(configuration);
	//LoadMesh game(configuration);
	//GameApplication game(configuration);
	game.StartGame();
	return 0;
}
//-----------------------------------------------------------------------------