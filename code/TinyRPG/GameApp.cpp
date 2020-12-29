#include "stdafx.h"
#include "GameApp.h"
#include "InputSystem.h"
#include "WindowSystem.h"
#include "RendererSystem.h"
#include "Application.h"
//-----------------------------------------------------------------------------
bool GameApp::Init()
{
	m_camera.SetPosition(0.0f, 2.0f, -10.0f);
	m_light.SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_light.SetDirection(0.0f, 0.0f, 1.0f);

	bool result = m_model.Init("data/cube.txt", "data/paimon.tga");
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
	static float rotation = 0.0f;
	rotation += (float)3.14 * 0.01f;
	if (rotation > 360.0f)
	{
		rotation -= 360.0f;
	}

	m_camera.Update();

	XMMATRIX viewMatrix, projectionMatrix;	
	XMMATRIX worldMatrix = XMMatrixIdentity();
	m_camera.GetViewMatrix(viewMatrix);
	Globals::RendererSystem().GetProjectionMatrix(projectionMatrix);

	worldMatrix = XMMatrixRotationY(45);

	m_model.Render();

	m_shaders.Render(m_model.GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_model.GetTexture(), m_light.GetDirection(), m_light.GetDiffuseColor());
}
//-----------------------------------------------------------------------------
void GameApp::Close()
{
	m_shaders.Close();
	m_model.Close();
}
//-----------------------------------------------------------------------------