#pragma once

#include "VKUtils.h"
#include "VKBuffer.h"
#include "VKTexture.h"
#include "VKShader.h"
#include "VKPipeline.h"
#include "VKMaterial.h"

class VKCompute
{
private:

    typedef std::unordered_map<std::string, VKSimulateBuffer>    BuffersMap;
    typedef std::unordered_map<std::string, VKSimulateTexture>   TexturesMap;
    typedef std::shared_ptr<VulkanDevice>                         VulkanDeviceRef;

    VKCompute()
    {

    }

public:
    virtual ~VKCompute();

    static VKCompute* Create(std::shared_ptr<VulkanDevice> vulkanDevice, VkPipelineCache pipelineCache, VKShader* shader);

    void BindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint);

    void BindDispatch(VkCommandBuffer commandBuffer, int groupX, int groupY, int groupZ);

    void SetUniform(const std::string& name, void* dataPtr, uint32_t size);

    void SetTexture(const std::string& name, VKTexture* texture);

    void SetStorageTexture(const std::string& name, VKTexture* texture);

    void SetStorageBuffer(const std::string& name, VKBuffer* buffer);

    inline VkPipeline GetPipeline() const
    {
        return pipeline;
    }

    inline VkPipelineLayout GetPipelineLayout() const
    {
        return shader->pipelineLayout;
    }

    inline std::vector<VkDescriptorSet>& GetDescriptorSets() const
    {
        return descriptorSet->descriptorSets;
    }

private:
    static void InitRingBuffer(std::shared_ptr<VulkanDevice> vulkanDevice);

    static void DestroyRingBuffer();

    void Prepare();

    void PreparePipeline();

private:

    static VKRingBuffer* ringBuffer;
    static int32_t            ringBufferRefCount;

public:

    VulkanDeviceRef             vulkanDevice = nullptr;
    VKShader* shader = nullptr;

    VkPipelineCache             pipelineCache = VK_NULL_HANDLE;
    VkPipeline                  pipeline = VK_NULL_HANDLE;

    VKDescriptorSet* descriptorSet = nullptr;

    uint32_t                      dynamicOffsetCount;
    std::vector<uint32_t>         dynamicOffsets;

    BuffersMap					uniformBuffers;
    BuffersMap					storageBuffers;
    TexturesMap                 textures;
};