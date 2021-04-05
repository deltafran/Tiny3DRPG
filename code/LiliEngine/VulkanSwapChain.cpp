#include "stdafx.h"
#include "VulkanSwapChain.h"
#include "VulkanDevice.h"
#include "VulkanGlobals.h"
#include "CoreMath2.h"

void CreateVKSurfaceFromWindows(const WindowInfo& windowInfo, VkInstance instance, VkSurfaceKHR* outSurface); // ==> VulkanPlatform.cpp

VulkanSwapChain::VulkanSwapChain(const WindowInfo& windowInfo, VkInstance instance, std::shared_ptr<VulkanDevice> device, PixelFormat& outPixelFormat, uint32_t width, uint32_t height, uint32_t* outDesiredNumBackBuffers, std::vector<VkImage>& outImages, int8_t lockToVsync)
	: m_Instance(instance)
	, m_SwapChain(VK_NULL_HANDLE)
	, m_Surface(VK_NULL_HANDLE)
	, m_ColorFormat(VK_FORMAT_R8G8B8A8_UNORM)
	, m_BackBufferCount(3)
	, m_Device(device)
	, m_CurrentImageIndex(-1)
	, m_SemaphoreIndex(0)
	, m_NumPresentCalls(0)
	, m_NumAcquireCalls(0)
	, m_LockToVsync(lockToVsync)
	, m_PresentID(0)
{

	// Surface
	CreateVKSurfaceFromWindows(windowInfo, instance, &m_Surface);

	// Present Queue
	m_Device->SetupPresentQueue(m_Surface);

	// Format
	uint32_t numFormats;
	VERIFYVULKANRESULT_EXPANDED(vkGetPhysicalDeviceSurfaceFormatsKHR(m_Device->GetPhysicalHandle(), m_Surface, &numFormats, nullptr));

	std::vector<VkSurfaceFormatKHR> formats(numFormats);
	VERIFYVULKANRESULT_EXPANDED(vkGetPhysicalDeviceSurfaceFormatsKHR(m_Device->GetPhysicalHandle(), m_Surface, &numFormats, formats.data()));

	VkSurfaceFormatKHR currFormat = {};
	if (outPixelFormat != PF_Unknown)
	{
		bool bFound = false;
		if (G_PixelFormats[outPixelFormat].supported)
		{
			VkFormat requested = (VkFormat)G_PixelFormats[outPixelFormat].platformFormat;
			for (int32_t index = 0; index < formats.size(); ++index)
			{
				if (formats[index].format == requested)
				{
					bFound = true;
					currFormat = formats[index];
					break;
				}
			}

			if (!bFound)
			{
				MLOG("Requested PixelFormat %d not supported by this swapchain! Falling back to supported swapchain format...", (uint32_t)outPixelFormat);
				outPixelFormat = PF_Unknown;
			}
		}
		else
		{
			MLOG("Requested PixelFormat %d not supported by this Vulkan implementation!", (uint32_t)outPixelFormat);
			outPixelFormat = PF_Unknown;
		}
	}

	if (outPixelFormat == PF_Unknown)
	{
		for (int32_t index = 0; index < formats.size(); ++index)
		{
			for (int32_t pfIndex = 0; pfIndex < PF_MAX; ++pfIndex)
			{
				if (formats[index].format == G_PixelFormats[pfIndex].platformFormat)
				{
					outPixelFormat = (PixelFormat)pfIndex;
					currFormat = formats[index];
					MLOG("No swapchain format requested, picking up VulkanFormat %d", (uint32_t)currFormat.format);
					break;
				}
			}

			if (outPixelFormat != PF_Unknown)
			{
				break;
			}
		}
	}

	if (outPixelFormat == PF_Unknown)
	{
		MLOG("Can't find a proper pixel format for the swapchain, trying to pick up the first available");
		VkFormat platformFormat = PixelFormatToVkFormat(outPixelFormat, false);
		bool supported = false;
		for (int32_t index = 0; index < formats.size(); ++index)
		{
			if (formats[index].format == platformFormat)
			{
				supported = true;
				currFormat = formats[index];
			}
		}
	}

	// Present Model
	uint32_t numFoundPresentModes = 0;
	VERIFYVULKANRESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(m_Device->GetPhysicalHandle(), m_Surface, &numFoundPresentModes, nullptr));

	std::vector< VkPresentModeKHR> foundPresentModes(numFoundPresentModes);
	VERIFYVULKANRESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(m_Device->GetPhysicalHandle(), m_Surface, &numFoundPresentModes, foundPresentModes.data()));

	bool foundPresentModeMailbox = false;
	bool foundPresentModeImmediate = false;
	bool foundPresentModeFIFO = false;

	MLOG("Found %d present mode.", numFoundPresentModes);
	for (int32_t index = 0; index < numFoundPresentModes; ++index)
	{
		switch (foundPresentModes[index])
		{
		case VK_PRESENT_MODE_MAILBOX_KHR:
			foundPresentModeMailbox = true;
			MLOG("- VK_PRESENT_MODE_MAILBOX_KHR (%d)", (int32_t)VK_PRESENT_MODE_MAILBOX_KHR);
			break;
		case VK_PRESENT_MODE_IMMEDIATE_KHR:
			foundPresentModeImmediate = true;
			MLOG("- VK_PRESENT_MODE_IMMEDIATE_KHR (%d)", (int32_t)VK_PRESENT_MODE_IMMEDIATE_KHR);
			break;
		case VK_PRESENT_MODE_FIFO_KHR:
			foundPresentModeFIFO = true;
			MLOG("- VK_PRESENT_MODE_FIFO_KHR (%d)", (int32_t)VK_PRESENT_MODE_FIFO_KHR);
			break;
		default:
			MLOG("- VkPresentModeKHR (%d)", (int32_t)foundPresentModes[index]);
			break;
		}
	}

	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
	if (foundPresentModeImmediate && !m_LockToVsync) {
		presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	}
	else if (foundPresentModeMailbox) {
		presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
	}
	else if (foundPresentModeFIFO) {
		presentMode = VK_PRESENT_MODE_FIFO_KHR;
	}
	else
	{
		MLOG("Couldn't find desired PresentMode! Using %d", (int32_t)foundPresentModes[0]);
		presentMode = foundPresentModes[0];
	}

	MLOG("Selected VkPresentModeKHR mode %d", presentMode);

	VkSurfaceCapabilitiesKHR surfProperties;
	VERIFYVULKANRESULT_EXPANDED(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_Device->GetPhysicalHandle(), m_Surface, &surfProperties));

	MLOG("Surface minSize:%dx%d maxSize:%dx%d",
		(int32_t)surfProperties.minImageExtent.width, (int32_t)surfProperties.minImageExtent.height,
		(int32_t)surfProperties.maxImageExtent.width, (int32_t)surfProperties.maxImageExtent.height
	);

	VkSurfaceTransformFlagBitsKHR preTransform;
	if (surfProperties.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else {
		preTransform = surfProperties.currentTransform;
	}

	VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
	if (surfProperties.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
		compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	uint32_t desiredNumBuffers = surfProperties.maxImageCount > 0 ? math::Clamp(*outDesiredNumBackBuffers, surfProperties.minImageCount, surfProperties.maxImageCount) : *outDesiredNumBackBuffers;
	uint32_t sizeX = surfProperties.currentExtent.width == 0xFFFFFFFF ? width : surfProperties.currentExtent.width;
	uint32_t sizeY = surfProperties.currentExtent.height == 0xFFFFFFFF ? height : surfProperties.currentExtent.height;

	ZeroVulkanStruct(m_SwapChainInfo, VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR);
	m_SwapChainInfo.surface = m_Surface;
	m_SwapChainInfo.minImageCount = desiredNumBuffers;
	m_SwapChainInfo.imageFormat = currFormat.format;
	m_SwapChainInfo.imageColorSpace = currFormat.colorSpace;
	m_SwapChainInfo.imageExtent.width = sizeX;
	m_SwapChainInfo.imageExtent.height = sizeY;
	m_SwapChainInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	m_SwapChainInfo.preTransform = preTransform;
	m_SwapChainInfo.imageArrayLayers = 1;
	m_SwapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	m_SwapChainInfo.presentMode = presentMode;
	m_SwapChainInfo.oldSwapchain = VK_NULL_HANDLE;
	m_SwapChainInfo.clipped = VK_TRUE;
	m_SwapChainInfo.compositeAlpha = compositeAlpha;

	if (m_SwapChainInfo.imageExtent.width == 0)
		m_SwapChainInfo.imageExtent.width = width;
	if (m_SwapChainInfo.imageExtent.height == 0)
		m_SwapChainInfo.imageExtent.height = height;

	// present
	VkBool32 supportsPresent;
	VERIFYVULKANRESULT(vkGetPhysicalDeviceSurfaceSupportKHR(m_Device->GetPhysicalHandle(), m_Device->GetPresentQueue()->GetFamilyIndex(), m_Surface, &supportsPresent));
	if (!supportsPresent)
		MLOGE("Present queue not support.");

	// SwapChain
	VERIFYVULKANRESULT(vkCreateSwapchainKHR(m_Device->GetInstanceHandle(), &m_SwapChainInfo, VULKAN_CPU_ALLOCATOR, &m_SwapChain));

	// Backbuffer
	uint32_t numSwapChainImages;
	VERIFYVULKANRESULT(vkGetSwapchainImagesKHR(m_Device->GetInstanceHandle(), m_SwapChain, &numSwapChainImages, nullptr));

	outImages.resize(numSwapChainImages);
	VERIFYVULKANRESULT(vkGetSwapchainImagesKHR(m_Device->GetInstanceHandle(), m_SwapChain, &numSwapChainImages, outImages.data()));

	*outDesiredNumBackBuffers = numSwapChainImages;
	m_BackBufferCount = numSwapChainImages;

	// Fence
	m_ImageAcquiredSemaphore.resize(numSwapChainImages);
	for (int32_t index = 0; index < numSwapChainImages; ++index)
	{
		VkSemaphoreCreateInfo createInfo;
		ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);
		VERIFYVULKANRESULT(vkCreateSemaphore(m_Device->GetInstanceHandle(), &createInfo, VULKAN_CPU_ALLOCATOR, &m_ImageAcquiredSemaphore[index]));
	}

	m_PresentID = 0;
	m_ColorFormat = currFormat.format;
	MLOG("SwapChain: Backbuffer:%d Format:%d ColorSpace:%d Size:%dx%d Present:%d", m_SwapChainInfo.minImageCount, m_SwapChainInfo.imageFormat, m_SwapChainInfo.imageColorSpace, m_SwapChainInfo.imageExtent.width, m_SwapChainInfo.imageExtent.height, m_SwapChainInfo.presentMode);
}

