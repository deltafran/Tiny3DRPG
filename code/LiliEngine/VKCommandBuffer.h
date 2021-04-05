#pragma once

class VulkanQueue;
class VulkanDevice;

class VKCommandBuffer
{
public:
	~VKCommandBuffer();

	void Begin();

	void End();

	void Submit(VkSemaphore* signalSemaphore = nullptr);

	static VKCommandBuffer* Create(std::shared_ptr<VulkanDevice> vulkanDevice, VkCommandPool commandPool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY, std::shared_ptr<VulkanQueue> queue = nullptr);

	std::shared_ptr<VulkanQueue>		queue = nullptr;

	VkCommandBuffer						cmdBuffer = VK_NULL_HANDLE;
	VkFence								fence = VK_NULL_HANDLE;
	VkCommandPool						commandPool = VK_NULL_HANDLE;
	std::shared_ptr<VulkanDevice>		vulkanDevice = nullptr;
	std::vector<VkPipelineStageFlags>	waitFlags;
	std::vector<VkSemaphore>			waitSemaphores;

	bool								isBegun;

private:
	VKCommandBuffer() = default;
};