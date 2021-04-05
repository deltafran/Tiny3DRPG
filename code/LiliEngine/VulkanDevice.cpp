#include "stdafx.h"
#include "VulkanDevice.h"
#include "Log.h"
#include "VulkanGlobals.h"
#include "VulkanFence.h"
#include "VulkanMemory.h"

VulkanDevice::VulkanDevice(VkPhysicalDevice physicalDevice) noexcept
	: m_physicalDevice(physicalDevice)
{
}

VulkanDevice::~VulkanDevice()
{
    if (m_device != VK_NULL_HANDLE)
    {
        Destroy();
        m_device = VK_NULL_HANDLE;
    }
}

bool VulkanDevice::QueryGPU(int32_t deviceIndex) noexcept
{
	bool discrete = false;
	assert(m_physicalDevice);
	vkGetPhysicalDeviceProperties(m_physicalDevice, &m_physicalDeviceProperties);

	auto getDeviceTypeString = [&]() -> std::string
	{
		std::string info;
		switch (m_physicalDeviceProperties.deviceType)
		{
		case  VK_PHYSICAL_DEVICE_TYPE_OTHER: info = "Other"; break;
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: info = "Integrated GPU"; break;
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			info = "Discrete GPU";
			discrete = true;
			break;
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: info = "Virtual GPU"; break;
		case VK_PHYSICAL_DEVICE_TYPE_CPU: info = "CPU"; break;
		default: info = "Unknown"; break;
		}
		return info;
	};

	Log::Message("Device " + std::to_string(deviceIndex) + ": " + m_physicalDeviceProperties.deviceName);
	//Log::Message("- API " + std::to_string(VK_VERSION_MAJOR(m_physicalDeviceProperties.apiVersion)) + "." + std::to_string(VK_VERSION_MINOR(m_physicalDeviceProperties.apiVersion)) + "." + std::to_string(VK_VERSION_PATCH(m_physicalDeviceProperties.apiVersion)) + "(" + std::to_string(m_physicalDeviceProperties.apiVersion) + ") Driver " + std::to_string(m_physicalDeviceProperties.driverVersion) + " VendorId " + std::to_string(m_physicalDeviceProperties.vendorID));
	//Log::Message("- DeviceID " + std::to_string(m_physicalDeviceProperties.deviceID) + " Type " + getDeviceTypeString());
	MLOG("- API %d.%d.%d(0x%x) Driver 0x%x VendorId 0x%x", VK_VERSION_MAJOR(m_physicalDeviceProperties.apiVersion), VK_VERSION_MINOR(m_physicalDeviceProperties.apiVersion), VK_VERSION_PATCH(m_physicalDeviceProperties.apiVersion), m_physicalDeviceProperties.apiVersion, m_physicalDeviceProperties.driverVersion, m_physicalDeviceProperties.vendorID);
	MLOG("- DeviceID 0x%x Type %s", m_physicalDeviceProperties.deviceID, getDeviceTypeString().c_str());
	Log::Message("- Max Descriptor Sets Bound " + std::to_string(m_physicalDeviceProperties.limits.maxBoundDescriptorSets) + " Timestamps " + std::to_string(m_physicalDeviceProperties.limits.timestampComputeAndGraphics));

	uint32_t queueCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueCount, nullptr);
	m_queueFamilyProps.resize(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueCount, m_queueFamilyProps.data());

	return discrete;
}

void VulkanDevice::InitGPU(int32_t deviceIndex) noexcept
{
	vkGetPhysicalDeviceFeatures(m_physicalDevice, &m_physicalDeviceFeatures);
	Log::Message("Using Device " + std::to_string(deviceIndex) + ": Geometry " + std::to_string(m_physicalDeviceFeatures.geometryShader) + " Tessellation " + std::to_string(m_physicalDeviceFeatures.tessellationShader));

	CreateDevice();
	setupFormats();

	m_memoryManager = new VulkanDeviceMemoryManager();
	m_memoryManager->Init(this);

	m_fenceManager = new VulkanFenceManager();
	m_fenceManager->Init(this);
}

