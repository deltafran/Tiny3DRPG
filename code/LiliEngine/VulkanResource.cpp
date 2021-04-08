#include "stdafx.h"
#include "VulkanResource.h"
#include "VKBuffer.h"
#include "Log.h"

void VulkanResource::Close() noexcept
{
	// TODO: хранить все ресурсы еще и тут, и в случае чего удалять
}

VKBuffer* VulkanResource::CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void* data) noexcept
{
	VKBuffer* buffer = new VKBuffer();
	const bool success = buffer->createBuffer(m_device, usageFlags, memoryPropertyFlags, size, data);
	assert(success);
	if (!success)
	{
		Log::Error("VKBuffer non create!!!!");
		delete buffer;
		return nullptr;
	}

	return buffer;
}