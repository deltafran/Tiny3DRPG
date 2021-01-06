#include "stdafx.h"
#include "Application.h"
#include "GameApp.h"

#if !ENABLE_EXAMPLE
int main()
{
	Configuration config;
	config.logFileName = "TinyRPG.log";

	Application& app = Globals::Application();
	GameApp game;

	if (app.Init(config) && game.Init())
	{
		while (!app.IsQuit())
		{
			app.Update();
			game.Update();
			app.BeginFrame();
			game.Draw();
			app.EndFrame();
		}
		game.Close();
	}

	return 0;
}
#endif // ENABLE_EXAMPLE