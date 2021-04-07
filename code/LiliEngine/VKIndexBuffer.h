#pragma once

#include "DVKBuffer.h"
#include "VKCommandBuffer.h"

class VKIndexBuffer
{
private:
	VKIndexBuffer()
	{

	}

public:
	~VKIndexBuffer()
	{
		if (dvkBuffer) {
			delete dvkBuffer;
		}
		dvkBuffer = nullptr;
	}

	void Bind(VkCommandBuffer cmdBuffer)
	{
		vkCmdBindIndexBuffer(cmdBuffer, dvkBuffer->buffer, 0, indexType);
	}

	static VKIndexBuffer* Create(std::shared_ptr<VulkanDevice> vulkanDevice, VKCommandBuffer* cmdBuffer, std::vector<uint16_t> indices);

	static VKIndexBuffer* Create(std::shared_ptr<VulkanDevice> vulkanDevice, VKCommandBuffer* cmdBuffer, std::vector<uint32_t> indices);

public:
	VkDevice		device = VK_NULL_HANDLE;
	DVKBuffer* dvkBuffer = nullptr;
	int32_t           instanceCount = 1;
	int32_t			indexCount = 0;
	VkIndexType		indexType = VK_INDEX_TYPE_UINT16;
};