#pragma once

#include "NonCopyable.h"

class VulkanDevice;

class VKBuffer final : NonCopyable
{
	friend class VulkanResource;
public:
	VKBuffer() = default;
	~VKBuffer();

	VkResult Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) noexcept;
	void UnMap() noexcept;

	VkResult Bind(VkDeviceSize offset = 0) noexcept;

	void SetupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) noexcept;

	void CopyFrom(void* data, VkDeviceSize size) noexcept;

	VkResult Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) noexcept;

	VkResult Invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) noexcept;

private:
	bool createBuffer(std::shared_ptr<VulkanDevice> device, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void* data = nullptr) noexcept;

	VkDevice               m_device = VK_NULL_HANDLE;

	VkBuffer               m_buffer = VK_NULL_HANDLE;
	VkDeviceMemory         m_memory = VK_NULL_HANDLE;

	VkDescriptorBufferInfo m_descriptor;

	VkDeviceSize           m_size = 0;
	VkDeviceSize           m_alignment = 0;

	void*                  m_mapped = nullptr;

	VkBufferUsageFlags     m_usageFlags;
	VkMemoryPropertyFlags  m_memoryPropertyFlags;
};