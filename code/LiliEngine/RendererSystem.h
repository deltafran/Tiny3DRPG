#pragma once

#include "WindowInfo.h"
#include "RendererConfiguration.h"
#include "VulkanRHI.h"
#include "VulkanContext.h"

class RendererSystem final
{
public:
	RendererSystem(RendererConfiguration& configuration) noexcept;

	bool Init(const WindowInfo &info, int32_t widthSwapChain, int32_t heightSwapChain) noexcept;
	void Close() noexcept;

	void BeginFrame() noexcept;
	void EndFrame() noexcept;

	VulkanRHI& GetVulkanRHI() noexcept { return m_vulkanRHI; }
	VulkanContext& GetVulkanContext() noexcept { return m_vulkanContext; }

private:
	RendererSystem() = delete;
	RendererSystem(const RendererSystem&) = delete;
	RendererSystem(RendererSystem&&) = delete;
	RendererSystem& operator=(const RendererSystem&) = delete;
	RendererSystem& operator=(RendererSystem&&) = delete;

	RendererConfiguration& m_configuration;
	VulkanRHI m_vulkanRHI;
	VulkanContext m_vulkanContext;
};