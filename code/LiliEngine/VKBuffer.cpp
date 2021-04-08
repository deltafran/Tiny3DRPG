#include "stdafx.h"
#include "VKBuffer.h"
#include "VulkanDevice.h"
#include "VulkanMemory.h"
//-----------------------------------------------------------------------------
VKBuffer::~VKBuffer()
{
	if (m_buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(m_device, m_buffer, nullptr);
		m_buffer = VK_NULL_HANDLE;
	}
	if (m_memory != VK_NULL_HANDLE) 
	{
		vkFreeMemory(m_device, m_memory, nullptr);
		m_memory = VK_NULL_HANDLE;
	}
}
//-----------------------------------------------------------------------------
VkResult VKBuffer::Map(VkDeviceSize size, VkDeviceSize offset) noexcept
{
	if (m_mapped)
		return VK_SUCCESS;
	return vkMapMemory(m_device, m_memory, offset, size, 0, &m_mapped);
}
//-----------------------------------------------------------------------------
void VKBuffer::UnMap() noexcept
{
	if (!m_mapped)
		return;
	vkUnmapMemory(m_device, m_memory);
	m_mapped = nullptr;
}
//-----------------------------------------------------------------------------
VkResult VKBuffer::Bind(VkDeviceSize offset) noexcept
{
	return vkBindBufferMemory(m_device, m_buffer, m_memory, offset);
}
//-----------------------------------------------------------------------------
void VKBuffer::SetupDescriptor(VkDeviceSize size, VkDeviceSize offset) noexcept
{
	m_descriptor.offset = offset;
	m_descriptor.buffer = m_buffer;
	m_descriptor.range = size;
}
//-----------------------------------------------------------------------------
void VKBuffer::CopyFrom(void* data, VkDeviceSize size) noexcept
{
	if (!m_mapped)
		return;
	memcpy(m_mapped, data, size);
}
//-----------------------------------------------------------------------------
VkResult VKBuffer::Flush(VkDeviceSize size, VkDeviceSize offset) noexcept
{
	VkMappedMemoryRange mappedRange = {};
	mappedRange.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedRange.memory = m_memory;
	mappedRange.offset = offset;
	mappedRange.size   = size;
	return vkFlushMappedMemoryRanges(m_device, 1, &mappedRange);
}
//-----------------------------------------------------------------------------
VkResult VKBuffer::Invalidate(VkDeviceSize size, VkDeviceSize offset) noexcept
{
	VkMappedMemoryRange mappedRange = {};
	mappedRange.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedRange.memory = m_memory;
	mappedRange.offset = offset;
	mappedRange.size   = size;
	return vkInvalidateMappedMemoryRanges(m_device, 1, &mappedRange);
}
//-----------------------------------------------------------------------------
bool VKBuffer::createBuffer(std::shared_ptr<VulkanDevice> vulkanDevice, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void* data) noexcept
{
	m_device = vulkanDevice->GetInstanceHandle();

	uint32_t memoryTypeIndex = 0;
	VkMemoryRequirements memReqs = {};
	VkMemoryAllocateInfo memAlloc;
	ZeroVulkanStruct(memAlloc, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);

	VkBufferCreateInfo bufferCreateInfo;
	ZeroVulkanStruct(bufferCreateInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
	bufferCreateInfo.usage = usageFlags;
	bufferCreateInfo.size  = size;
	vkCreateBuffer(m_device, &bufferCreateInfo, nullptr, &(m_buffer));

	vkGetBufferMemoryRequirements(m_device, m_buffer, &memReqs);
	vulkanDevice->GetMemoryManager().GetMemoryTypeFromProperties(memReqs.memoryTypeBits, memoryPropertyFlags, &memoryTypeIndex);
	memAlloc.allocationSize  = memReqs.size;
	memAlloc.memoryTypeIndex = memoryTypeIndex;

	vkAllocateMemory(m_device, &memAlloc, nullptr, &m_memory);

	m_size                = memAlloc.allocationSize;
	m_alignment           = memReqs.alignment;
	m_usageFlags          = usageFlags;
	m_memoryPropertyFlags = memoryPropertyFlags;

	if (data != nullptr)
	{
		Map();
		memcpy(m_mapped, data, size);
		if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
			Flush();
		UnMap();
	}

	SetupDescriptor();
	Bind();

	// TODO: проверки на ошибки
	return true;
}
//-----------------------------------------------------------------------------