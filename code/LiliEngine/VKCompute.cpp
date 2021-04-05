#include "stdafx.h"
#include "VKCompute.h"
#include "VulkanDevice.h"

VKRingBuffer* VKCompute::ringBuffer = nullptr;
int32_t            VKCompute::ringBufferRefCount = 0;

void VKCompute::InitRingBuffer(std::shared_ptr<VulkanDevice> vulkanDevice)
{
    ringBuffer = new VKRingBuffer();
    ringBuffer->device = vulkanDevice->GetInstanceHandle();
    ringBuffer->bufferSize = 8 * 1024 * 1024; // 8MB
    ringBuffer->bufferOffset = ringBuffer->bufferSize;
    ringBuffer->minAlignment = vulkanDevice->GetLimits().minUniformBufferOffsetAlignment;
    ringBuffer->realBuffer = VKBuffer::CreateBuffer(
        vulkanDevice,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        ringBuffer->bufferSize
    );
    ringBuffer->realBuffer->Map();
    ringBufferRefCount = 0;
}

void VKCompute::DestroyRingBuffer()
{
    delete ringBuffer;
    ringBuffer = nullptr;
    ringBufferRefCount = 0;
}

VKCompute::~VKCompute()
{
    delete descriptorSet;
    descriptorSet = nullptr;

    textures.clear();
    uniformBuffers.clear();

    vkDestroyPipeline(vulkanDevice->GetInstanceHandle(), pipeline, VULKAN_CPU_ALLOCATOR);
    pipeline = VK_NULL_HANDLE;

    ringBufferRefCount -= 1;
    if (ringBufferRefCount == 0) {
        DestroyRingBuffer();
    }

    shader = nullptr;
    vulkanDevice = nullptr;
}

VKCompute* VKCompute::Create(std::shared_ptr<VulkanDevice> vulkanDevice, VkPipelineCache pipelineCache, VKShader* shader)
{
    if (ringBufferRefCount == 0) {
        InitRingBuffer(vulkanDevice);
    }
    ringBufferRefCount += 1;

    VKCompute* processor = new VKCompute();
    processor->vulkanDevice = vulkanDevice;
    processor->shader = shader;
    processor->pipelineCache = pipelineCache;
    processor->Prepare();
    processor->PreparePipeline();

    return processor;
}

void VKCompute::Prepare()
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
    dynamicOffsets.resize(dynamicOffsetCount);

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

void VKCompute::PreparePipeline()
{
    VkDevice device = vulkanDevice->GetInstanceHandle();
    if (pipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(device, pipeline, VULKAN_CPU_ALLOCATOR);
        pipeline = VK_NULL_HANDLE;
    }

    VkComputePipelineCreateInfo computeCreateInfo;
    ZeroVulkanStruct(computeCreateInfo, VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO);
    computeCreateInfo.layout = shader->pipelineLayout;
    computeCreateInfo.stage = shader->shaderStageCreateInfos[0];
    VERIFYVULKANRESULT(vkCreateComputePipelines(device, pipelineCache, 1, &computeCreateInfo, VULKAN_CPU_ALLOCATOR, &pipeline));
}

void VKCompute::BindDispatch(VkCommandBuffer commandBuffer, int groupX, int groupY, int groupZ)
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE);
    vkCmdDispatch(commandBuffer, groupX, groupY, groupZ);
}

void VKCompute::BindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint)
{
    uint32_t* dynOffsets = dynamicOffsets.data();

    vkCmdBindDescriptorSets(
        commandBuffer,
        bindPoint,
        GetPipelineLayout(),
        0, GetDescriptorSets().size(), GetDescriptorSets().data(),
        dynamicOffsetCount, dynOffsets
    );
}

void VKCompute::SetStorageBuffer(const std::string& name, VKBuffer* buffer)
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

void VKCompute::SetUniform(const std::string& name, void* dataPtr, uint32_t size)
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

    uint8_t* ringCPUData = (uint8_t*)(ringBuffer->GetMappedPointer());
    uint64_t ringOffset = ringBuffer->AllocateMemory(it->second.dataSize);
    uint64_t bufferSize = it->second.dataSize;

    memcpy(ringCPUData + ringOffset, dataPtr, bufferSize);

    dynamicOffsets[it->second.dynamicIndex] = ringOffset;
}

void VKCompute::SetStorageTexture(const std::string& name, VKTexture* texture)
{
    SetTexture(name, texture);
}

void VKCompute::SetTexture(const std::string& name, VKTexture* texture)
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