#pragma once

class VulkanDevice;

class VKBuffer;

class VulkanResource final
{
public:
	VulkanResource(std::shared_ptr<VulkanDevice> device) noexcept
		: m_device(device)
	{
	}

	VKBuffer* CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void* data = nullptr) noexcept;

private:
	VulkanResource() = delete;
	VulkanResource(const VulkanResource&) = delete;
	VulkanResource(VulkanResource&&) = delete;
	VulkanResource& operator=(const VulkanResource&) = delete;
	VulkanResource& operator=(VulkanResource&&) = delete;

	std::shared_ptr<VulkanDevice> m_device = nullptr;
};