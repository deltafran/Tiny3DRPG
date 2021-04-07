#include "stdafx.h"
#include "VKVertexBuffer.h"
#include "VulkanDevice.h"

VKVertexBuffer* VKVertexBuffer::Create(std::shared_ptr<VulkanDevice> vulkanDevice, VKCommandBuffer* cmdBuffer, std::vector<float> vertices, const std::vector<VertexAttribute>& attributes)
{
	VkDevice device = vulkanDevice->GetInstanceHandle();

	VKVertexBuffer* vertexBuffer = new VKVertexBuffer();
	vertexBuffer->device = device;
	vertexBuffer->attributes = attributes;

	DVKBuffer* vertexStaging = DVKBuffer::CreateBuffer(
		vulkanDevice,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		vertices.size() * sizeof(float),
		vertices.data()
	);

	vertexBuffer->dvkBuffer = DVKBuffer::CreateBuffer(
		vulkanDevice,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vertices.size() * sizeof(float)
	);

	cmdBuffer->Begin();

	VkBufferCopy copyRegion = {};
	copyRegion.size = vertices.size() * sizeof(float);
	vkCmdCopyBuffer(cmdBuffer->cmdBuffer, vertexStaging->buffer, vertexBuffer->dvkBuffer->buffer, 1, &copyRegion);

	cmdBuffer->End();
	cmdBuffer->Submit();

	delete vertexStaging;

	return vertexBuffer;
}

std::vector<VkVertexInputAttributeDescription> VKVertexBuffer::GetInputAttributes(const std::vector<VertexAttribute>& shaderInputs)
{
	std::vector<VkVertexInputAttributeDescription> vertexInputAttributs;
	int32_t offset = 0;
	for (int32_t i = 0; i < shaderInputs.size(); ++i)
	{
		VkVertexInputAttributeDescription inputAttribute = {};
		inputAttribute.binding = 0;
		inputAttribute.location = i;
		inputAttribute.format = VertexAttributeToVkFormat(shaderInputs[i]);
		inputAttribute.offset = offset;
		offset += VertexAttributeToSize(shaderInputs[i]);
		vertexInputAttributs.push_back(inputAttribute);
	}
	return vertexInputAttributs;
}

VkVertexInputBindingDescription VKVertexBuffer::GetInputBinding()
{
	int32_t stride = 0;
	for (int32_t i = 0; i < attributes.size(); ++i) {
		stride += VertexAttributeToSize(attributes[i]);
	}

	VkVertexInputBindingDescription vertexInputBinding = {};
	vertexInputBinding.binding = 0;
	vertexInputBinding.stride = stride;
	vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return vertexInputBinding;
}
