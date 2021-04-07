#include "stdafx.h"
#include "VKIndexBuffer.h"
#include "VulkanDevice.h"

VKIndexBuffer* VKIndexBuffer::Create(std::shared_ptr<VulkanDevice> vulkanDevice, VKCommandBuffer* cmdBuffer, std::vector<uint32_t> indices)
{
	VkDevice device = vulkanDevice->GetInstanceHandle();

	VKIndexBuffer* indexBuffer = new VKIndexBuffer();
	indexBuffer->device = device;
	indexBuffer->indexCount = indices.size();
	indexBuffer->indexType = VK_INDEX_TYPE_UINT32;

	DVKBuffer* indexStaging = DVKBuffer::CreateBuffer(
		vulkanDevice,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		indices.size() * sizeof(uint32_t),
		indices.data()
	);

	indexBuffer->dvkBuffer = DVKBuffer::CreateBuffer(
		vulkanDevice,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		indices.size() * sizeof(uint32_t)
	);

	cmdBuffer->Begin();

	VkBufferCopy copyRegion = {};
	copyRegion.size = indices.size() * sizeof(uint32_t);

	vkCmdCopyBuffer(cmdBuffer->cmdBuffer, indexStaging->buffer, indexBuffer->dvkBuffer->buffer, 1, &copyRegion);

	cmdBuffer->End();
	cmdBuffer->Submit();

	delete indexStaging;

	return indexBuffer;
}

VKIndexBuffer* VKIndexBuffer::Create(std::shared_ptr<VulkanDevice> vulkanDevice, VKCommandBuffer* cmdBuffer, std::vector<uint16_t> indices)
{
	VkDevice device = vulkanDevice->GetInstanceHandle();

	VKIndexBuffer* indexBuffer = new VKIndexBuffer();
	indexBuffer->device = device;
	indexBuffer->indexCount = indices.size();
	indexBuffer->indexType = VK_INDEX_TYPE_UINT16;

	DVKBuffer* indexStaging = DVKBuffer::CreateBuffer(
		vulkanDevice,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		indices.size() * sizeof(uint16_t),
		indices.data()
	);

	indexBuffer->dvkBuffer = DVKBuffer::CreateBuffer(
		vulkanDevice,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		indices.size() * sizeof(uint16_t)
	);

	cmdBuffer->Begin();

	VkBufferCopy copyRegion = {};
	copyRegion.size = indices.size() * sizeof(uint16_t);

	vkCmdCopyBuffer(cmdBuffer->cmdBuffer, indexStaging->buffer, indexBuffer->dvkBuffer->buffer, 1, &copyRegion);

	cmdBuffer->End();
	cmdBuffer->Submit();

	delete indexStaging;

	return indexBuffer;
}