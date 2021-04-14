#include "stdafx.h"
#include "VulkanResource.h"
#include "VKBuffer.h"
#include "Log.h"

void VulkanResource::Close() noexcept
{
	m_buffers.clear();
}

std::shared_ptr<VKBuffer> VulkanResource::CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void* data) noexcept
{
	std::shared_ptr<VKBuffer> buffer(new VKBuffer());
	const bool success = buffer->createBuffer(m_device, usageFlags, memoryPropertyFlags, size, data);
	assert(success);
	if (!success)
	{
		Log::Error("VKBuffer non create!!!!");
		return nullptr;
	}

	m_buffers.emplace_back(buffer);
	return buffer;
}