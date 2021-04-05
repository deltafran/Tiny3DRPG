#include "stdafx.h"
#include "VKCamera.h"
#include "InputSystem.h"

VKCamera::VKCamera()
{
	freeze = Vector3(0, 0, 0);
	m_LastMouse = InputSystem::GetMousePosition();
}

void VKCamera::Update(float time, float delta)
{
	float mouseSpeedX = InputSystem::GetMousePosition().x - m_LastMouse.x;
	float mouseSpeedY = InputSystem::GetMousePosition().y - m_LastMouse.y;

	if (InputSystem::IsMouseUp(MouseType::MOUSE_BUTTON_LEFT)) {
		m_Drag = false;
	}

	if (m_Drag)
	{
		if (InputSystem::IsKeyDown(KeyboardType::KEY_SPACE))
		{
			m_World.TranslateX(-mouseSpeedX * m_World.GetOrigin().Size() / 300);
			m_World.TranslateY(mouseSpeedY * m_World.GetOrigin().Size() / 300);
		}
		else
		{
			m_SpinX += mouseSpeedX * smooth * speedFactor;
			m_SpinY += mouseSpeedY * smooth * speedFactor;
		}
	}

	if (InputSystem::GetMouseDelta() != 0.0f) {
		m_SpinZ = (m_World.GetOrigin().Size() + 0.1f) * speedFactor * InputSystem::GetMouseDelta() / 20.0f;
	}

	m_SpinX *= 1.0f - freeze.y;
	m_SpinY *= 1.0f - freeze.x;
	m_SpinZ *= 1.0f - freeze.z;

	m_World.TranslateZ(m_SpinZ);
	m_World.RotateY(m_SpinX, false, &Vector3::ZeroVector);
	m_World.RotateX(m_SpinY, true, &Vector3::ZeroVector);

	m_SpinX *= (1 - smooth);
	m_SpinY *= (1 - smooth);
	m_SpinZ *= (1 - smooth);

	if (InputSystem::IsMouseDown(MouseType::MOUSE_BUTTON_LEFT)) {
		m_Drag = true;
	}

	m_LastMouse = InputSystem::GetMousePosition();
}