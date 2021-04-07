#include "stdafx.h"
#include "VulkanContext.h"
#include "VulkanGlobals.h"
#include "VulkanRHI.h"
#include "VulkanSwapChain.h"
#include "VulkanDevice.h"
#include "VulkanMemory.h"
#include "VKCommandBuffer.h"
#include "VKDefaultRes.h"

void VulkanContext::createDepthStencil() noexcept
{
	destroyDepthStencil();

	int32_t fwidth  = m_vulkanRHI.GetSwapChain()->GetWidth();
	int32_t fheight = m_vulkanRHI.GetSwapChain()->GetHeight();
	VkDevice device = m_vulkanRHI.GetDevice()->GetInstanceHandle();

	VkImageCreateInfo imageCreateInfo;
	ZeroVulkanStruct(imageCreateInfo, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
	imageCreateInfo.imageType   = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format      = PixelFormatToVkFormat(m_DepthFormat, false);
	imageCreateInfo.extent      = { (uint32_t)fwidth, (uint32_t)fheight, 1 };
	imageCreateInfo.mipLevels   = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples     = m_SampleCount;
	imageCreateInfo.tiling      = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage       = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	imageCreateInfo.flags       = 0;
	VERIFYVULKANRESULT(vkCreateImage(device, &imageCreateInfo, VULKAN_CPU_ALLOCATOR, &m_DepthStencilImage));

	VkMemoryRequirements memRequire;
	vkGetImageMemoryRequirements(device, m_DepthStencilImage, &memRequire);
	uint32_t memoryTypeIndex = 0;
	VERIFYVULKANRESULT(m_vulkanRHI.GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memRequire.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryTypeIndex));

	VkMemoryAllocateInfo memAllocateInfo;
	ZeroVulkanStruct(memAllocateInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);
	memAllocateInfo.allocationSize  = memRequire.size;
	memAllocateInfo.memoryTypeIndex = memoryTypeIndex;
	vkAllocateMemory(device, &memAllocateInfo, VULKAN_CPU_ALLOCATOR, &m_DepthStencilMemory);
	vkBindImageMemory(device, m_DepthStencilImage, m_DepthStencilMemory, 0);

	VkImageViewCreateInfo imageViewCreateInfo;
	ZeroVulkanStruct(imageViewCreateInfo, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
	imageViewCreateInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format                          = PixelFormatToVkFormat(m_DepthFormat, false);
	imageViewCreateInfo.flags                           = 0;
	imageViewCreateInfo.image                           = m_DepthStencilImage;
	imageViewCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	imageViewCreateInfo.subresourceRange.baseMipLevel   = 0;
	imageViewCreateInfo.subresourceRange.levelCount     = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount     = 1;
	VERIFYVULKANRESULT(vkCreateImageView(device, &imageViewCreateInfo, VULKAN_CPU_ALLOCATOR, &m_DepthStencilView));	
}

void VulkanContext::createRenderPass() noexcept
{
	destroyRenderPass();

	VkDevice device = m_vulkanRHI.GetDevice()->GetInstanceHandle();
	const PixelFormat pixelFormat = m_vulkanRHI.GetPixelFormat();

	std::vector<VkAttachmentDescription> attachments(2);
	// color attachment
	attachments[0].format         = PixelFormatToVkFormat(pixelFormat, false);
	attachments[0].samples        = m_SampleCount;
	attachments[0].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	// depth stencil attachment
	attachments[1].format         = PixelFormatToVkFormat(m_DepthFormat, false);
	attachments[1].samples        = m_SampleCount;
	attachments[1].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorReference = { };
	colorReference.attachment = 0;
	colorReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthReference = { };
	depthReference.attachment = 1;
	depthReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription = { };
	subpassDescription.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount    = 1;
	subpassDescription.pColorAttachments       = &colorReference;
	subpassDescription.pDepthStencilAttachment = &depthReference;
	subpassDescription.pResolveAttachments     = nullptr;
	subpassDescription.inputAttachmentCount    = 0;
	subpassDescription.pInputAttachments       = nullptr;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments    = nullptr;

	std::vector<VkSubpassDependency> dependencies(2);
	dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass      = 0;
	dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass      = 0;
	dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassInfo;
	ZeroVulkanStruct(renderPassInfo, VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO);
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments    = attachments.data();
	renderPassInfo.subpassCount    = 1;
	renderPassInfo.pSubpasses      = &subpassDescription;
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies   = dependencies.data();
	VERIFYVULKANRESULT(vkCreateRenderPass(device, &renderPassInfo, VULKAN_CPU_ALLOCATOR, &m_RenderPass));
}

void VulkanContext::createFrameBuffers() noexcept
{
	destroyFrameBuffers();

	int32_t fwidth  = m_vulkanRHI.GetSwapChain()->GetWidth();
	int32_t fheight = m_vulkanRHI.GetSwapChain()->GetHeight();
	VkDevice device = m_vulkanRHI.GetDevice()->GetInstanceHandle();

	VkImageView attachments[2];
	attachments[1] = m_DepthStencilView;

	VkFramebufferCreateInfo frameBufferCreateInfo;
	ZeroVulkanStruct(frameBufferCreateInfo, VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO);
	frameBufferCreateInfo.renderPass      = m_RenderPass;
	frameBufferCreateInfo.attachmentCount = 2;
	frameBufferCreateInfo.pAttachments    = attachments;
	frameBufferCreateInfo.width           = fwidth;
	frameBufferCreateInfo.height          = fheight;
	frameBufferCreateInfo.layers          = 1;

	const std::vector<VkImageView>& backbufferViews = m_vulkanRHI.GetBackbufferViews();

	m_FrameBuffers.resize(backbufferViews.size());
	for (uint32_t i = 0; i < m_FrameBuffers.size(); ++i)
	{
		attachments[0] = backbufferViews[i];
		VERIFYVULKANRESULT(vkCreateFramebuffer(device, &frameBufferCreateInfo, VULKAN_CPU_ALLOCATOR, &m_FrameBuffers[i]));
	}
}

void VulkanContext::destroyDepthStencil() noexcept
{
	VkDevice device = m_vulkanRHI.GetDevice()->GetInstanceHandle();

	if (m_DepthStencilMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory(device, m_DepthStencilMemory, VULKAN_CPU_ALLOCATOR);
		m_DepthStencilMemory = VK_NULL_HANDLE;
	}

	if (m_DepthStencilView != VK_NULL_HANDLE)
	{
		vkDestroyImageView(device, m_DepthStencilView, VULKAN_CPU_ALLOCATOR);
		m_DepthStencilView = VK_NULL_HANDLE;
	}

	if (m_DepthStencilImage != VK_NULL_HANDLE)
	{
		vkDestroyImage(device, m_DepthStencilImage, VULKAN_CPU_ALLOCATOR);
		m_DepthStencilImage = VK_NULL_HANDLE;
	}
}

void VulkanContext::destroyRenderPass() noexcept
{
	VkDevice device = m_vulkanRHI.GetDevice()->GetInstanceHandle();
	if (m_RenderPass != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass(device, m_RenderPass, VULKAN_CPU_ALLOCATOR);
		m_RenderPass = VK_NULL_HANDLE;
	}
}

void VulkanContext::destroyFrameBuffers() noexcept
{
	VkDevice device = m_vulkanRHI.GetDevice()->GetInstanceHandle();
	for (int32_t i = 0; i < m_FrameBuffers.size(); ++i)
		vkDestroyFramebuffer(device, m_FrameBuffers[i], VULKAN_CPU_ALLOCATOR);

	m_FrameBuffers.clear();
}

void VulkanContext::Present(int backBufferIndex) noexcept
{
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pWaitDstStageMask = &m_WaitStageMask;
	submitInfo.pWaitSemaphores = &m_PresentComplete;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &m_RenderComplete;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pCommandBuffers = &(m_CommandBuffers[backBufferIndex]);
	submitInfo.commandBufferCount = 1;

	vkResetFences(m_Device, 1, &(m_Fences[backBufferIndex]));

	VERIFYVULKANRESULT(vkQueueSubmit(m_GfxQueue, 1, &submitInfo, m_Fences[backBufferIndex]));
	vkWaitForFences(m_Device, 1, &(m_Fences[backBufferIndex]), true, UINT64_MAX);

	// present
	m_SwapChain->Present(m_VulkanDevice->GetGraphicsQueue(), m_VulkanDevice->GetPresentQueue(), &m_RenderComplete);
}

int32_t VulkanContext::AcquireBackbufferIndex() noexcept
{
	int32_t backBufferIndex = m_SwapChain->AcquireImageIndex(&m_PresentComplete);
	return backBufferIndex;
}

uint32_t VulkanContext::GetMemoryTypeFromProperties(uint32_t typeBits, VkMemoryPropertyFlags properties) noexcept
{
	uint32_t memoryTypeIndex = 0;
	m_vulkanRHI.GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(typeBits, properties, &memoryTypeIndex);
	return memoryTypeIndex;
}

void VulkanContext::setup() noexcept
{
	auto vulkanDevice = m_vulkanRHI.GetDevice();

	m_SwapChain = m_vulkanRHI.GetSwapChain();

	m_Device = vulkanDevice->GetInstanceHandle();
	m_VulkanDevice = vulkanDevice;
	m_GfxQueue = vulkanDevice->GetGraphicsQueue()->GetHandle();
	m_PresentQueue = vulkanDevice->GetPresentQueue()->GetHandle();

	m_FrameWidth = m_vulkanRHI.GetSwapChain()->GetWidth();
	m_FrameHeight = m_vulkanRHI.GetSwapChain()->GetHeight();
}

void VulkanContext::createFences() noexcept
{
	VkDevice device = m_vulkanRHI.GetDevice()->GetInstanceHandle();
	int32_t frameCount = m_vulkanRHI.GetSwapChain()->GetBackBufferCount();

	VkFenceCreateInfo fenceCreateInfo;
	ZeroVulkanStruct(fenceCreateInfo, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	m_Fences.resize(frameCount);
	for (int32_t i = 0; i < m_Fences.size(); ++i)
	{
		VERIFYVULKANRESULT(vkCreateFence(device, &fenceCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Fences[i]));
	}

	VkSemaphoreCreateInfo createInfo;
	ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);
	vkCreateSemaphore(device, &createInfo, VULKAN_CPU_ALLOCATOR, &m_RenderComplete);
}

void VulkanContext::createCommandBuffers() noexcept
{
	VkDevice device = m_vulkanRHI.GetDevice()->GetInstanceHandle();

	VkCommandPoolCreateInfo cmdPoolInfo;
	ZeroVulkanStruct(cmdPoolInfo, VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO);
	cmdPoolInfo.queueFamilyIndex = m_vulkanRHI.GetDevice()->GetPresentQueue()->GetFamilyIndex();
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VERIFYVULKANRESULT(vkCreateCommandPool(device, &cmdPoolInfo, VULKAN_CPU_ALLOCATOR, &m_CommandPool));

	VkCommandPoolCreateInfo computePoolInfo;
	ZeroVulkanStruct(computePoolInfo, VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO);
	computePoolInfo.queueFamilyIndex = m_vulkanRHI.GetDevice()->GetComputeQueue()->GetFamilyIndex();
	computePoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VERIFYVULKANRESULT(vkCreateCommandPool(device, &computePoolInfo, VULKAN_CPU_ALLOCATOR, &m_ComputeCommandPool));

	VkCommandBufferAllocateInfo cmdBufferInfo;
	ZeroVulkanStruct(cmdBufferInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);
	cmdBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferInfo.commandBufferCount = 1;
	cmdBufferInfo.commandPool = m_CommandPool;

	m_CommandBuffers.resize(m_vulkanRHI.GetSwapChain()->GetBackBufferCount());
	for (int32_t i = 0; i < m_CommandBuffers.size(); ++i)
		vkAllocateCommandBuffers(device, &cmdBufferInfo, &(m_CommandBuffers[i]));
}

void VulkanContext::createPipelineCache() noexcept
{
	VkDevice device = m_vulkanRHI.GetDevice()->GetInstanceHandle();

	VkPipelineCacheCreateInfo createInfo;
	ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO);
	VERIFYVULKANRESULT(vkCreatePipelineCache(device, &createInfo, VULKAN_CPU_ALLOCATOR, &m_PipelineCache));
}

