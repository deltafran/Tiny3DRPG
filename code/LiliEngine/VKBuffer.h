#pragma once

#include "NonCopyable.h"

class VKBuffer final : NonCopyable
{
public:
	VKBuffer() = default;
	~VKBuffer();

private:
	VkDevice m_device = VK_NULL_HANDLE;

	VkBuffer m_buffer = VK_NULL_HANDLE;
	VkDeviceMemory m_memory = VK_NULL_HANDLE;

	VkDescriptorBufferInfo m_descriptor;

	VkDeviceSize m_size = 0;
	VkDeviceSize m_alignment = 0;

	void* m_mapped = nullptr;

	VkBufferUsageFlags m_usageFlags;
	VkMemoryPropertyFlags m_memoryPropertyFlags;
};