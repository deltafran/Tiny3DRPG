#include "stdafx.h"
#include "Application.h"

#if !ENABLE_EXAMPLE
int main()
{
	Configuration config;
	Application& app = Globals::Application(config);

	if (app.Init())
	{
		// TODO: game init
		while (!app.IsQuit())
		{
			app.Update();
			// TODO: game update
			app.BeginFrame();
			// TODO: game frame
			app.EndFrame();
		}
	}
}
#endif // ENABLE_EXAMPLE