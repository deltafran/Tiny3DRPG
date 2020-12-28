#include "stdafx.h"
#include "Camera.h"
//-----------------------------------------------------------------------------
Camera::Camera()
{
	m_positionX = 0.0f;
	m_positionY = 0.0f;
	m_positionZ = 0.0f;

	m_rotationX = 0.0f;
	m_rotationY = 0.0f;
	m_rotationZ = 0.0f;
}
//-----------------------------------------------------------------------------
void Camera::SetPosition(float x, float y, float z)
{
	m_positionX = x;
	m_positionY = y;
	m_positionZ = z;
}
//-----------------------------------------------------------------------------
void Camera::SetRotation(float x, float y, float z)
{
	m_rotationX = x;
	m_rotationY = y;
	m_rotationZ = z;
}
//-----------------------------------------------------------------------------
XMFLOAT3 Camera::GetPosition()
{
	return XMFLOAT3(m_positionX, m_positionY, m_positionZ);
}
//-----------------------------------------------------------------------------
XMFLOAT3 Camera::GetRotation()
{
	return XMFLOAT3(m_rotationX, m_rotationY, m_rotationZ);
}
//-----------------------------------------------------------------------------
void Camera::Update()
{
	XMFLOAT3 up;
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;
	XMVECTOR upVector = XMLoadFloat3(&up);

	XMFLOAT3 position;
	position.x = m_positionX;
	position.y = m_positionY;
	position.z = m_positionZ;
	XMVECTOR positionVector = XMLoadFloat3(&position);

	XMFLOAT3 lookAt;
	lookAt.x = 0.0f;
	lookAt.y = 0.0f;
	lookAt.z = 1.0f;
	XMVECTOR lookAtVector = XMLoadFloat3(&lookAt);

	// Set the yaw (Y axis), pitch (X axis), and roll (Z axis) rotations in radians.
	float pitch = m_rotationX * 0.0174532925f;
	float yaw = m_rotationY * 0.0174532925f;
	float roll = m_rotationZ * 0.0174532925f;

	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	lookAtVector = XMVector3TransformCoord(lookAtVector, rotationMatrix);
	upVector = XMVector3TransformCoord(upVector, rotationMatrix);
	lookAtVector = XMVectorAdd(positionVector, lookAtVector);

	m_viewMatrix = XMMatrixLookAtLH(positionVector, lookAtVector, upVector);
}
//-----------------------------------------------------------------------------
void Camera::GetViewMatrix(XMMATRIX& viewMatrix)
{
	viewMatrix = m_viewMatrix;
}
//-----------------------------------------------------------------------------