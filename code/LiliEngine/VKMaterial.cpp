#include "stdafx.h"
#include "VKMaterial.h"
#include "VulkanDevice.h"

VKRingBuffer* VKMaterial::ringBuffer = nullptr;
int32_t			VKMaterial::ringBufferRefCount = 0;

void VKMaterial::InitRingBuffer(std::shared_ptr<VulkanDevice> vulkanDevice)
{
	ringBuffer = new VKRingBuffer();
	ringBuffer->device = vulkanDevice->GetInstanceHandle();
	ringBuffer->bufferSize = 32 * 1024 * 1024; // 32MB
	ringBuffer->bufferOffset = ringBuffer->bufferSize;
	ringBuffer->minAlignment = vulkanDevice->GetLimits().minUniformBufferOffsetAlignment;
	ringBuffer->realBuffer = DVKBuffer::CreateBuffer(
		vulkanDevice,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		ringBuffer->bufferSize
	);
	ringBuffer->realBuffer->Map();

	ringBufferRefCount = 0;
}

void VKMaterial::DestroyRingBuffer()
{
	delete ringBuffer;
	ringBuffer = nullptr;
	ringBufferRefCount = 0;
}

VKMaterial::~VKMaterial()
{
	shader = nullptr;

	delete descriptorSet;
	descriptorSet = nullptr;

	textures.clear();
	uniformBuffers.clear();

	vulkanDevice = nullptr;

	if (pipeline)
	{
		delete pipeline;
		pipeline = nullptr;
	}

	ringBufferRefCount -= 1;
	if (ringBufferRefCount == 0) {
		DestroyRingBuffer();
	}
}

VKMaterial* VKMaterial::Create(std::shared_ptr<VulkanDevice> vulkanDevice, VKRenderTarget* renderTarget, VkPipelineCache pipelineCache, VKShader* shader)
{
	if (ringBufferRefCount == 0) {
		InitRingBuffer(vulkanDevice);
	}
	ringBufferRefCount += 1;

	VKMaterial* material = new VKMaterial();
	material->pipelineInfo.colorAttachmentCount = renderTarget->renderPassInfo.numColorRenderTargets;

	material->vulkanDevice = vulkanDevice;
	material->shader = shader;
	material->renderPass = renderTarget->GetRenderPass();
	material->pipelineCache = pipelineCache;
	material->Prepare();

	return material;
}

VKMaterial* VKMaterial::Create(std::shared_ptr<VulkanDevice> vulkanDevice, VkRenderPass renderPass, VkPipelineCache pipelineCache, VKShader* shader)
{
	if (ringBufferRefCount == 0) {
		InitRingBuffer(vulkanDevice);
	}
	ringBufferRefCount += 1;

	VKMaterial* material = new VKMaterial();
	material->vulkanDevice = vulkanDevice;
	material->shader = shader;
	material->renderPass = renderPass;
	material->pipelineCache = pipelineCache;
	material->Prepare();

	return material;
}

