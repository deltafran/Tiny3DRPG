﻿#include "stdafx.h"
#include "VKTexture.h"
#include "DVKBuffer.h"
#include "CoreMath2.h"
#include "VulkanDevice.h"
#include "VulkanMemory.h"
#include "VKUtils.h"
#include "ImageLoader.h"

VKTexture* VKTexture::Create2D(const uint8_t* rgbaData, uint32_t size, VkFormat format, int32_t width, int32_t height, std::shared_ptr<VulkanDevice> vulkanDevice, VKCommandBuffer* cmdBuffer, VkImageUsageFlags imageUsageFlags, ImageLayoutBarrier imageLayout)
{
	int32_t mipLevels = math::FloorToInt(math::Log2(math::Max(width, height))) + 1;
	VkDevice device = vulkanDevice->GetInstanceHandle();

	DVKBuffer* stagingBuffer = DVKBuffer::CreateBuffer(vulkanDevice, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, size);
	stagingBuffer->Map();
	stagingBuffer->CopyFrom((void*)rgbaData, size);
	stagingBuffer->UnMap();

	uint32_t memoryTypeIndex = 0;
	VkMemoryRequirements memReqs = {};
	VkMemoryAllocateInfo memAllocInfo;
	ZeroVulkanStruct(memAllocInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);

	// image info
	VkImage                         image = VK_NULL_HANDLE;
	VkDeviceMemory                  imageMemory = VK_NULL_HANDLE;
	VkImageView                     imageView = VK_NULL_HANDLE;
	VkSampler                       imageSampler = VK_NULL_HANDLE;
	VkDescriptorImageInfo           descriptorInfo = {};

	if (!(imageUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
		imageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}

	if (!(imageUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)) {
		imageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}

	// image
	VkImageCreateInfo imageCreateInfo;
	ZeroVulkanStruct(imageCreateInfo, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.mipLevels = mipLevels;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.extent = { (uint32_t)width, (uint32_t)height, 1 };
	imageCreateInfo.usage = imageUsageFlags;
	VERIFYVULKANRESULT(vkCreateImage(device, &imageCreateInfo, VULKAN_CPU_ALLOCATOR, &image));

	// bind image buffer
	vkGetImageMemoryRequirements(device, image, &memReqs);
	vulkanDevice->GetMemoryManager().GetMemoryTypeFromProperties(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryTypeIndex);
	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = memoryTypeIndex;
	VERIFYVULKANRESULT(vkAllocateMemory(device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &imageMemory));
	VERIFYVULKANRESULT(vkBindImageMemory(device, image, imageMemory, 0));

	// start record
	cmdBuffer->Begin();

	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.levelCount = 1;
	subresourceRange.layerCount = 1;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.baseMipLevel = 0;

	// undefined to TransferDest
	vkutils::ImagePipelineBarrier(cmdBuffer->cmdBuffer, image, ImageLayoutBarrier::Undefined, ImageLayoutBarrier::TransferDest, subresourceRange);

	VkBufferImageCopy bufferCopyRegion = {};
	bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	bufferCopyRegion.imageSubresource.mipLevel = 0;
	bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
	bufferCopyRegion.imageSubresource.layerCount = 1;
	bufferCopyRegion.imageExtent.width = width;
	bufferCopyRegion.imageExtent.height = height;
	bufferCopyRegion.imageExtent.depth = 1;

	// copy buffer to image
	vkCmdCopyBufferToImage(cmdBuffer->cmdBuffer, stagingBuffer->buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferCopyRegion);

	// TransferDest to TransferSrc
	vkutils::ImagePipelineBarrier(cmdBuffer->cmdBuffer, image, ImageLayoutBarrier::TransferDest, ImageLayoutBarrier::TransferSource, subresourceRange);

	// Generate the mip chain
	for (uint32_t i = 1; i < mipLevels; i++)
	{
		VkImageBlit imageBlit = {};

		int32_t mip0Width = math::Max(width >> (i - 1), 1);
		int32_t mip0Height = math::Max(height >> (i - 1), 1);
		int32_t mip1Width = math::Max(width >> (i - 0), 1);
		int32_t mip1Height = math::Max(height >> (i - 0), 1);

		imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBlit.srcSubresource.layerCount = 1;
		imageBlit.srcSubresource.mipLevel = i - 1;
		imageBlit.srcOffsets[1].x = int32_t(mip0Width);
		imageBlit.srcOffsets[1].y = int32_t(mip0Height);
		imageBlit.srcOffsets[1].z = 1;

		imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBlit.dstSubresource.layerCount = 1;
		imageBlit.dstSubresource.mipLevel = i;
		imageBlit.dstOffsets[1].x = int32_t(mip1Width);
		imageBlit.dstOffsets[1].y = int32_t(mip1Height);
		imageBlit.dstOffsets[1].z = 1;

		VkImageSubresourceRange mipSubRange = {};
		mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		mipSubRange.baseMipLevel = i;
		mipSubRange.levelCount = 1;
		mipSubRange.layerCount = 1;
		mipSubRange.baseArrayLayer = 0;

		// undefined to dst
		vkutils::ImagePipelineBarrier(cmdBuffer->cmdBuffer, image, ImageLayoutBarrier::Undefined, ImageLayoutBarrier::TransferDest, mipSubRange);

		// blit image
		vkCmdBlitImage(cmdBuffer->cmdBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);

		// dst to src
		vkutils::ImagePipelineBarrier(cmdBuffer->cmdBuffer, image, ImageLayoutBarrier::TransferDest, ImageLayoutBarrier::TransferSource, mipSubRange);
	}

	subresourceRange.levelCount = mipLevels;

	// dst to layout
	vkutils::ImagePipelineBarrier(cmdBuffer->cmdBuffer, image, ImageLayoutBarrier::TransferSource, imageLayout, subresourceRange);

	cmdBuffer->End();
	cmdBuffer->Submit();

	delete stagingBuffer;

	VkSamplerCreateInfo samplerInfo;
	ZeroVulkanStruct(samplerInfo, VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO);
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	samplerInfo.maxAnisotropy = 1.0;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxLod = mipLevels;
	samplerInfo.minLod = 0.0f;
	VERIFYVULKANRESULT(vkCreateSampler(device, &samplerInfo, VULKAN_CPU_ALLOCATOR, &imageSampler));

	VkImageViewCreateInfo viewInfo;
	ZeroVulkanStruct(viewInfo, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.layerCount = 1;
	viewInfo.subresourceRange.levelCount = mipLevels;
	VERIFYVULKANRESULT(vkCreateImageView(device, &viewInfo, VULKAN_CPU_ALLOCATOR, &imageView));

	descriptorInfo.sampler = imageSampler;
	descriptorInfo.imageView = imageView;
	descriptorInfo.imageLayout = vkutils::GetImageLayout(imageLayout);

	VKTexture* texture = new VKTexture();
	texture->descriptorInfo = descriptorInfo;
	texture->format = format;
	texture->height = height;
	texture->image = image;
	texture->imageLayout = vkutils::GetImageLayout(imageLayout);
	texture->imageMemory = imageMemory;
	texture->imageSampler = imageSampler;
	texture->imageView = imageView;
	texture->device = device;
	texture->width = width;
	texture->mipLevels = mipLevels;
	texture->layerCount = 1;

	return texture;
}

VKTexture* VKTexture::Create2D(const std::string& filename, std::shared_ptr<VulkanDevice> vulkanDevice, VKCommandBuffer* cmdBuffer, VkImageUsageFlags imageUsageFlags, ImageLayoutBarrier imageLayout)
{
	uint32_t dataSize = 0;
	uint8_t* dataPtr = nullptr;
	if (!FileManager::ReadFile(filename, dataPtr, dataSize))
	{
		MLOGE("Failed load image : %s", filename.c_str());
		return nullptr;
	}

	int32_t comp = 0;
	int32_t width = 0;
	int32_t height = 0;
	uint8_t* rgbaData = ImageLoader::LoadFromMemory(dataPtr, dataSize, &width, &height, &comp, 4);

	delete[] dataPtr;
	dataPtr = nullptr;

	if (rgbaData == nullptr)
	{
		MLOGE("Failed load image : %s", filename.c_str());
		return nullptr;
	}

	VKTexture* texture = Create2D(rgbaData, width * height * 4, VK_FORMAT_R8G8B8A8_UNORM, width, height, vulkanDevice, cmdBuffer, imageUsageFlags, imageLayout);

	ImageLoader::Free(rgbaData);

	return texture;
}

VKTexture* VKTexture::CreateCubeRenderTarget(std::shared_ptr<VulkanDevice> vulkanDevice, VkFormat format, VkImageAspectFlags aspect, int32_t width, int32_t height, VkImageUsageFlags usage, VkSampleCountFlagBits sampleCount)
{
	VKTexture* texture = CreateCube(vulkanDevice, nullptr, format, aspect, width, height, false, usage, sampleCount);
	texture->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	texture->descriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	return texture;
}

VKTexture* VKTexture::CreateCube(std::shared_ptr<VulkanDevice> vulkanDevice, VKCommandBuffer* cmdBuffer, VkFormat format, VkImageAspectFlags aspect, int32_t width, int32_t height, bool mipmaps, VkImageUsageFlags usage, VkSampleCountFlagBits sampleCount, ImageLayoutBarrier imageLayout)
{
	VkDevice device = vulkanDevice->GetInstanceHandle();
	int32_t mipLevels = 1;
	if (mipmaps) {
		mipLevels = math::FloorToInt(math::Log2(math::Max(width, height))) + 1;
	}

	uint32_t memoryTypeIndex = 0;
	VkMemoryRequirements memReqs = {};
	VkMemoryAllocateInfo memAllocInfo;
	ZeroVulkanStruct(memAllocInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);

	// image info
	VkImage                         image = VK_NULL_HANDLE;
	VkDeviceMemory                  imageMemory = VK_NULL_HANDLE;
	VkImageView                     imageView = VK_NULL_HANDLE;
	VkSampler                       imageSampler = VK_NULL_HANDLE;
	VkDescriptorImageInfo           descriptorInfo = {};

	// image
	VkImageCreateInfo imageCreateInfo;
	ZeroVulkanStruct(imageCreateInfo, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.mipLevels = mipLevels;
	imageCreateInfo.arrayLayers = 6;
	imageCreateInfo.samples = sampleCount;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.extent = { (uint32_t)width, (uint32_t)height, 1 };
	imageCreateInfo.usage = usage;
	imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	VERIFYVULKANRESULT(vkCreateImage(device, &imageCreateInfo, VULKAN_CPU_ALLOCATOR, &image));

	// bind image buffer
	vkGetImageMemoryRequirements(device, image, &memReqs);
	vulkanDevice->GetMemoryManager().GetMemoryTypeFromProperties(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryTypeIndex);
	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = memoryTypeIndex;
	VERIFYVULKANRESULT(vkAllocateMemory(device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &imageMemory));
	VERIFYVULKANRESULT(vkBindImageMemory(device, image, imageMemory, 0));

	VkSamplerCreateInfo samplerInfo;
	ZeroVulkanStruct(samplerInfo, VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO);
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	samplerInfo.maxAnisotropy = 1.0;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxLod = mipLevels;
	samplerInfo.minLod = 0.0f;
	VERIFYVULKANRESULT(vkCreateSampler(device, &samplerInfo, VULKAN_CPU_ALLOCATOR, &imageSampler));

	VkImageViewCreateInfo viewInfo;
	ZeroVulkanStruct(viewInfo, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	viewInfo.format = format;
	viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	viewInfo.subresourceRange.aspectMask = aspect;
	viewInfo.subresourceRange.layerCount = 6;
	viewInfo.subresourceRange.levelCount = mipLevels;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	VERIFYVULKANRESULT(vkCreateImageView(device, &viewInfo, VULKAN_CPU_ALLOCATOR, &imageView));

	if (cmdBuffer != nullptr && imageLayout != ImageLayoutBarrier::Undefined)
	{
		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.levelCount = mipLevels;
		subresourceRange.layerCount = 6;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.baseMipLevel = 0;

		cmdBuffer->Begin();

		vkutils::ImagePipelineBarrier(cmdBuffer->cmdBuffer, image, ImageLayoutBarrier::Undefined, imageLayout, subresourceRange);

		cmdBuffer->Submit();
	}
	else
	{
		imageLayout = ImageLayoutBarrier::Undefined;
	}

	descriptorInfo.sampler = imageSampler;
	descriptorInfo.imageView = imageView;
	descriptorInfo.imageLayout = vkutils::GetImageLayout(imageLayout);

	VKTexture* texture = new VKTexture();
	texture->descriptorInfo = descriptorInfo;
	texture->format = format;
	texture->width = width;
	texture->height = height;
	texture->depth = 6;
	texture->image = image;
	texture->imageLayout = vkutils::GetImageLayout(imageLayout);
	texture->imageMemory = imageMemory;
	texture->imageSampler = imageSampler;
	texture->imageView = imageView;
	texture->device = device;
	texture->mipLevels = mipLevels;
	texture->layerCount = 1;
	texture->numSamples = sampleCount;
	texture->isCubeMap = true;

	return texture;
}

VKTexture* VKTexture::CreateAttachment(std::shared_ptr<VulkanDevice> vulkanDevice, VkFormat format, VkImageAspectFlags aspect, int32_t width, int32_t height, VkImageUsageFlags usage)
{
	VKTexture* texture = Create2D(vulkanDevice, nullptr, format, aspect, width, height, usage);
	texture->descriptorInfo.sampler = VK_NULL_HANDLE;
	texture->descriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	texture->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	return texture;
}

VKTexture* VKTexture::CreateRenderTarget(std::shared_ptr<VulkanDevice> vulkanDevice, VkFormat format, VkImageAspectFlags aspect, int32_t width, int32_t height, VkImageUsageFlags usage, VkSampleCountFlagBits sampleCount)
{
	VKTexture* texture = Create2D(vulkanDevice, nullptr, format, aspect, width, height, usage, sampleCount);
	texture->descriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	texture->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	return texture;
}

VKTexture* VKTexture::Create2DArray(
	std::shared_ptr<VulkanDevice> vulkanDevice,
	VKCommandBuffer* cmdBuffer,
	VkFormat format,
	VkImageAspectFlags aspect,
	int32_t width,
	int32_t height,
	int32_t numArray,
	VkImageUsageFlags usage,
	VkSampleCountFlagBits sampleCount,
	ImageLayoutBarrier imageLayout
)
{
	VkDevice device = vulkanDevice->GetInstanceHandle();

	uint32_t memoryTypeIndex = 0;
	VkMemoryRequirements memReqs = {};
	VkMemoryAllocateInfo memAllocInfo;
	ZeroVulkanStruct(memAllocInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);

	int32_t mipLevels = 1;

	// image info
	VkImage                         image = VK_NULL_HANDLE;
	VkDeviceMemory                  imageMemory = VK_NULL_HANDLE;
	VkImageView                     imageView = VK_NULL_HANDLE;
	VkSampler                       imageSampler = VK_NULL_HANDLE;
	VkDescriptorImageInfo           descriptorInfo = {};

	// image
	VkImageCreateInfo imageCreateInfo;
	ZeroVulkanStruct(imageCreateInfo, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.mipLevels = mipLevels;
	imageCreateInfo.arrayLayers = numArray;
	imageCreateInfo.samples = sampleCount;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.extent = { (uint32_t)width, (uint32_t)height, (uint32_t)1 };
	imageCreateInfo.usage = usage;
	VERIFYVULKANRESULT(vkCreateImage(device, &imageCreateInfo, VULKAN_CPU_ALLOCATOR, &image));

	// bind image buffer
	vkGetImageMemoryRequirements(device, image, &memReqs);
	vulkanDevice->GetMemoryManager().GetMemoryTypeFromProperties(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryTypeIndex);
	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = memoryTypeIndex;
	VERIFYVULKANRESULT(vkAllocateMemory(device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &imageMemory));
	VERIFYVULKANRESULT(vkBindImageMemory(device, image, imageMemory, 0));

	VkSamplerCreateInfo samplerInfo;
	ZeroVulkanStruct(samplerInfo, VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO);
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	samplerInfo.maxAnisotropy = 1.0;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxLod = mipLevels;
	samplerInfo.minLod = 0.0f;
	VERIFYVULKANRESULT(vkCreateSampler(device, &samplerInfo, VULKAN_CPU_ALLOCATOR, &imageSampler));

	VkImageViewCreateInfo viewInfo;
	ZeroVulkanStruct(viewInfo, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
	viewInfo.format = format;
	viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	viewInfo.subresourceRange.aspectMask = aspect;
	viewInfo.subresourceRange.layerCount = numArray;
	viewInfo.subresourceRange.levelCount = mipLevels;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	VERIFYVULKANRESULT(vkCreateImageView(device, &viewInfo, VULKAN_CPU_ALLOCATOR, &imageView));

	if (cmdBuffer != nullptr && imageLayout != ImageLayoutBarrier::Undefined)
	{
		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.levelCount = mipLevels;
		subresourceRange.layerCount = numArray;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.baseMipLevel = 0;

		cmdBuffer->Begin();

		vkutils::ImagePipelineBarrier(cmdBuffer->cmdBuffer, image, ImageLayoutBarrier::Undefined, imageLayout, subresourceRange);

		cmdBuffer->Submit();
	}
	else
	{
		imageLayout = ImageLayoutBarrier::Undefined;
	}

	descriptorInfo.sampler = imageSampler;
	descriptorInfo.imageView = imageView;
	descriptorInfo.imageLayout = vkutils::GetImageLayout(imageLayout);

	VKTexture* texture = new VKTexture();
	texture->descriptorInfo = descriptorInfo;
	texture->format = format;
	texture->width = width;
	texture->height = height;
	texture->depth = 1;
	texture->image = image;
	texture->imageLayout = vkutils::GetImageLayout(imageLayout);
	texture->imageMemory = imageMemory;
	texture->imageSampler = imageSampler;
	texture->imageView = imageView;
	texture->device = device;
	texture->mipLevels = mipLevels;
	texture->layerCount = numArray;
	texture->numSamples = sampleCount;

	return texture;
}

VKTexture* VKTexture::Create2D(std::shared_ptr<VulkanDevice> vulkanDevice, VKCommandBuffer* cmdBuffer, VkFormat format, VkImageAspectFlags aspect, int32_t width, int32_t height, VkImageUsageFlags usage, VkSampleCountFlagBits sampleCount, ImageLayoutBarrier imageLayout)
{
	VkDevice device = vulkanDevice->GetInstanceHandle();

	uint32_t memoryTypeIndex = 0;
	VkMemoryRequirements memReqs = {};
	VkMemoryAllocateInfo memAllocInfo;
	ZeroVulkanStruct(memAllocInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);

	int32_t mipLevels = 1;

	// image info
	VkImage                         image = VK_NULL_HANDLE;
	VkDeviceMemory                  imageMemory = VK_NULL_HANDLE;
	VkImageView                     imageView = VK_NULL_HANDLE;
	VkSampler                       imageSampler = VK_NULL_HANDLE;
	VkDescriptorImageInfo           descriptorInfo = {};

	// image
	VkImageCreateInfo imageCreateInfo;
	ZeroVulkanStruct(imageCreateInfo, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.mipLevels = mipLevels;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = sampleCount;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.extent = { (uint32_t)width, (uint32_t)height, (uint32_t)1 };
	imageCreateInfo.usage = usage;
	VERIFYVULKANRESULT(vkCreateImage(device, &imageCreateInfo, VULKAN_CPU_ALLOCATOR, &image));

	// bind image buffer
	vkGetImageMemoryRequirements(device, image, &memReqs);
	vulkanDevice->GetMemoryManager().GetMemoryTypeFromProperties(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryTypeIndex);
	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = memoryTypeIndex;
	VERIFYVULKANRESULT(vkAllocateMemory(device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &imageMemory));
	VERIFYVULKANRESULT(vkBindImageMemory(device, image, imageMemory, 0));

	VkSamplerCreateInfo samplerInfo;
	ZeroVulkanStruct(samplerInfo, VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO);
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	samplerInfo.maxAnisotropy = 1.0;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxLod = mipLevels;
	samplerInfo.minLod = 0.0f;
	VERIFYVULKANRESULT(vkCreateSampler(device, &samplerInfo, VULKAN_CPU_ALLOCATOR, &imageSampler));

	VkImageViewCreateInfo viewInfo;
	ZeroVulkanStruct(viewInfo, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	viewInfo.subresourceRange.aspectMask = aspect;
	viewInfo.subresourceRange.layerCount = 1;
	viewInfo.subresourceRange.levelCount = mipLevels;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	VERIFYVULKANRESULT(vkCreateImageView(device, &viewInfo, VULKAN_CPU_ALLOCATOR, &imageView));

	if (cmdBuffer != nullptr && imageLayout != ImageLayoutBarrier::Undefined)
	{
		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.levelCount = mipLevels;
		subresourceRange.layerCount = 1;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.baseMipLevel = 0;

		cmdBuffer->Begin();

		vkutils::ImagePipelineBarrier(cmdBuffer->cmdBuffer, image, ImageLayoutBarrier::Undefined, imageLayout, subresourceRange);

		cmdBuffer->Submit();
	}
	else
	{
		imageLayout = ImageLayoutBarrier::Undefined;
	}

	descriptorInfo.sampler = imageSampler;
	descriptorInfo.imageView = imageView;
	descriptorInfo.imageLayout = vkutils::GetImageLayout(imageLayout);

	VKTexture* texture = new VKTexture();
	texture->descriptorInfo = descriptorInfo;
	texture->format = format;
	texture->width = width;
	texture->height = height;
	texture->depth = 1;
	texture->image = image;
	texture->imageLayout = vkutils::GetImageLayout(imageLayout);
	texture->imageMemory = imageMemory;
	texture->imageSampler = imageSampler;
	texture->imageView = imageView;
	texture->device = device;
	texture->mipLevels = mipLevels;
	texture->layerCount = 1;
	texture->numSamples = sampleCount;

	return texture;
}

void VKTexture::UpdateSampler(
	VkFilter magFilter,
	VkFilter minFilter,
	VkSamplerMipmapMode mipmapMode,
	VkSamplerAddressMode addressModeU,
	VkSamplerAddressMode addressModeV,
	VkSamplerAddressMode addressModeW
)
{
	VkSamplerCreateInfo samplerInfo;
	ZeroVulkanStruct(samplerInfo, VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO);
	samplerInfo.magFilter = magFilter;
	samplerInfo.minFilter = minFilter;
	samplerInfo.mipmapMode = mipmapMode;
	samplerInfo.addressModeU = addressModeU;
	samplerInfo.addressModeV = addressModeV;
	samplerInfo.addressModeW = addressModeW;
	samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	samplerInfo.maxAnisotropy = 1.0;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxLod = 1.0f;
	VERIFYVULKANRESULT(vkCreateSampler(device, &samplerInfo, VULKAN_CPU_ALLOCATOR, &imageSampler));

	if (descriptorInfo.sampler) {
		vkDestroySampler(device, descriptorInfo.sampler, VULKAN_CPU_ALLOCATOR);
	}
	descriptorInfo.sampler = imageSampler;
}

VKTexture* VKTexture::CreateCube(const std::vector<std::string> filenames, std::shared_ptr<VulkanDevice> vulkanDevice, VKCommandBuffer* cmdBuffer, ImageLayoutBarrier imageLayout)
{
	struct ImageInfo
	{
		int32_t	width = 0;
		int32_t	height = 0;
		int32_t	comp = 0;
		uint8_t* data = nullptr;
		uint32_t	size = 0;
	};

	std::vector<ImageInfo> images(filenames.size());
	for (int32_t i = 0; i < filenames.size(); ++i)
	{
		uint32_t dataSize = 0;
		uint8_t* dataPtr = nullptr;
		if (!FileManager::ReadFile(filenames[i], dataPtr, dataSize))
		{
			MLOGE("Failed load image : %s", filenames[i].c_str());
			return nullptr;
		}

		ImageInfo& imageInfo = images[i];
		imageInfo.data = (uint8_t*)ImageLoader::LoadFloatFromMemory(dataPtr, dataSize, &imageInfo.width, &imageInfo.height, &imageInfo.comp, 4);
		imageInfo.comp = 4;
		imageInfo.size = imageInfo.width * imageInfo.height * imageInfo.comp * 4;

		delete[] dataPtr;
		dataPtr = nullptr;

		if (!imageInfo.data)
		{
			MLOGE("Failed load image : %s", filenames[i].c_str());
			return nullptr;
		}
	}

	// TextureArray
	int32_t width = images[0].width;
	int32_t height = images[0].height;
	int32_t numArray = images.size();
	VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT;
	int32_t mipLevels = math::FloorToInt(math::Log2(math::Max(width, height))) + 1;
	VkDevice device = vulkanDevice->GetInstanceHandle();

	uint32_t memoryTypeIndex = 0;
	VkMemoryRequirements memReqs = {};
	VkMemoryAllocateInfo memAllocInfo;
	ZeroVulkanStruct(memAllocInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);

	// stagingBuffer
	DVKBuffer* stagingBuffer = DVKBuffer::CreateBuffer(
		vulkanDevice,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		images[0].size * 6
	);

	for (int32_t i = 0; i < images.size(); ++i)
	{
		uint8_t* src = images[i].data;
		uint32_t size = images[i].size;
		stagingBuffer->Map(size, size * i);
		stagingBuffer->CopyFrom(src, size);
		stagingBuffer->UnMap();

		ImageLoader::Free(src);
	}

	// image info
	VkImage                image = VK_NULL_HANDLE;
	VkDeviceMemory         imageMemory = VK_NULL_HANDLE;
	VkImageView            imageView = VK_NULL_HANDLE;
	VkSampler              imageSampler = VK_NULL_HANDLE;
	VkDescriptorImageInfo  descriptorInfo = {};

	// 创建image
	VkImageCreateInfo imageCreateInfo;
	ZeroVulkanStruct(imageCreateInfo, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.mipLevels = mipLevels;
	imageCreateInfo.arrayLayers = numArray;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.extent = { (uint32_t)width, (uint32_t)height, 1 };
	imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	VERIFYVULKANRESULT(vkCreateImage(device, &imageCreateInfo, VULKAN_CPU_ALLOCATOR, &image));

	// bind image buffer
	vkGetImageMemoryRequirements(device, image, &memReqs);
	vulkanDevice->GetMemoryManager().GetMemoryTypeFromProperties(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryTypeIndex);
	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = memoryTypeIndex;
	VERIFYVULKANRESULT(vkAllocateMemory(device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &imageMemory));
	VERIFYVULKANRESULT(vkBindImageMemory(device, image, imageMemory, 0));

	// start record
	cmdBuffer->Begin();

	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.levelCount = 1;
	subresourceRange.layerCount = numArray;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.baseArrayLayer = 0;

	vkutils::ImagePipelineBarrier(cmdBuffer->cmdBuffer, image, ImageLayoutBarrier::Undefined, ImageLayoutBarrier::TransferDest, subresourceRange);

	std::vector<VkBufferImageCopy> bufferCopyRegions;
	for (int32_t i = 0; i < images.size(); ++i)
	{
		VkBufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel = 0;
		bufferCopyRegion.imageSubresource.baseArrayLayer = i;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = width;
		bufferCopyRegion.imageExtent.height = height;
		bufferCopyRegion.imageExtent.depth = 1;
		bufferCopyRegion.bufferOffset = images[0].size * i;
		bufferCopyRegions.push_back(bufferCopyRegion);
	}

	vkCmdCopyBufferToImage(cmdBuffer->cmdBuffer, stagingBuffer->buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, bufferCopyRegions.size(), bufferCopyRegions.data());

	vkutils::ImagePipelineBarrier(cmdBuffer->cmdBuffer, image, ImageLayoutBarrier::TransferDest, ImageLayoutBarrier::TransferSource, subresourceRange);

	// Generate the mip chain
	for (uint32_t i = 1; i < mipLevels; i++)
	{
		VkImageBlit imageBlit = {};

		imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBlit.srcSubresource.layerCount = numArray;
		imageBlit.srcSubresource.mipLevel = i - 1;
		imageBlit.srcOffsets[1].x = int32_t(width >> (i - 1));
		imageBlit.srcOffsets[1].y = int32_t(height >> (i - 1));
		imageBlit.srcOffsets[1].z = 1;

		imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBlit.dstSubresource.layerCount = numArray;
		imageBlit.dstSubresource.mipLevel = i;
		imageBlit.dstOffsets[1].x = int32_t(width >> i);
		imageBlit.dstOffsets[1].y = int32_t(height >> i);
		imageBlit.dstOffsets[1].z = 1;

		VkImageSubresourceRange mipSubRange = {};
		mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		mipSubRange.baseMipLevel = i;
		mipSubRange.levelCount = 1;
		mipSubRange.layerCount = numArray;
		mipSubRange.baseArrayLayer = 0;

		vkutils::ImagePipelineBarrier(cmdBuffer->cmdBuffer, image, ImageLayoutBarrier::Undefined, ImageLayoutBarrier::TransferDest, mipSubRange);

		vkCmdBlitImage(cmdBuffer->cmdBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);

		vkutils::ImagePipelineBarrier(cmdBuffer->cmdBuffer, image, ImageLayoutBarrier::TransferDest, ImageLayoutBarrier::TransferSource, mipSubRange);
	}

	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.levelCount = mipLevels;
	subresourceRange.layerCount = numArray;
	subresourceRange.baseMipLevel = 0;

	vkutils::ImagePipelineBarrier(cmdBuffer->cmdBuffer, image, ImageLayoutBarrier::TransferSource, imageLayout, subresourceRange);

	cmdBuffer->End();
	cmdBuffer->Submit();

	delete stagingBuffer;

	VkSamplerCreateInfo samplerInfo;
	ZeroVulkanStruct(samplerInfo, VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO);
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	samplerInfo.maxAnisotropy = 1.0;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxLod = mipLevels;
	samplerInfo.minLod = 0;
	VERIFYVULKANRESULT(vkCreateSampler(device, &samplerInfo, VULKAN_CPU_ALLOCATOR, &imageSampler));

	VkImageViewCreateInfo viewInfo;
	ZeroVulkanStruct(viewInfo, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	viewInfo.image = image;
	viewInfo.format = format;
	viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.layerCount = numArray;
	viewInfo.subresourceRange.levelCount = mipLevels;
	VERIFYVULKANRESULT(vkCreateImageView(device, &viewInfo, VULKAN_CPU_ALLOCATOR, &imageView));

	descriptorInfo.sampler = imageSampler;
	descriptorInfo.imageView = imageView;
	descriptorInfo.imageLayout = vkutils::GetImageLayout(imageLayout);

	VKTexture* texture = new VKTexture();
	texture->descriptorInfo = descriptorInfo;
	texture->format = format;
	texture->height = height;
	texture->image = image;
	texture->imageLayout = vkutils::GetImageLayout(imageLayout);
	texture->imageMemory = imageMemory;
	texture->imageSampler = imageSampler;
	texture->imageView = imageView;
	texture->device = device;
	texture->width = width;
	texture->mipLevels = mipLevels;
	texture->layerCount = numArray;

	return texture;
}

VKTexture* VKTexture::Create2DArray(const std::vector<std::string> filenames, std::shared_ptr<VulkanDevice> vulkanDevice, VKCommandBuffer* cmdBuffer, ImageLayoutBarrier imageLayout)
{
	struct ImageInfo
	{
		int32_t	width = 0;
		int32_t	height = 0;
		int32_t	comp = 0;
		uint8_t* data = nullptr;
		uint32_t	size = 0;
	};

	std::vector<ImageInfo> images(filenames.size());
	for (int32_t i = 0; i < filenames.size(); ++i)
	{
		uint32_t dataSize = 0;
		uint8_t* dataPtr = nullptr;
		if (!FileManager::ReadFile(filenames[i], dataPtr, dataSize))
		{
			MLOGE("Failed load image : %s", filenames[i].c_str());
			return nullptr;
		}

		ImageInfo& imageInfo = images[i];
		imageInfo.data = ImageLoader::LoadFromMemory(dataPtr, dataSize, &imageInfo.width, &imageInfo.height, &imageInfo.comp, 4);
		imageInfo.comp = 4;
		imageInfo.size = imageInfo.width * imageInfo.height * imageInfo.comp;

		delete[] dataPtr;
		dataPtr = nullptr;

		if (!imageInfo.data)
		{
			MLOGE("Failed load image : %s", filenames[i].c_str());
			return nullptr;
		}
	}

	// ，TextureArray
	int32_t width = images[0].width;
	int32_t height = images[0].height;
	int32_t numArray = images.size();
	VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
	int32_t mipLevels = math::FloorToInt(math::Log2(math::Max(width, height))) + 1;
	VkDevice device = vulkanDevice->GetInstanceHandle();

	uint32_t memoryTypeIndex = 0;
	VkMemoryRequirements memReqs = {};
	VkMemoryAllocateInfo memAllocInfo;
	ZeroVulkanStruct(memAllocInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);

	// stagingBuffer
	DVKBuffer* stagingBuffer = DVKBuffer::CreateBuffer(
		vulkanDevice,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		width * height * 4 * numArray
	);

	for (int32_t i = 0; i < images.size(); ++i)
	{
		uint8_t* src = images[i].data;
		uint32_t size = width * height * 4;
		stagingBuffer->Map(size, size * i);
		stagingBuffer->CopyFrom(src, size);
		stagingBuffer->UnMap();

		ImageLoader::Free(src);
	}

	// image info
	VkImage                image = VK_NULL_HANDLE;
	VkDeviceMemory         imageMemory = VK_NULL_HANDLE;
	VkImageView            imageView = VK_NULL_HANDLE;
	VkSampler              imageSampler = VK_NULL_HANDLE;
	VkDescriptorImageInfo  descriptorInfo = {};

	// image
	VkImageCreateInfo imageCreateInfo;
	ZeroVulkanStruct(imageCreateInfo, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.mipLevels = mipLevels;
	imageCreateInfo.arrayLayers = numArray;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.extent = { (uint32_t)width, (uint32_t)height, 1 };
	imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	VERIFYVULKANRESULT(vkCreateImage(device, &imageCreateInfo, VULKAN_CPU_ALLOCATOR, &image));

	// bind image buffer
	vkGetImageMemoryRequirements(device, image, &memReqs);
	vulkanDevice->GetMemoryManager().GetMemoryTypeFromProperties(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryTypeIndex);
	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = memoryTypeIndex;
	VERIFYVULKANRESULT(vkAllocateMemory(device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &imageMemory));
	VERIFYVULKANRESULT(vkBindImageMemory(device, image, imageMemory, 0));

	// start record
	cmdBuffer->Begin();

	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.levelCount = 1;
	subresourceRange.layerCount = numArray;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.baseArrayLayer = 0;

	vkutils::ImagePipelineBarrier(cmdBuffer->cmdBuffer, image, ImageLayoutBarrier::Undefined, ImageLayoutBarrier::TransferDest, subresourceRange);

	std::vector<VkBufferImageCopy> bufferCopyRegions;
	for (int32_t i = 0; i < images.size(); ++i)
	{
		VkBufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel = 0;
		bufferCopyRegion.imageSubresource.baseArrayLayer = i;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = width;
		bufferCopyRegion.imageExtent.height = height;
		bufferCopyRegion.imageExtent.depth = 1;
		bufferCopyRegion.bufferOffset = width * height * 4 * i;
		bufferCopyRegions.push_back(bufferCopyRegion);
	}

	vkCmdCopyBufferToImage(cmdBuffer->cmdBuffer, stagingBuffer->buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, bufferCopyRegions.size(), bufferCopyRegions.data());

	vkutils::ImagePipelineBarrier(cmdBuffer->cmdBuffer, image, ImageLayoutBarrier::TransferDest, ImageLayoutBarrier::TransferSource, subresourceRange);

	// Generate the mip chain
	for (uint32_t i = 1; i < mipLevels; i++)
	{
		VkImageBlit imageBlit = {};

		imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBlit.srcSubresource.layerCount = numArray;
		imageBlit.srcSubresource.mipLevel = i - 1;
		imageBlit.srcOffsets[1].x = int32_t(width >> (i - 1));
		imageBlit.srcOffsets[1].y = int32_t(height >> (i - 1));
		imageBlit.srcOffsets[1].z = 1;

		imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBlit.dstSubresource.layerCount = numArray;
		imageBlit.dstSubresource.mipLevel = i;
		imageBlit.dstOffsets[1].x = int32_t(width >> i);
		imageBlit.dstOffsets[1].y = int32_t(height >> i);
		imageBlit.dstOffsets[1].z = 1;

		VkImageSubresourceRange mipSubRange = {};
		mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		mipSubRange.baseMipLevel = i;
		mipSubRange.levelCount = 1;
		mipSubRange.layerCount = numArray;
		mipSubRange.baseArrayLayer = 0;

		vkutils::ImagePipelineBarrier(cmdBuffer->cmdBuffer, image, ImageLayoutBarrier::Undefined, ImageLayoutBarrier::TransferDest, mipSubRange);

		vkCmdBlitImage(cmdBuffer->cmdBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);

		vkutils::ImagePipelineBarrier(cmdBuffer->cmdBuffer, image, ImageLayoutBarrier::TransferDest, ImageLayoutBarrier::TransferSource, mipSubRange);
	}

	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.levelCount = mipLevels;
	subresourceRange.layerCount = numArray;
	subresourceRange.baseMipLevel = 0;

	vkutils::ImagePipelineBarrier(cmdBuffer->cmdBuffer, image, ImageLayoutBarrier::TransferSource, imageLayout, subresourceRange);

	cmdBuffer->End();
	cmdBuffer->Submit();

	delete stagingBuffer;

	VkSamplerCreateInfo samplerInfo;
	ZeroVulkanStruct(samplerInfo, VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO);
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	samplerInfo.maxAnisotropy = 1.0;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxLod = mipLevels;
	samplerInfo.minLod = 0;
	VERIFYVULKANRESULT(vkCreateSampler(device, &samplerInfo, VULKAN_CPU_ALLOCATOR, &imageSampler));

	VkImageViewCreateInfo viewInfo;
	ZeroVulkanStruct(viewInfo, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
	viewInfo.image = image;
	viewInfo.format = format;
	viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.layerCount = numArray;
	viewInfo.subresourceRange.levelCount = mipLevels;
	VERIFYVULKANRESULT(vkCreateImageView(device, &viewInfo, VULKAN_CPU_ALLOCATOR, &imageView));

	descriptorInfo.sampler = imageSampler;
	descriptorInfo.imageView = imageView;
	descriptorInfo.imageLayout = vkutils::GetImageLayout(imageLayout);

	VKTexture* texture = new VKTexture();
	texture->descriptorInfo = descriptorInfo;
	texture->format = format;
	texture->height = height;
	texture->image = image;
	texture->imageLayout = vkutils::GetImageLayout(imageLayout);
	texture->imageMemory = imageMemory;
	texture->imageSampler = imageSampler;
	texture->imageView = imageView;
	texture->device = device;
	texture->width = width;
	texture->mipLevels = mipLevels;
	texture->layerCount = numArray;

	return texture;
}

VKTexture* VKTexture::Create3D(VkFormat format, const uint8_t* rgbaData, int32_t size, int32_t width, int32_t height, int32_t depth, std::shared_ptr<VulkanDevice> vulkanDevice, VKCommandBuffer* cmdBuffer, ImageLayoutBarrier imageLayout)
{
	VkDevice device = vulkanDevice->GetInstanceHandle();

	DVKBuffer* stagingBuffer = DVKBuffer::CreateBuffer(vulkanDevice, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, size);
	stagingBuffer->Map();
	stagingBuffer->CopyFrom((void*)rgbaData, size);
	stagingBuffer->UnMap();

	uint32_t memoryTypeIndex = 0;
	VkMemoryRequirements memReqs = {};
	VkMemoryAllocateInfo memAllocInfo;
	ZeroVulkanStruct(memAllocInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);

	// image info
	VkImage                         image = VK_NULL_HANDLE;
	VkDeviceMemory                  imageMemory = VK_NULL_HANDLE;
	VkImageView                     imageView = VK_NULL_HANDLE;
	VkSampler                       imageSampler = VK_NULL_HANDLE;
	VkDescriptorImageInfo           descriptorInfo = {};

	// Create optimal tiled target image
	VkImageCreateInfo imageCreateInfo;
	ZeroVulkanStruct(imageCreateInfo, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
	imageCreateInfo.imageType = VK_IMAGE_TYPE_3D;
	imageCreateInfo.format = format;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.extent.width = width;
	imageCreateInfo.extent.height = height;
	imageCreateInfo.extent.depth = depth;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	VERIFYVULKANRESULT(vkCreateImage(device, &imageCreateInfo, VULKAN_CPU_ALLOCATOR, &image));

	// bind image buffer
	vkGetImageMemoryRequirements(device, image, &memReqs);
	vulkanDevice->GetMemoryManager().GetMemoryTypeFromProperties(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryTypeIndex);
	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = memoryTypeIndex;
	VERIFYVULKANRESULT(vkAllocateMemory(device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &imageMemory));
	VERIFYVULKANRESULT(vkBindImageMemory(device, image, imageMemory, 0));

	cmdBuffer->Begin();

	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.levelCount = 1;
	subresourceRange.layerCount = 1;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.baseArrayLayer = 0;

	vkutils::ImagePipelineBarrier(cmdBuffer->cmdBuffer, image, ImageLayoutBarrier::Undefined, ImageLayoutBarrier::TransferDest, subresourceRange);

	VkBufferImageCopy bufferCopyRegion = {};
	bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	bufferCopyRegion.imageSubresource.mipLevel = 0;
	bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
	bufferCopyRegion.imageSubresource.layerCount = 1;
	bufferCopyRegion.imageExtent.width = width;
	bufferCopyRegion.imageExtent.height = height;
	bufferCopyRegion.imageExtent.depth = depth;

	vkCmdCopyBufferToImage(cmdBuffer->cmdBuffer, stagingBuffer->buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferCopyRegion);

	vkutils::ImagePipelineBarrier(cmdBuffer->cmdBuffer, image, ImageLayoutBarrier::TransferDest, imageLayout, subresourceRange);

	cmdBuffer->End();
	cmdBuffer->Submit();

	delete stagingBuffer;

	// Create sampler
	VkSamplerCreateInfo samplerInfo;
	ZeroVulkanStruct(samplerInfo, VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO);
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 1.0f;
	samplerInfo.maxAnisotropy = 1.0;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	VERIFYVULKANRESULT(vkCreateSampler(device, &samplerInfo, VULKAN_CPU_ALLOCATOR, &imageSampler));

	// Create image view
	VkImageViewCreateInfo viewInfo;
	ZeroVulkanStruct(viewInfo, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
	viewInfo.format = format;
	viewInfo.components = {
		VK_COMPONENT_SWIZZLE_R,
		VK_COMPONENT_SWIZZLE_G,
		VK_COMPONENT_SWIZZLE_B,
		VK_COMPONENT_SWIZZLE_A
	};
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;
	viewInfo.subresourceRange.levelCount = 1;
	VERIFYVULKANRESULT(vkCreateImageView(device, &viewInfo, VULKAN_CPU_ALLOCATOR, &imageView));

	descriptorInfo.sampler = imageSampler;
	descriptorInfo.imageView = imageView;
	descriptorInfo.imageLayout = vkutils::GetImageLayout(imageLayout);

	VKTexture* texture = new VKTexture();
	texture->descriptorInfo = descriptorInfo;
	texture->format = format;
	texture->width = width;
	texture->height = height;
	texture->depth = depth;
	texture->image = image;
	texture->imageLayout = vkutils::GetImageLayout(imageLayout);
	texture->imageMemory = imageMemory;
	texture->imageSampler = imageSampler;
	texture->imageView = imageView;
	texture->device = device;
	texture->mipLevels = 1;
	texture->layerCount = 1;

	return texture;
}