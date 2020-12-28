#include "stdafx.h"
#include "Application.h"
#include "GameApp.h"

#if !ENABLE_EXAMPLE
int main()
{
	Configuration config;
	Application& app = Globals::Application(config);
	GameApp game;

	if (app.Init() && game.Init())
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