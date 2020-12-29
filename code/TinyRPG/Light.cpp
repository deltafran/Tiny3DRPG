#include "stdafx.h"
#include "Light.h"
//-----------------------------------------------------------------------------
void Light::SetDiffuseColor(float red, float green, float blue, float alpha)
{
	m_diffuseColor = XMFLOAT4(red, green, blue, alpha);
}
//-----------------------------------------------------------------------------
void Light::SetDirection(float x, float y, float z)
{
	m_direction = XMFLOAT3(x, y, z);
}
//-----------------------------------------------------------------------------
XMFLOAT4 Light::GetDiffuseColor()
{
	return m_diffuseColor;
}
//-----------------------------------------------------------------------------
XMFLOAT3 Light::GetDirection()
{
	return m_direction;
}
//-----------------------------------------------------------------------------