void VulkanDevice::CreateDevice() noexcept
{
	bool debugMarkersFound = false;
	std::vector<const char*> deviceExtensions;
	std::vector<const char*> validationLayers;
	getDeviceExtensionsAndLayers(deviceExtensions, validationLayers, debugMarkersFound);

	if (m_appDeviceExtensions.size() > 0)
	{
		Log::Message("Using app device extensions");
		for (size_t i = 0; i < m_appDeviceExtensions.size(); ++i)
		{
			deviceExtensions.push_back(m_appDeviceExtensions[i]);
			Log::Message("* " + std::string(m_appDeviceExtensions[i]));
		}
	}

	VkDeviceCreateInfo deviceInfo;
	ZeroVulkanStruct(deviceInfo, VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO);
	deviceInfo.enabledExtensionCount   = uint32_t(deviceExtensions.size());
	deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();
	deviceInfo.enabledLayerCount       = uint32_t(validationLayers.size());
	deviceInfo.ppEnabledLayerNames     = validationLayers.data();

	if (m_physicalDeviceFeatures2) 
	{
		deviceInfo.pNext = m_physicalDeviceFeatures2;
		deviceInfo.pEnabledFeatures = nullptr;
		m_physicalDeviceFeatures2->features = m_physicalDeviceFeatures;
	}
	else 
	{
		deviceInfo.pEnabledFeatures = &m_physicalDeviceFeatures;
	}

	Log::Message("Found "+ std::to_string(m_queueFamilyProps.size()) + " Queue Families");

	std::vector<VkDeviceQueueCreateInfo> queueFamilyInfos;

	uint32_t numPriorities = 0;
	int32_t gfxQueueFamilyIndex = -1;
	int32_t computeQueueFamilyIndex = -1;
	int32_t transferQueueFamilyIndex = -1;

	for (int32_t familyIndex = 0; familyIndex < m_queueFamilyProps.size(); ++familyIndex)
	{
		const VkQueueFamilyProperties& currProps = m_queueFamilyProps[familyIndex];
		bool isValidQueue = false;

		if ((currProps.queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT)
		{
			if (gfxQueueFamilyIndex == -1)
			{
				gfxQueueFamilyIndex = familyIndex;
				isValidQueue = true;
			}
		}

		if ((currProps.queueFlags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT)
		{
			if (computeQueueFamilyIndex == -1)
			{
				computeQueueFamilyIndex = familyIndex;
				isValidQueue = true;
			}
		}

		if ((currProps.queueFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT)
		{
			if (transferQueueFamilyIndex == -1)
			{
				transferQueueFamilyIndex = familyIndex;
				isValidQueue = true;
			}
		}

		auto getQueueInfoString = [](const VkQueueFamilyProperties& Props) -> std::string
		{
			std::string info;
			if ((Props.queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT)
				info += " Gfx";
			if ((Props.queueFlags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT)
				info += " Compute";
			if ((Props.queueFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT)
				info += " Xfer";
			if ((Props.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) == VK_QUEUE_SPARSE_BINDING_BIT)
				info += " Sparse";
			return info;
		};

		if (!isValidQueue)
		{
			Log::Message("Skipping unnecessary Queue Family " + std::to_string(familyIndex) + ": " + std::to_string(currProps.queueCount) + " queues" + getQueueInfoString(currProps));
			continue;
		}

		VkDeviceQueueCreateInfo currQueue;
		ZeroVulkanStruct(currQueue, VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO);
		currQueue.queueFamilyIndex = familyIndex;
		currQueue.queueCount = currProps.queueCount;
		numPriorities += currProps.queueCount;
		queueFamilyInfos.push_back(currQueue);

		Log::Message("Initializing Queue Family " + std::to_string(familyIndex) + ": " + std::to_string(currProps.queueCount) + " queues" + getQueueInfoString(currProps));
	}

	std::vector<float> queuePriorities(numPriorities);
	float* CurrentPriority = queuePriorities.data();
	for (int32_t index = 0; index < queueFamilyInfos.size(); ++index)
	{
		VkDeviceQueueCreateInfo& currQueue = queueFamilyInfos[index];
		currQueue.pQueuePriorities = CurrentPriority;
		const VkQueueFamilyProperties& currProps = m_queueFamilyProps[currQueue.queueFamilyIndex];
		for (int32_t queueIndex = 0; queueIndex < (int32_t)currProps.queueCount; ++queueIndex) 
		{
			*CurrentPriority++ = 1.0f;
		}
	}

	deviceInfo.queueCreateInfoCount = uint32_t(queueFamilyInfos.size());
	deviceInfo.pQueueCreateInfos = queueFamilyInfos.data();

	VkResult result = vkCreateDevice(m_physicalDevice, &deviceInfo, VULKAN_CPU_ALLOCATOR, &m_device);
	if (result == VK_ERROR_INITIALIZATION_FAILED)
	{
		Log::Error("Cannot create a Vulkan device. Try updating your video driver to a more recent version.");
		return;
	}

	m_gfxQueue = std::make_shared<VulkanQueue>(this, gfxQueueFamilyIndex);

	if (computeQueueFamilyIndex == -1)
		computeQueueFamilyIndex = gfxQueueFamilyIndex;

	m_computeQueue = std::make_shared<VulkanQueue>(this, computeQueueFamilyIndex);

	if (transferQueueFamilyIndex == -1)
		transferQueueFamilyIndex = computeQueueFamilyIndex;

	m_transferQueue = std::make_shared<VulkanQueue>(this, transferQueueFamilyIndex);
}

void VulkanDevice::Destroy() noexcept
{
	m_fenceManager->Destory();
	delete m_fenceManager;

	m_memoryManager->Destory();
	delete m_memoryManager;

	vkDestroyDevice(m_device, VULKAN_CPU_ALLOCATOR);
	m_device = VK_NULL_HANDLE;
}

void VulkanDevice::setupFormats() noexcept
{
	for (uint32_t index = 0; index < VK_FORMAT_RANGE_SIZE; ++index)
	{
		const VkFormat format = (VkFormat)index;
		memset(&m_formatProperties[index], 0, sizeof(VkFormat));
		vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &m_formatProperties[index]);
	}

	for (int32_t index = 0; index < PF_MAX; ++index)
	{
		G_PixelFormats[index].platformFormat = VK_FORMAT_UNDEFINED;
		G_PixelFormats[index].supported = false;

		VkComponentMapping& componentMapping = m_pixelFormatComponentMapping[index];
		componentMapping.r = VK_COMPONENT_SWIZZLE_R;
		componentMapping.g = VK_COMPONENT_SWIZZLE_G;
		componentMapping.b = VK_COMPONENT_SWIZZLE_B;
		componentMapping.a = VK_COMPONENT_SWIZZLE_A;
	}

	mapFormatSupport(PF_B8G8R8A8, VK_FORMAT_B8G8R8A8_UNORM);
	setComponentMapping(PF_B8G8R8A8, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A);

	mapFormatSupport(PF_G8, VK_FORMAT_R8_UNORM);
	setComponentMapping(PF_G8, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);

	mapFormatSupport(PF_G16, VK_FORMAT_R16_UNORM);
	setComponentMapping(PF_G16, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);

	mapFormatSupport(PF_FloatRGB, VK_FORMAT_B10G11R11_UFLOAT_PACK32);
	setComponentMapping(PF_FloatRGB, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_ZERO);

	mapFormatSupport(PF_FloatRGBA, VK_FORMAT_R16G16B16A16_SFLOAT, 8);
	setComponentMapping(PF_FloatRGBA, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A);

	mapFormatSupport(PF_DepthStencil, VK_FORMAT_D32_SFLOAT_S8_UINT);
	if (!G_PixelFormats[PF_DepthStencil].supported)
	{
		mapFormatSupport(PF_DepthStencil, VK_FORMAT_D24_UNORM_S8_UINT);
		if (!G_PixelFormats[PF_DepthStencil].supported)
		{
			mapFormatSupport(PF_DepthStencil, VK_FORMAT_D16_UNORM_S8_UINT);
			if (!G_PixelFormats[PF_DepthStencil].supported) 
			{
				Log::Error("No stencil texture format supported!");
			}
		}
	}
	setComponentMapping(PF_DepthStencil, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY);

	mapFormatSupport(PF_ShadowDepth, VK_FORMAT_D16_UNORM);
	setComponentMapping(PF_ShadowDepth, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY);

	mapFormatSupport(PF_G32R32F, VK_FORMAT_R32G32_SFLOAT, 8);
	setComponentMapping(PF_G32R32F, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);

	mapFormatSupport(PF_A32B32G32R32F, VK_FORMAT_R32G32B32A32_SFLOAT, 16);
	setComponentMapping(PF_A32B32G32R32F, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A);

	mapFormatSupport(PF_G16R16, VK_FORMAT_R16G16_UNORM);
	setComponentMapping(PF_G16R16, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);

	mapFormatSupport(PF_G16R16F, VK_FORMAT_R16G16_SFLOAT);
	setComponentMapping(PF_G16R16F, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);

	mapFormatSupport(PF_G16R16F_FILTER, VK_FORMAT_R16G16_SFLOAT);
	setComponentMapping(PF_G16R16F_FILTER, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);

	mapFormatSupport(PF_R16_UINT, VK_FORMAT_R16_UINT);
	setComponentMapping(PF_R16_UINT, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);

	mapFormatSupport(PF_R16_SINT, VK_FORMAT_R16_SINT);
	setComponentMapping(PF_R16_SINT, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);

	mapFormatSupport(PF_R32_UINT, VK_FORMAT_R32_UINT);
	setComponentMapping(PF_R32_UINT, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);

	mapFormatSupport(PF_R32_SINT, VK_FORMAT_R32_SINT);
	setComponentMapping(PF_R32_SINT, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);

	mapFormatSupport(PF_R8_UINT, VK_FORMAT_R8_UINT);
	setComponentMapping(PF_R8_UINT, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);

	mapFormatSupport(PF_D24, VK_FORMAT_D24_UNORM_S8_UINT);
	if (!G_PixelFormats[PF_D24].supported)
	{
		mapFormatSupport(PF_D24, VK_FORMAT_D16_UNORM_S8_UINT);
		if (!G_PixelFormats[PF_D24].supported)
		{
			mapFormatSupport(PF_D24, VK_FORMAT_D32_SFLOAT);
			if (!G_PixelFormats[PF_D24].supported)
			{
				mapFormatSupport(PF_D24, VK_FORMAT_D32_SFLOAT_S8_UINT);
				if (!G_PixelFormats[PF_D24].supported)
				{
					mapFormatSupport(PF_D24, VK_FORMAT_D16_UNORM);
					if (!G_PixelFormats[PF_D24].supported) {
						Log::Error("No Depth texture format supported!");
					}
				}
			}
		}
	}
	setComponentMapping(PF_D24, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);

	mapFormatSupport(PF_R16F, VK_FORMAT_R16_SFLOAT);
	setComponentMapping(PF_R16F, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);

	mapFormatSupport(PF_R16F_FILTER, VK_FORMAT_R16_SFLOAT);
	setComponentMapping(PF_R16F_FILTER, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);

	mapFormatSupport(PF_FloatR11G11B10, VK_FORMAT_B10G11R11_UFLOAT_PACK32, 4);
	setComponentMapping(PF_FloatR11G11B10, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_ZERO);

	mapFormatSupport(PF_A2B10G10R10, VK_FORMAT_A2B10G10R10_UNORM_PACK32, 4);
	setComponentMapping(PF_A2B10G10R10, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A);

	mapFormatSupport(PF_A16B16G16R16, VK_FORMAT_R16G16B16A16_UNORM, 8);
	setComponentMapping(PF_A16B16G16R16, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A);

	mapFormatSupport(PF_A8, VK_FORMAT_R8_UNORM);
	setComponentMapping(PF_A8, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_R);

	mapFormatSupport(PF_R5G6B5_UNORM, VK_FORMAT_R5G6B5_UNORM_PACK16);
	setComponentMapping(PF_R5G6B5_UNORM, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A);

	mapFormatSupport(PF_R8G8B8A8, VK_FORMAT_R8G8B8A8_UNORM);
	setComponentMapping(PF_R8G8B8A8, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A);

	mapFormatSupport(PF_R8G8B8A8_UINT, VK_FORMAT_R8G8B8A8_UINT);
	setComponentMapping(PF_R8G8B8A8_UINT, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A);

	mapFormatSupport(PF_R8G8B8A8_SNORM, VK_FORMAT_R8G8B8A8_SNORM);
	setComponentMapping(PF_R8G8B8A8_SNORM, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A);

	mapFormatSupport(PF_R16G16_UINT, VK_FORMAT_R16G16_UINT);
	setComponentMapping(PF_R16G16_UINT, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);

	mapFormatSupport(PF_R16G16B16A16_UINT, VK_FORMAT_R16G16B16A16_UINT);
	setComponentMapping(PF_R16G16B16A16_UINT, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A);

	mapFormatSupport(PF_R16G16B16A16_SINT, VK_FORMAT_R16G16B16A16_SINT);
	setComponentMapping(PF_R16G16B16A16_SINT, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A);

	mapFormatSupport(PF_R32G32B32A32_UINT, VK_FORMAT_R32G32B32A32_UINT);
	setComponentMapping(PF_R32G32B32A32_UINT, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A);

	mapFormatSupport(PF_R16G16B16A16_SNORM, VK_FORMAT_R16G16B16A16_SNORM);
	setComponentMapping(PF_R16G16B16A16_SNORM, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A);

	mapFormatSupport(PF_R16G16B16A16_UNORM, VK_FORMAT_R16G16B16A16_UNORM);
	setComponentMapping(PF_R16G16B16A16_UNORM, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A);

	mapFormatSupport(PF_R8G8, VK_FORMAT_R8G8_UNORM);
	setComponentMapping(PF_R8G8, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);

	mapFormatSupport(PF_V8U8, VK_FORMAT_R8G8_UNORM);
	setComponentMapping(PF_V8U8, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);

	mapFormatSupport(PF_R32_FLOAT, VK_FORMAT_R32_SFLOAT);
	setComponentMapping(PF_R32_FLOAT, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);
}

void VulkanDevice::mapFormatSupport(PixelFormat format, VkFormat vkFormat) noexcept
{
	PixelFormatInfo& formatInfo = G_PixelFormats[format];
	formatInfo.platformFormat = vkFormat;
	formatInfo.supported = IsFormatSupported(vkFormat);

	if (!formatInfo.supported)
		Log::Error("PixelFormat(" + std::to_string((int32_t)format) + ") is not supported with Vk format " + std::to_string((int32_t)vkFormat));
}

void VulkanDevice::mapFormatSupport(PixelFormat format, VkFormat vkFormat, int32_t blockBytes) noexcept
{
	mapFormatSupport(format, vkFormat);
	PixelFormatInfo& formatInfo = G_PixelFormats[format];
	formatInfo.blockBytes = blockBytes;
}

void VulkanDevice::setComponentMapping(PixelFormat format, VkComponentSwizzle r, VkComponentSwizzle g, VkComponentSwizzle b, VkComponentSwizzle a) noexcept
{
	VkComponentMapping& componentMapping = m_pixelFormatComponentMapping[format];
	componentMapping.r = r;
	componentMapping.g = g;
	componentMapping.b = b;
	componentMapping.a = a;
}

bool VulkanDevice::IsFormatSupported(VkFormat format) noexcept
{
	auto ArePropertiesSupported = [](const VkFormatProperties& prop) -> bool
	{
		return (prop.bufferFeatures != 0) || (prop.linearTilingFeatures != 0) || (prop.optimalTilingFeatures != 0);
	};

	if (format >= 0 && format < VK_FORMAT_RANGE_SIZE)
	{
		const VkFormatProperties& prop = m_formatProperties[format];
		return ArePropertiesSupported(prop);
	}

	auto it = m_extensionFormatProperties.find(format);
	if (it != m_extensionFormatProperties.end())
	{
		const VkFormatProperties& foundProperties = it->second;
		return ArePropertiesSupported(foundProperties);
	}

	VkFormatProperties newProperties;
	memset(&newProperties, 0, sizeof(VkFormatProperties));

	vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &newProperties);
	m_extensionFormatProperties.insert(std::pair<VkFormat, VkFormatProperties>(format, newProperties));

	return ArePropertiesSupported(newProperties);
}

const VkComponentMapping& VulkanDevice::GetFormatComponentMapping(PixelFormat format) const noexcept
{
	return m_pixelFormatComponentMapping[format];
}