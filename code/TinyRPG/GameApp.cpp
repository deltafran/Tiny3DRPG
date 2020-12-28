#include "stdafx.h"
#include "GameApp.h"
#include "InputSystem.h"
#include "Application.h"
//-----------------------------------------------------------------------------
bool GameApp::Init()
{
	return true;
}
//-----------------------------------------------------------------------------
void GameApp::Update()
{
	auto &input = Globals::InputSystem();
	auto &app = Globals::Application();

	if (input.IsKeyDown(VK_ESCAPE))
		app.Quit();
}
//-----------------------------------------------------------------------------
void GameApp::Draw()
{
}
//-----------------------------------------------------------------------------
void GameApp::Close()
{
}
//-----------------------------------------------------------------------------