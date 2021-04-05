#include "stdafx.h"
#include "VulkanRHI.h"
#include "Log.h"
#include "VulkanGlobals.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"

constexpr auto VulkanEngineName = "LiliEngine";

bool VulkanRHI::Init(const WindowInfo& windowInfo, int32_t width, int32_t height) noexcept
{
	m_windowInfo = &windowInfo;

	if (!createInstance())
		return false;
#ifdef _DEBUG
	setupDebugLayerCallback();
#endif
	if (!selectAndInitDevice())
		return false;

	if (!recreateSwapChain(width, height))
		return false;
	return true;
}

void VulkanRHI::Close() noexcept
{
	destorySwapChain();
#ifdef _DEBUG
	removeDebugLayerCallback();
#endif
	m_device->Destroy();
	m_device = nullptr;
	vkDestroyInstance(m_instance, VULKAN_CPU_ALLOCATOR);
}

bool VulkanRHI::createInstance() noexcept
{
	getInstanceLayersAndExtensions();
	if (m_appInstanceExtensions.size() > 0)
	{
		Log::Message("Using app instance extensions");
		for (size_t i = 0; i < m_appInstanceExtensions.size(); ++i)
		{
			m_instanceExtensions.push_back(m_appInstanceExtensions[i]);
			Log::Message("*" + std::string(m_appInstanceExtensions[i]));
		}
	}

	VkApplicationInfo appInfo;
	ZeroVulkanStruct(appInfo, VK_STRUCTURE_TYPE_APPLICATION_INFO);
	appInfo.pApplicationName   = VulkanEngineName;
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
	appInfo.pEngineName        = VulkanEngineName;
	appInfo.engineVersion      = VK_MAKE_VERSION(0, 0, 0);
	appInfo.apiVersion         = VK_API_VERSION_1_2;

	VkInstanceCreateInfo instanceCreateInfo;
	ZeroVulkanStruct(instanceCreateInfo, VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO);
	instanceCreateInfo.pApplicationInfo        = &appInfo;
	instanceCreateInfo.enabledExtensionCount   = uint32_t(m_instanceExtensions.size());
	instanceCreateInfo.ppEnabledExtensionNames = m_instanceExtensions.size() > 0 ? m_instanceExtensions.data() : nullptr;
	instanceCreateInfo.enabledLayerCount       = uint32_t(m_instanceLayers.size());
	instanceCreateInfo.ppEnabledLayerNames     = m_instanceLayers.size() > 0 ? m_instanceLayers.data() : nullptr;

	VkResult result = vkCreateInstance(&instanceCreateInfo, VULKAN_CPU_ALLOCATOR, &m_instance);
	if (result == VK_ERROR_INCOMPATIBLE_DRIVER)
	{
		Log::Error("Cannot find a compatible Vulkan driver (ICD).");
		return false;
	}
	else if (result == VK_ERROR_EXTENSION_NOT_PRESENT)
	{
		std::string missingExtensions;
		uint32_t propertyCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, nullptr);
		std::vector<VkExtensionProperties> properties(propertyCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, properties.data());

		for (const char* extension : m_instanceExtensions)
		{
			bool found = false;
			for (uint32_t i = 0; i < propertyCount; ++i)
			{
				const char* propExtension = properties[i].extensionName;
				if (strcmp(propExtension, extension) == 0)
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				std::string extensionStr(extension);
				missingExtensions += extensionStr + "\n";
			}
		}

		Log::Error("Vulkan driver doesn't contain specified extensions:\n" + missingExtensions);
		return false;
	}
	else if (result != VK_SUCCESS)
	{
		Log::Error("Vulkan failed to create instance.");
		return false;
	}

	Log::Message("Vulkan successed to create instance.");
	return true;
}