VulkanSwapChain::~VulkanSwapChain()
{
	VkDevice device = m_Device->GetInstanceHandle();

	for (int32_t index = 0; index < m_ImageAcquiredSemaphore.size(); ++index) {
		vkDestroySemaphore(m_Device->GetInstanceHandle(), m_ImageAcquiredSemaphore[index], VULKAN_CPU_ALLOCATOR);
	}

	vkDestroySwapchainKHR(device, m_SwapChain, VULKAN_CPU_ALLOCATOR);
	vkDestroySurfaceKHR(m_Instance, m_Surface, VULKAN_CPU_ALLOCATOR);
}

int32_t VulkanSwapChain::AcquireImageIndex(VkSemaphore* outSemaphore)
{
	uint32_t imageIndex = 0;
	VkDevice device = m_Device->GetInstanceHandle();
	const int32_t prev = m_SemaphoreIndex;

	m_SemaphoreIndex = (m_SemaphoreIndex + 1) % m_ImageAcquiredSemaphore.size();
	VkResult result = vkAcquireNextImageKHR(device, m_SwapChain, UINT64_MAX, m_ImageAcquiredSemaphore[m_SemaphoreIndex], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		m_SemaphoreIndex = prev;
		return (int32_t)SwapStatus::OutOfDate;
	}

	if (result == VK_ERROR_SURFACE_LOST_KHR) {
		m_SemaphoreIndex = prev;
		return (int32_t)SwapStatus::SurfaceLost;
	}

	m_NumAcquireCalls += 1;
	*outSemaphore = m_ImageAcquiredSemaphore[m_SemaphoreIndex];
	m_CurrentImageIndex = (int32_t)imageIndex;

	return m_CurrentImageIndex;
}

