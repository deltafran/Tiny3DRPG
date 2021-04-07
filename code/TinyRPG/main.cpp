#include "stdafx.h"
#include "GameApplication.h"
#include "02_Triangle.h"
#include "03_Triangle2.h"
#include "04_UniformBuffer.h"
#include "05_LoadMesh.h"
#include "06_Pipelines.h"
#include "07_Texture.h"
#include "08_PushConstants.h"
#include "09_DynamicUniformBuffer.h"
#include "10_TextureArray.h"
#include "11_Texture3D.h"
#include "12_OptimizeShaderAndLayout.h"
#include "16_Material.h"
#include "22_SkeletonMatrix4x4.h"
#include "23_SkeletonPackIndexWeight.h"
#include "24_SkeletonQuat.h"
#include "25_SkinInTexture.h"
#include "26_SkinInstance.h"
#include "28_FXAA.h"
#include "40_QueryStatistics.h"
//-----------------------------------------------------------------------------
#pragma comment(lib, "LiliEngine.lib")
#pragma comment(lib, "3rdparty.lib")
#if LILI_VULKAN
#	pragma comment(lib, "vulkan-1.lib")
#endif
роглайк
посмотреть в yo - там много интересного - например аабб и пересечения, или структура рендера или dynamic_array(аналог векторв).в TimeManager есть сглаженная дельтатайм
//-----------------------------------------------------------------------------
int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
	Configuration configuration;
	configuration.logFileName = "TinyRPG.log";
	configuration.window.windowWidth = 1024;
	configuration.window.windowHeight = 768;
	//Triangle game(configuration);
	//Triangle2 game(configuration);
	//UniformBuffer game(configuration);
	//LoadMesh game(configuration);
	//Pipelines game(configuration);
	//Texture game(configuration);
	//PushConstants game(configuration);
	//DynamicUniformBuffer game(configuration);
	//TextureArray game(configuration);
	//Texture3D game(configuration);
	//OptimizeShaderAndLayout game(configuration);
	//Material game(configuration);
	//SkeletonMatrix4x4 game(configuration);
	//SkeletonPackIndexWeight game(configuration);
	//SkeletonQuat game(configuration);
	//SkinInTexture game(configuration);
	//SkinInstance game(configuration);
	//FXAA game(configuration);
	QueryStatistics game(configuration);
	//GameApplication game(configuration);
	game.StartGame();
	return 0;
}
//-----------------------------------------------------------------------------