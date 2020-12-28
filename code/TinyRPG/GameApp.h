#pragma once

#include "Camera.h"
#include "GraphicShaders.h"
#include "Model.h"

class GameApp
{
public:
	bool Init();

	void Update();

	void Draw();

	void Close();

private:
	Camera m_camera;
	Model m_model;
	GraphicShaders m_shaders;
};