#include "stdafx.h"
#include "RendererSystem.h"
//-----------------------------------------------------------------------------
// Indicates to hybrid graphics systems to prefer the discrete part by default
//extern "C"
//{
//	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
//	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
//}
//-----------------------------------------------------------------------------
RendererSystem::RendererSystem(RendererConfiguration& configuration) noexcept
	: m_configuration(configuration)
	, m_vulkanContext(m_vulkanRHI)
	, m_vulkanResource(m_vulkanRHI.GetDevice())
{
}
//-----------------------------------------------------------------------------
bool RendererSystem::Init(const WindowInfo& info, int32_t widthSwapChain, int32_t heightSwapChain) noexcept
{
	if (!m_vulkanRHI.Init(info, widthSwapChain, heightSwapChain))
		return false;

	m_vulkanContext.Init();

	return true;
}
//-----------------------------------------------------------------------------
void RendererSystem::Close() noexcept
{
	m_vulkanContext.Close();
	m_vulkanResource.Close();
	m_vulkanRHI.Close();
}
//-----------------------------------------------------------------------------
void RendererSystem::BeginFrame() noexcept
{
}
//-----------------------------------------------------------------------------
void RendererSystem::EndFrame() noexcept
{

}
//-----------------------------------------------------------------------------