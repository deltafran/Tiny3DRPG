#pragma once

#include "PixelFormat.h"

class VulkanRHI;

class VulkanContext final
{
public:
	VulkanContext(VulkanRHI& vulkanRHI) noexcept
		: m_vulkanRHI(vulkanRHI)
	{
	}

	void Init() noexcept
	{
		createDepthStencil();
		createRenderPass();
		createFrameBuffers();
	}

	void Close() noexcept
	{
		destroyFrameBuffers();
		destroyRenderPass();
		destroyDepthStencil();
	}

	VkSampleCountFlagBits GetSampleCount() const noexcept { return m_SampleCount; }
	VkRenderPass GetRenderPass() const noexcept { return m_RenderPass; }

	std::vector<VkFramebuffer>& GetFrameBuffers() noexcept { return m_FrameBuffers; }
//private:
	VulkanContext() = delete;
	VulkanContext(const VulkanContext&) = delete;
	VulkanContext(VulkanContext&&) = delete;
	VulkanContext& operator=(const VulkanContext&) = delete;
	VulkanContext& operator=(VulkanContext&&) = delete;

	void createDepthStencil() noexcept;
	void createRenderPass() noexcept;
	void createFrameBuffers() noexcept;
	void destroyDepthStencil() noexcept;
	void destroyRenderPass() noexcept;
	void destroyFrameBuffers() noexcept;

	VulkanRHI&					m_vulkanRHI;

	std::vector<VkFramebuffer>	m_FrameBuffers;

	VkImage						m_DepthStencilImage = VK_NULL_HANDLE;
	VkImageView					m_DepthStencilView = VK_NULL_HANDLE;
	VkDeviceMemory				m_DepthStencilMemory = VK_NULL_HANDLE;

	VkRenderPass				m_RenderPass = VK_NULL_HANDLE;
	VkSampleCountFlagBits		m_SampleCount = VK_SAMPLE_COUNT_1_BIT;

	PixelFormat					m_DepthFormat = PF_D24;
};