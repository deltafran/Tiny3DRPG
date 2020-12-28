#include "stdafx.h"
#include "GameApp.h"
#include "InputSystem.h"
#include "WindowSystem.h"
#include "RendererSystem.h"
#include "Application.h"
//-----------------------------------------------------------------------------
bool GameApp::Init()
{
	m_camera.SetPosition(0.0f, 0.0f, -5.0f);
	bool result = m_model.Init("data/paimon.tga");
	if (!result)
	{
		auto hwnd = Globals::WindowSystem().GetHWND();
		MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
		return false;
	}

	result = m_shaders.Init();
	if (!result)
	{
		auto hwnd = Globals::WindowSystem().GetHWND();
		MessageBox(hwnd, L"Could not initialize the color shader object.", L"Error", MB_OK);
		return false;
	}

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
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	m_camera.Update();
	worldMatrix = XMMatrixIdentity();
	m_camera.GetViewMatrix(viewMatrix);
	Globals::RendererSystem().GetProjectionMatrix(projectionMatrix);

	m_model.Render();

	m_shaders.Render(m_model.GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_model.GetTexture());
}
//-----------------------------------------------------------------------------
void GameApp::Close()
{
	m_shaders.Close();
	m_model.Close();
}
//-----------------------------------------------------------------------------