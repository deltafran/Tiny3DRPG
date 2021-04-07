#pragma once

#include "VKUtils.h"
#include "DVKBuffer.h"
#include "VKTexture.h"
#include "VKShader.h"
#include "VKPipeline.h"
#include "VKModel.h"
#include "VKRenderTarget.h"
#include "Alignment.h"

struct VKSimulateBuffer
{
	std::vector<uint8_t>		dataContent;
	bool                    global = false;
	uint32_t					dataSize = 0;
	uint32_t					set = 0;
	uint32_t					binding = 0;
	uint32_t					dynamicIndex = 0;
	VkDescriptorType		descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	VkShaderStageFlags		stageFlags = 0;
	VkDescriptorBufferInfo	bufferInfo;
};

struct VKSimulateTexture
{
	uint32_t              set = 0;
	uint32_t              binding = 0;
	VkDescriptorType    descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	VkShaderStageFlags  stageFlags = 0;
	VKTexture* texture = nullptr;
};

class VKRingBuffer
{
public:
	VKRingBuffer()
	{

	}

	virtual ~VKRingBuffer()
	{
		realBuffer->UnMap();
		delete realBuffer;
		realBuffer = nullptr;
	}

	void* GetMappedPointer()
	{
		return realBuffer->mapped;
	}

	uint64_t AllocateMemory(uint64_t size)
	{
		uint64_t allocationOffset = Align<uint64_t>(bufferOffset, minAlignment);

		if (allocationOffset + size <= bufferSize)
		{
			bufferOffset = allocationOffset + size;
			return allocationOffset;
		}

		bufferOffset = 0;
		return bufferOffset;
	}

public:
	VkDevice		device = VK_NULL_HANDLE;
	uint64_t			bufferSize = 0;
	uint64_t			bufferOffset = 0;
	uint32_t			minAlignment = 0;
	DVKBuffer* realBuffer = nullptr;
};

class VKMaterial
{
private:

	typedef std::unordered_map<std::string, VKSimulateBuffer>		BuffersMap;
	typedef std::unordered_map<std::string, VKSimulateTexture>		TexturesMap;
	typedef std::shared_ptr<VulkanDevice>							VulkanDeviceRef;

	VKMaterial()
	{

	}

public:
	virtual ~VKMaterial();

	static VKMaterial* Create(std::shared_ptr<VulkanDevice> vulkanDevice, VkRenderPass renderPass, VkPipelineCache pipelineCache, VKShader* shader);

	static VKMaterial* Create(std::shared_ptr<VulkanDevice> vulkanDevice, VKRenderTarget* renderTarget, VkPipelineCache pipelineCache, VKShader* shader);

	void PreparePipeline();

	void BeginObject();

	void EndObject();

	void BeginFrame();

	void EndFrame();

	void BindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint, int32_t objIndex);

	void SetLocalUniform(const std::string& name, void* dataPtr, uint32_t size);

	void SetTexture(const std::string& name, VKTexture* texture);

	void SetGlobalUniform(const std::string& name, void* dataPtr, uint32_t size);

	void SetStorageBuffer(const std::string& name, DVKBuffer* buffer);

	void SetInputAttachment(const std::string& name, VKTexture* texture);

	inline VkPipeline GetPipeline() const
	{
		return pipeline->pipeline;
	}

	inline VkPipelineLayout GetPipelineLayout() const
	{
		return pipeline->pipelineLayout;
	}

	inline std::vector<VkDescriptorSet>& GetDescriptorSets() const
	{
		return descriptorSet->descriptorSets;
	}

private:
	static void InitRingBuffer(std::shared_ptr<VulkanDevice> vulkanDevice);

	static void DestroyRingBuffer();

	void Prepare();

private:

	static VKRingBuffer* ringBuffer;
	static int32_t			ringBufferRefCount;

public:

	VulkanDeviceRef			vulkanDevice = nullptr;
	VKShader* shader = nullptr;

	VkRenderPass            renderPass = VK_NULL_HANDLE;
	VkPipelineCache         pipelineCache = VK_NULL_HANDLE;

	VKGfxPipelineInfo      pipelineInfo;
	VKGfxPipeline* pipeline = nullptr;
	VKDescriptorSet* descriptorSet = nullptr;

	uint32_t					dynamicOffsetCount;
	std::vector<uint32_t>		globalOffsets;
	std::vector<uint32_t>     dynamicOffsets;
	std::vector<uint32_t>		perObjectIndexes;

	BuffersMap				uniformBuffers;
	BuffersMap				storageBuffers;
	TexturesMap				textures;

	bool                    actived = false;
};