bool VulkanRHI::selectAndInitDevice() noexcept
{
	uint32_t gpuCount = 0;
	VkResult result = vkEnumeratePhysicalDevices(m_instance, &gpuCount, nullptr);

	if (result == VK_ERROR_INITIALIZATION_FAILED) 
	{
		Log::Error("Cannot find a compatible Vulkan device or driver. Try updating your video driver to a more recent version and make sure your video card supports Vulkan.");
		return false;
	}

	if (gpuCount == 0)
	{
		Log::Error("Couldn't enumerate physical devices! Make sure your drivers are up to date and that you are not pending a reboot.");
		return false;
	}

	Log::Message("Found " + std::to_string(gpuCount) + " device(s)");

	std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
	vkEnumeratePhysicalDevices(m_instance, &gpuCount, physicalDevices.data());

	struct DeviceInfo
	{
		std::shared_ptr<VulkanDevice> device;
		uint32_t deviceIndex;
	};
	std::vector<DeviceInfo> discreteDevices;
	std::vector<DeviceInfo> integratedDevices;

	for (uint32_t index = 0; index < gpuCount; ++index)
	{
		std::shared_ptr<VulkanDevice> newDevice = std::make_shared<VulkanDevice>(physicalDevices[index]);
		bool isDiscrete = newDevice->QueryGPU(index);
		if (isDiscrete)
			discreteDevices.push_back({ newDevice, index });
		else
			integratedDevices.push_back({ newDevice, index });
	}

	for (int32_t index = 0; index < integratedDevices.size(); ++index)
		discreteDevices.push_back(integratedDevices[index]);

	int32_t deviceIndex = -1;
	if (discreteDevices.size() > 0)
	{
		int32_t preferredVendor = -1;
		if (discreteDevices.size() > 1 && preferredVendor != -1)
		{
			for (int32_t index = 0; index < discreteDevices.size(); ++index)
			{
				if (discreteDevices[index].device->GetDeviceProperties().vendorID == preferredVendor)
				{
					m_device = discreteDevices[index].device;
					deviceIndex = discreteDevices[index].deviceIndex;
					break;
				}
			}
		}

		if (deviceIndex == -1)
		{
			m_device = discreteDevices[0].device;
			deviceIndex = discreteDevices[0].deviceIndex;
		}
	}
	else
	{
		Log::Message("No devices found!");
		deviceIndex = -1;
		return false;
	}

	for (int32_t i = 0; i < m_appDeviceExtensions.size(); ++i)
	{
		m_device->AddAppDeviceExtensions(m_appDeviceExtensions[i]);
	}

	m_device->SetPhysicalDeviceFeatures(m_physicalDeviceFeatures2);

	m_device->InitGPU(deviceIndex);
	return true;
}

bool VulkanRHI::recreateSwapChain(int32_t width, int32_t height) noexcept
{
	destorySwapChain();

	uint32_t desiredNumBackBuffers = 3;
	m_swapChain = std::shared_ptr<VulkanSwapChain>(new VulkanSwapChain(*m_windowInfo, m_instance, m_device, m_pixelFormat, width, height, &desiredNumBackBuffers, m_backbufferImages, 1));

	m_backbufferViews.resize(m_backbufferImages.size());
	for (int32_t i = 0; i < m_backbufferViews.size(); ++i)
	{
		VkImageViewCreateInfo imageViewCreateInfo;
		ZeroVulkanStruct(imageViewCreateInfo, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
		imageViewCreateInfo.format = PixelFormatToVkFormat(m_pixelFormat, false);
		imageViewCreateInfo.components = m_device->GetFormatComponentMapping(m_pixelFormat);
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.image = m_backbufferImages[i];
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;
		VERIFYVULKANRESULT(vkCreateImageView(m_device->GetInstanceHandle(), &imageViewCreateInfo, VULKAN_CPU_ALLOCATOR, &(m_backbufferViews[i])));
	}

	return true;
}

void VulkanRHI::destorySwapChain() noexcept
{
	m_swapChain = nullptr;
	for (size_t i = 0; i < m_backbufferViews.size(); ++i)
		vkDestroyImageView(m_device->GetInstanceHandle(), m_backbufferViews[i], VULKAN_CPU_ALLOCATOR);
}