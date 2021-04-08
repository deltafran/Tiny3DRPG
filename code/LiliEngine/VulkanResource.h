#pragma once

#include "NonCopyable.h"

class VulkanDevice;

class VKBuffer;

class VulkanResource final : NonCopyable
{
public:
	VulkanResource(std::shared_ptr<VulkanDevice> device) noexcept
		: m_device(device)
	{
	}

	void Close() noexcept;

	VKBuffer* CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void* data = nullptr) noexcept;

private:
	VulkanResource() = delete;

	std::shared_ptr<VulkanDevice> m_device = nullptr;
};