#pragma once

#include "PixelFormat.h"

class VulkanRHI;
class VulkanSwapChain;
class VulkanDevice;

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
		setup();
		createFences();
		createCommandBuffers();
		createPipelineCache();
		createDefaultRes();
	}

	void Close() noexcept
	{
		destroyDefaultRes();
		destroyFences();
		destroyCommandBuffers();
		destroyPipelineCache();
		m_SwapChain = nullptr;
		destroyFrameBuffers();
		destroyRenderPass();
		destroyDepthStencil();
	}

	VkSampleCountFlagBits GetSampleCount() const noexcept { return m_SampleCount; }
	VkRenderPass GetRenderPass() const noexcept { return m_RenderPass; }

	std::vector<VkFramebuffer>& GetFrameBuffers() noexcept { return m_FrameBuffers; }

	VkDevice& GetVkDevice() noexcept { return m_Device; }
	std::vector<VkCommandBuffer>& GetCommandBuffers() noexcept { return m_CommandBuffers; }
	int32_t GetFrameWidth() noexcept { return m_FrameWidth; }
	int32_t GetFrameHeight() noexcept { return m_FrameHeight; }

	void Present(int backBufferIndex) noexcept;

	int32_t AcquireBackbufferIndex() noexcept;

	uint32_t GetMemoryTypeFromProperties(uint32_t typeBits, VkMemoryPropertyFlags properties) noexcept;

	void UpdateFPS(float time, float delta) noexcept
	{
		m_FrameCounter += 1;
		m_LastFrameTime += delta;
		if (m_LastFrameTime >= 1.0f)
		{
			m_LastFPS = m_FrameCounter;
			m_FrameCounter = 0;
			m_LastFrameTime = 0.0f;
		}
	}

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

	void setup() noexcept;

	void createFences() noexcept;
	void createCommandBuffers() noexcept;
	void createPipelineCache() noexcept;
	void createDefaultRes() noexcept;

	void destroyFences() noexcept;
	void destroyCommandBuffers() noexcept;
	void destroyPipelineCache() noexcept;
	void destroyDefaultRes() noexcept;

	VulkanRHI&					m_vulkanRHI;

	std::vector<VkFramebuffer>	m_FrameBuffers;

	VkImage						m_DepthStencilImage = VK_NULL_HANDLE;
	VkImageView					m_DepthStencilView = VK_NULL_HANDLE;
	VkDeviceMemory				m_DepthStencilMemory = VK_NULL_HANDLE;

	VkRenderPass				m_RenderPass = VK_NULL_HANDLE;
	VkSampleCountFlagBits		m_SampleCount = VK_SAMPLE_COUNT_1_BIT;

	PixelFormat					m_DepthFormat = PF_D24;


	typedef std::shared_ptr<VulkanSwapChain> VulkanSwapChainRef;

	VkDevice						m_Device = VK_NULL_HANDLE;
	std::shared_ptr<VulkanDevice>	m_VulkanDevice = nullptr;
	VkQueue							m_GfxQueue = VK_NULL_HANDLE;
	VkQueue							m_PresentQueue = VK_NULL_HANDLE;

	int32_t							m_FrameWidth = 0;
	int32_t							m_FrameHeight = 0;

	VkPipelineCache                 m_PipelineCache = VK_NULL_HANDLE;

	std::vector<VkFence> 			m_Fences;
	VkSemaphore 					m_PresentComplete = VK_NULL_HANDLE;
	VkSemaphore 					m_RenderComplete = VK_NULL_HANDLE;

	VkCommandPool					m_CommandPool = VK_NULL_HANDLE;
	VkCommandPool					m_ComputeCommandPool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer>	m_CommandBuffers;

	VkPipelineStageFlags			m_WaitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VulkanSwapChainRef				m_SwapChain = VK_NULL_HANDLE;

	int32_t                         m_FrameCounter = 0;
	float                           m_LastFrameTime = 0.0f;
	float                           m_LastFPS = 0.0f;
};