void VKMaterial::Prepare()
{
	descriptorSet = shader->AllocateDescriptorSet();

	for (auto it = shader->bufferParams.begin(); it != shader->bufferParams.end(); ++it)
	{
		VKSimulateBuffer uboBuffer = {};
		uboBuffer.binding = it->second.binding;
		uboBuffer.descriptorType = it->second.descriptorType;
		uboBuffer.set = it->second.set;
		uboBuffer.stageFlags = it->second.stageFlags;
		uboBuffer.dataSize = it->second.bufferSize;
		uboBuffer.bufferInfo = {};
		uboBuffer.bufferInfo.buffer = ringBuffer->realBuffer->buffer;
		uboBuffer.bufferInfo.offset = 0;
		uboBuffer.bufferInfo.range = uboBuffer.dataSize;

		if (it->second.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
			it->second.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
		{
			uniformBuffers.insert(std::make_pair(it->first, uboBuffer));
			descriptorSet->WriteBuffer(it->first, &(uboBuffer.bufferInfo));
		}
		else if (it->second.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
			it->second.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
		{
			storageBuffers.insert(std::make_pair(it->first, uboBuffer));
		}
	}

	dynamicOffsetCount = 0;
	std::vector<VKDescriptorSetLayoutInfo>& setLayouts = shader->setLayoutsInfo.setLayouts;
	for (int32_t i = 0; i < setLayouts.size(); ++i)
	{
		std::vector<VkDescriptorSetLayoutBinding>& bindings = setLayouts[i].bindings;
		for (int32_t j = 0; j < bindings.size(); ++j)
		{
			for (auto it = uniformBuffers.begin(); it != uniformBuffers.end(); ++it)
			{
				if (it->second.set == setLayouts[i].set &&
					it->second.binding == bindings[j].binding &&
					it->second.descriptorType == bindings[j].descriptorType &&
					it->second.stageFlags == bindings[j].stageFlags
					)
				{
					it->second.dynamicIndex = dynamicOffsetCount;
					dynamicOffsetCount += 1;
					break;
				}
			}
		}
	}
	globalOffsets.resize(dynamicOffsetCount);

	for (auto it = shader->imageParams.begin(); it != shader->imageParams.end(); ++it)
	{
		VKSimulateTexture texture = {};
		texture.texture = nullptr;
		texture.binding = it->second.binding;
		texture.descriptorType = it->second.descriptorType;
		texture.set = it->second.set;
		texture.stageFlags = it->second.stageFlags;
		textures.insert(std::make_pair(it->first, texture));
	}
}

void VKMaterial::PreparePipeline()
{
	if (pipeline)
	{
		delete pipeline;
		pipeline = nullptr;
	}

	// pipeline
	pipelineInfo.shader = shader;
	pipeline = VKGfxPipeline::Create(
		vulkanDevice,
		pipelineCache,
		pipelineInfo,
		shader->inputBindings,
		shader->inputAttributes,
		shader->pipelineLayout,
		renderPass
	);
}

void VKMaterial::BeginFrame()
{
	if (actived) {
		return;
	}
	actived = true;
	perObjectIndexes.clear();

	memset(globalOffsets.data(), MAX_uint32, sizeof(uint32_t) * globalOffsets.size());

	for (auto it = uniformBuffers.begin(); it != uniformBuffers.end(); ++it)
	{
		if (!it->second.global) {
			continue;
		}
		uint8_t* ringCPUData = (uint8_t*)(ringBuffer->GetMappedPointer());
		uint64_t ringOffset = ringBuffer->AllocateMemory(it->second.dataSize);
		uint64_t bufferSize = it->second.dataSize;

		memcpy(ringCPUData + ringOffset, it->second.dataContent.data(), bufferSize);

		globalOffsets[it->second.dynamicIndex] = ringOffset;
	}
}

void VKMaterial::EndFrame()
{
	actived = false;
}

void VKMaterial::BeginObject()
{
	int32_t index = perObjectIndexes.size();
	perObjectIndexes.push_back(index);

	int32_t offsetStart = index * dynamicOffsetCount;

	if (offsetStart + dynamicOffsetCount > dynamicOffsets.size()) {
		for (int32_t i = 0; i < dynamicOffsetCount; ++i) {
			dynamicOffsets.push_back(0);
		}
	}

	for (int32_t offsetIndex = offsetStart; offsetIndex < dynamicOffsetCount; ++offsetIndex) {
		dynamicOffsets[offsetIndex] = globalOffsets[offsetIndex - offsetStart];
	}
}

void VKMaterial::EndObject()
{
	for (int32_t i = 0; i < perObjectIndexes.size(); ++i)
	{
		int32_t offsetStart = i * dynamicOffsetCount;
		for (int32_t offsetIndex = offsetStart; offsetIndex < dynamicOffsetCount; ++offsetIndex) {
			if (dynamicOffsets[offsetIndex] == MAX_uint32) {
				MLOGE("Uniform not set\n");
			}
		}
	}

	if (perObjectIndexes.size() == 0)
	{
		for (int32_t i = 0; i < dynamicOffsetCount; ++i) {
			if (globalOffsets[i] == MAX_uint32) {
				MLOGE("Uniform not set\n");
			}
		}
	}
}

void VKMaterial::BindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint, int32_t objIndex)
{
	uint32_t* dynOffsets = nullptr;
	if (objIndex < perObjectIndexes.size())
	{
		dynOffsets = dynamicOffsets.data() + perObjectIndexes[objIndex] * dynamicOffsetCount;;;
	}
	else if (globalOffsets.size() > 0)
	{
		dynOffsets = globalOffsets.data();
	}

	vkCmdBindDescriptorSets(
		commandBuffer,
		bindPoint,
		GetPipelineLayout(),
		0, GetDescriptorSets().size(), GetDescriptorSets().data(),
		dynamicOffsetCount, dynOffsets
	);
}

void VKMaterial::SetLocalUniform(const std::string& name, void* dataPtr, uint32_t size)
{
	auto it = uniformBuffers.find(name);
	if (it == uniformBuffers.end())
	{
		MLOGE("Uniform %s not found.", name.c_str());
		return;
	}

	if (it->second.dataSize != size)
	{
		MLOGE("Uniform %s size not match, dst=%ud src=%ud", name.c_str(), it->second.dataSize, size);
		return;
	}

	int32_t objIndex = perObjectIndexes.back();
	int32_t offsetStart = objIndex * dynamicOffsetCount;
	uint32_t* dynOffsets = dynamicOffsets.data() + offsetStart;

	uint8_t* ringCPUData = (uint8_t*)(ringBuffer->GetMappedPointer());
	uint64_t ringOffset = ringBuffer->AllocateMemory(it->second.dataSize);
	uint64_t bufferSize = it->second.dataSize;

	memcpy(ringCPUData + ringOffset, dataPtr, bufferSize);

	dynOffsets[it->second.dynamicIndex] = ringOffset;
}

void VKMaterial::SetGlobalUniform(const std::string& name, void* dataPtr, uint32_t size)
{
	auto it = uniformBuffers.find(name);
	if (it == uniformBuffers.end())
	{
		MLOGE("Uniform %s not found.", name.c_str());
		return;
	}

	if (it->second.dataSize != size)
	{
		MLOGE("Uniform %s size not match, dst=%ud src=%ud", name.c_str(), it->second.dataSize, size);
		return;
	}

	if (it->second.dataContent.size() != size) {
		it->second.dataContent.resize(size);
	}

	it->second.global = true;
	memcpy(it->second.dataContent.data(), dataPtr, size);
}

void VKMaterial::SetTexture(const std::string& name, VKTexture* texture)
{
	auto it = textures.find(name);
	if (it == textures.end())
	{
		MLOGE("Texture %s not found.", name.c_str());
		return;
	}

	if (texture == nullptr)
	{
		MLOGE("Texture %s can't be null.", name.c_str());
		return;
	}

	if (it->second.texture != texture)
	{
		it->second.texture = texture;
		descriptorSet->WriteImage(name, texture);
	}
}

void VKMaterial::SetInputAttachment(const std::string& name, VKTexture* texture)
{
	SetTexture(name, texture);
}

void VKMaterial::SetStorageBuffer(const std::string& name, DVKBuffer* buffer)
{
	auto it = storageBuffers.find(name);
	if (it == storageBuffers.end())
	{
		MLOGE("StorageBuffer %s not found.", name.c_str());
		return;
	}

	if (buffer == nullptr)
	{
		MLOGE("StorageBuffer %s can't be null.", name.c_str());
		return;
	}

	if (it->second.bufferInfo.buffer != buffer->buffer)
	{
		it->second.dataSize = buffer->size;
		it->second.bufferInfo.buffer = buffer->buffer;
		it->second.bufferInfo.offset = 0;
		it->second.bufferInfo.range = buffer->size;
		descriptorSet->WriteBuffer(name, buffer);
	}
}