#pragma once

#include "VKCommandBuffer.h"
#include "VulkanGlobals.h"
#include "RHIDefinitions.h"

class DVKTexture
{
public:
	DVKTexture()
	{

	}

	~DVKTexture()
	{
		if (imageView != VK_NULL_HANDLE)
		{
			vkDestroyImageView(device, imageView, VULKAN_CPU_ALLOCATOR);
			imageView = VK_NULL_HANDLE;
		}

		if (image != VK_NULL_HANDLE)
		{
			vkDestroyImage(device, image, VULKAN_CPU_ALLOCATOR);
			image = VK_NULL_HANDLE;
		}

		if (imageSampler != VK_NULL_HANDLE)
		{
			vkDestroySampler(device, imageSampler, VULKAN_CPU_ALLOCATOR);
			imageSampler = VK_NULL_HANDLE;
		}

		if (imageMemory != VK_NULL_HANDLE)
		{
			vkFreeMemory(device, imageMemory, VULKAN_CPU_ALLOCATOR);
			imageMemory = VK_NULL_HANDLE;
		}
	}

	void UpdateSampler(
		VkFilter magFilter = VK_FILTER_LINEAR,
		VkFilter minFilter = VK_FILTER_LINEAR,
		VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT
	);

	static DVKTexture* Create2D(
		const uint8_t* rgbaData,
		uint32_t size,
		VkFormat format,
		int32_t width,
		int32_t height,
		std::shared_ptr<VulkanDevice> vulkanDevice,
		VKCommandBuffer* cmdBuffer,
		VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		ImageLayoutBarrier imageLayout = ImageLayoutBarrier::PixelShaderRead
	);

	static DVKTexture* Create2D(
		const std::string& filename,
		std::shared_ptr<VulkanDevice> vulkanDevice,
		VKCommandBuffer* cmdBuffer,
		VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		ImageLayoutBarrier imageLayout = ImageLayoutBarrier::PixelShaderRead
	);

	static DVKTexture* CreateAttachment(
		std::shared_ptr<VulkanDevice> vulkanDevice,
		VkFormat format,
		VkImageAspectFlags aspect,
		int32_t width,
		int32_t height,
		VkImageUsageFlags usage
	);

	static DVKTexture* CreateRenderTarget(
		std::shared_ptr<VulkanDevice> vulkanDevice,
		VkFormat format,
		VkImageAspectFlags aspect,
		int32_t width,
		int32_t height,
		VkImageUsageFlags usage,
		VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT
	);

	static DVKTexture* Create2D(
		std::shared_ptr<VulkanDevice> vulkanDevice,
		VKCommandBuffer* cmdBuffer,
		VkFormat format,
		VkImageAspectFlags aspect,
		int32_t width,
		int32_t height,
		VkImageUsageFlags usage,
		VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT,
		ImageLayoutBarrier imageLayout = ImageLayoutBarrier::Undefined
	);

	static DVKTexture* CreateCube(
		std::shared_ptr<VulkanDevice> vulkanDevice,
		VKCommandBuffer* cmdBuffer,
		VkFormat format,
		VkImageAspectFlags aspect,
		int32_t width,
		int32_t height,
		bool mipmaps,
		VkImageUsageFlags usage,
		VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT,
		ImageLayoutBarrier imageLayout = ImageLayoutBarrier::Undefined
	);

	static DVKTexture* CreateCube(
		const std::vector<std::string> filenames,
		std::shared_ptr<VulkanDevice> vulkanDevice,
		VKCommandBuffer* cmdBuffer,
		ImageLayoutBarrier imageLayout = ImageLayoutBarrier::PixelShaderRead
	);

	static DVKTexture* CreateCubeRenderTarget(
		std::shared_ptr<VulkanDevice> vulkanDevice,
		VkFormat format,
		VkImageAspectFlags aspect,
		int32_t width,
		int32_t height,
		VkImageUsageFlags usage,
		VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT
	);

	static DVKTexture* Create2DArray(
		const std::vector<std::string> filenames,
		std::shared_ptr<VulkanDevice> vulkanDevice,
		VKCommandBuffer* cmdBuffer,
		ImageLayoutBarrier imageLayout = ImageLayoutBarrier::PixelShaderRead
	);

	static DVKTexture* Create2DArray(
		std::shared_ptr<VulkanDevice> vulkanDevice,
		VKCommandBuffer* cmdBuffer,
		VkFormat format,
		VkImageAspectFlags aspect,
		int32_t width,
		int32_t height,
		int32_t depth,
		VkImageUsageFlags usage,
		VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT,
		ImageLayoutBarrier imageLayout = ImageLayoutBarrier::Undefined
	);

	static DVKTexture* Create3D(
		VkFormat format,
		const uint8_t* rgbaData,
		int32_t size,
		int32_t width,
		int32_t height,
		int32_t depth,
		std::shared_ptr<VulkanDevice> vulkanDevice,
		VKCommandBuffer* cmdBuffer,
		ImageLayoutBarrier imageLayout = ImageLayoutBarrier::PixelShaderRead
	);

public:
	VkDevice						device = nullptr;

	VkImage                         image = VK_NULL_HANDLE;
	VkImageLayout                   imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkDeviceMemory                  imageMemory = VK_NULL_HANDLE;
	VkImageView                     imageView = VK_NULL_HANDLE;
	VkSampler                       imageSampler = VK_NULL_HANDLE;
	VkDescriptorImageInfo           descriptorInfo;

	int32_t                           width = 0;
	int32_t                          height = 0;
	int32_t							depth = 1;
	int32_t                           mipLevels = 0;
	int32_t							layerCount = 1;
	VkSampleCountFlagBits           numSamples = VK_SAMPLE_COUNT_1_BIT;
	VkFormat                        format = VK_FORMAT_R8G8B8A8_UNORM;

	bool							isCubeMap = false;
};