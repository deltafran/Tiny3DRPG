#include "stdafx.h"
#include "DefaultVulkanContext.h"
#include "VulkanRHI.h"
#include "VulkanSwapChain.h"
#include "VulkanDevice.h"
#include "VulkanMemory.h"
#include "VKCommandBuffer.h"
#include "VKDefaultRes.h"

void DefaultVulkanContext::Present(int backBufferIndex)
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

int32_t DefaultVulkanContext::AcquireBackbufferIndex()
{
	int32_t backBufferIndex = m_SwapChain->AcquireImageIndex(&m_PresentComplete);
	return backBufferIndex;
}

uint32_t DefaultVulkanContext::GetMemoryTypeFromProperties(uint32_t typeBits, VkMemoryPropertyFlags properties)
{
	uint32_t memoryTypeIndex = 0;
	m_vulkanRHI.GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(typeBits, properties, &memoryTypeIndex);
	return memoryTypeIndex;
}

void DefaultVulkanContext::setup()
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

void DefaultVulkanContext::createFences()
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

void DefaultVulkanContext::createCommandBuffers()
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

void DefaultVulkanContext::createPipelineCache()
{
	VkDevice device = m_vulkanRHI.GetDevice()->GetInstanceHandle();

	VkPipelineCacheCreateInfo createInfo;
	ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO);
	VERIFYVULKANRESULT(vkCreatePipelineCache(device, &createInfo, VULKAN_CPU_ALLOCATOR, &m_PipelineCache));
}

void DefaultVulkanContext::createDefaultRes()
{
	VKCommandBuffer* cmdbuffer = VKCommandBuffer::Create(m_vulkanRHI.GetDevice(), m_CommandPool);
	VKDefaultRes::Init(m_vulkanRHI.GetDevice(), cmdbuffer);
	delete cmdbuffer;
}

void DefaultVulkanContext::destroyFences()
{
	VkDevice device = m_vulkanRHI.GetDevice()->GetInstanceHandle();

	for (int32_t i = 0; i < m_Fences.size(); ++i)
		vkDestroyFence(device, m_Fences[i], VULKAN_CPU_ALLOCATOR);

	vkDestroySemaphore(device, m_RenderComplete, VULKAN_CPU_ALLOCATOR);
}

void DefaultVulkanContext::destroyCommandBuffers()
{
	VkDevice device = m_vulkanRHI.GetDevice()->GetInstanceHandle();
	for (int32_t i = 0; i < m_CommandBuffers.size(); ++i)
		vkFreeCommandBuffers(device, m_CommandPool, 1, &(m_CommandBuffers[i]));

	vkDestroyCommandPool(device, m_CommandPool, VULKAN_CPU_ALLOCATOR);
	vkDestroyCommandPool(device, m_ComputeCommandPool, VULKAN_CPU_ALLOCATOR);
}

void DefaultVulkanContext::destroyPipelineCache()
{
	VkDevice device = m_vulkanRHI.GetDevice()->GetInstanceHandle();
	vkDestroyPipelineCache(device, m_PipelineCache, VULKAN_CPU_ALLOCATOR);
	m_PipelineCache = VK_NULL_HANDLE;
}

void DefaultVulkanContext::destroyDefaultRes()
{
	VKDefaultRes::Destroy();
}