void VulkanContext::createDefaultRes() noexcept
{
	VKCommandBuffer* cmdbuffer = VKCommandBuffer::Create(m_vulkanRHI.GetDevice(), m_CommandPool);
	VKDefaultRes::Init(m_vulkanRHI.GetDevice(), cmdbuffer);
	delete cmdbuffer;
}

void VulkanContext::destroyFences() noexcept
{
	VkDevice device = m_vulkanRHI.GetDevice()->GetInstanceHandle();

	for (int32_t i = 0; i < m_Fences.size(); ++i)
		vkDestroyFence(device, m_Fences[i], VULKAN_CPU_ALLOCATOR);

	vkDestroySemaphore(device, m_RenderComplete, VULKAN_CPU_ALLOCATOR);
}

void VulkanContext::destroyCommandBuffers() noexcept
{
	VkDevice device = m_vulkanRHI.GetDevice()->GetInstanceHandle();
	for (int32_t i = 0; i < m_CommandBuffers.size(); ++i)
		vkFreeCommandBuffers(device, m_CommandPool, 1, &(m_CommandBuffers[i]));

	vkDestroyCommandPool(device, m_CommandPool, VULKAN_CPU_ALLOCATOR);
	vkDestroyCommandPool(device, m_ComputeCommandPool, VULKAN_CPU_ALLOCATOR);
}

void VulkanContext::destroyPipelineCache() noexcept
{
	VkDevice device = m_vulkanRHI.GetDevice()->GetInstanceHandle();
	vkDestroyPipelineCache(device, m_PipelineCache, VULKAN_CPU_ALLOCATOR);
	m_PipelineCache = VK_NULL_HANDLE;
}

void VulkanContext::destroyDefaultRes() noexcept
{
	VKDefaultRes::Destroy();
}