VulkanSwapChain::SwapStatus VulkanSwapChain::Present(std::shared_ptr<VulkanQueue> gfxQueue, std::shared_ptr<VulkanQueue> presentQueue, VkSemaphore* doneSemaphore)
{
	if (m_CurrentImageIndex == -1) {
		return SwapStatus::Healthy;
	}

	m_PresentID += 1;

	VkPresentInfoKHR createInfo;
	ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_PRESENT_INFO_KHR);
	createInfo.waitSemaphoreCount = doneSemaphore == nullptr ? 0 : 1;
	createInfo.pWaitSemaphores = doneSemaphore;
	createInfo.swapchainCount = 1;
	createInfo.pSwapchains = &m_SwapChain;
	createInfo.pImageIndices = (uint32_t*)&m_CurrentImageIndex;

	VkResult presentResult = vkQueuePresentKHR(presentQueue->GetHandle(), &createInfo);

	if (presentResult == VK_ERROR_OUT_OF_DATE_KHR) {
		return SwapStatus::OutOfDate;
	}

	if (presentResult == VK_ERROR_SURFACE_LOST_KHR) {
		return SwapStatus::SurfaceLost;
	}

	if (presentResult != VK_SUCCESS && presentResult != VK_SUBOPTIMAL_KHR) {
		VERIFYVULKANRESULT(presentResult);
	}

	m_NumPresentCalls += 1;

	return SwapStatus::Healthy;
}

void VulkanDevice::SetupPresentQueue(VkSurfaceKHR surface)
{
	if (m_presentQueue) {
		return;
	}

	const auto SupportsPresent = [surface](VkPhysicalDevice physicalDevice, std::shared_ptr<VulkanQueue> queue)
	{
		VkBool32 supportsPresent = VK_FALSE;
		const uint32_t familyIndex = queue->GetFamilyIndex();
		VERIFYVULKANRESULT(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, familyIndex, surface, &supportsPresent));
		if (supportsPresent) {
			MLOG("Queue Family %d: Supports Present", familyIndex);
		}
		return (supportsPresent == VK_TRUE);
	};

	bool compute = SupportsPresent(m_physicalDevice, m_computeQueue);
	if (m_transferQueue->GetFamilyIndex() != m_gfxQueue->GetFamilyIndex() && m_transferQueue->GetFamilyIndex() != m_computeQueue->GetFamilyIndex()) {
		SupportsPresent(m_physicalDevice, m_transferQueue);
	}
	if (m_computeQueue->GetFamilyIndex() != m_gfxQueue->GetFamilyIndex() && compute) {
		m_presentQueue = m_computeQueue;
	}
	else {
		m_presentQueue = m_gfxQueue;
	}
}