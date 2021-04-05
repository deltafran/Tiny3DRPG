#pragma once

class VulkanRHI;
class VulkanSwapChain;
class VulkanDevice;

// базовый контекст вулкана - создает основные ресурсы
class DefaultVulkanContext
{
public:
	DefaultVulkanContext(VulkanRHI& vulkanRHI) noexcept
		: m_vulkanRHI(vulkanRHI)
	{
	}

	void Init() noexcept
	{
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
	}

	void Present(int backBufferIndex);

	int32_t AcquireBackbufferIndex();

	uint32_t GetMemoryTypeFromProperties(uint32_t typeBits, VkMemoryPropertyFlags properties);

	void UpdateFPS(float time, float delta)
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

	VkDevice& GetVkDevice() noexcept { return m_Device; }
	std::vector<VkCommandBuffer>& GetCommandBuffers() noexcept { return m_CommandBuffers; }
	int32_t GetFrameWidth() noexcept { return m_FrameWidth; }
	int32_t GetFrameHeight() noexcept { return m_FrameHeight; }


//private:
	DefaultVulkanContext() = delete;
	DefaultVulkanContext(const DefaultVulkanContext&) = delete;
	DefaultVulkanContext(DefaultVulkanContext&&) = delete;
	DefaultVulkanContext& operator=(const DefaultVulkanContext&) = delete;
	DefaultVulkanContext& operator=(DefaultVulkanContext&&) = delete;

	void setup();

	void createFences();
	void createCommandBuffers();
	void createPipelineCache();
	void createDefaultRes();

	void destroyFences();
	void destroyCommandBuffers();
	void destroyPipelineCache();
	void destroyDefaultRes();

	VulkanRHI& m_vulkanRHI;

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