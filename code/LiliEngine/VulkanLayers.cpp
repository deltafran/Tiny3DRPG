#include "stdafx.h"
#include "VulkanRHI.h"
#include "VulkanDevice.h"
#include "StringUtils.h"
#include "Log.h"

void GetInstanceExtensionsPlatform(std::vector<const char*>& outExtensions) noexcept; // VulkanPlatform.cpp

struct VulkanLayerExtension
{
	VulkanLayerExtension() noexcept;
	void AddUniqueExtensionNames(std::vector<std::string>& outExtensions) noexcept;
	void AddUniqueExtensionNames(std::vector<const char*>& outExtensions) noexcept;

	VkLayerProperties layerProps;
	std::vector<VkExtensionProperties> extensionProps;
};

static const char* G_ValidationLayersInstance[] =
{
#if PLATFORM_WINDOWS
	"VK_LAYER_KHRONOS_validation",
#elif PLATFORM_MAC
	"VK_LAYER_KHRONOS_validation",
	"VK_LAYER_GOOGLE_unique_objects",
	"VK_LAYER_GOOGLE_threading",
	"VK_LAYER_LUNARG_core_validation",
	"VK_LAYER_LUNARG_parameter_validation",
	"VK_LAYER_LUNARG_object_tracker",
#elif PLATFORM_IOS
	"MoltenVK",
#elif PLATFORM_ANDROID
	"VK_LAYER_GOOGLE_threading",
	"VK_LAYER_LUNARG_parameter_validation",
	"VK_LAYER_LUNARG_object_tracker",
	"VK_LAYER_LUNARG_core_validation",
	"VK_LAYER_LUNARG_swapchain",
	"VK_LAYER_GOOGLE_unique_objects",
#elif PLATFORM_LINUX
	"VK_LAYER_GOOGLE_threading",
	"VK_LAYER_LUNARG_parameter_validation",
	"VK_LAYER_LUNARG_object_tracker",
	"VK_LAYER_LUNARG_core_validation",
	"VK_LAYER_GOOGLE_unique_objects",
#elif PLATFORM_ANDROID

#endif
	nullptr
};

static const char* G_ValidationLayersDevice[] =
{
#if PLATFORM_WINDOWS
	"VK_LAYER_KHRONOS_validation",
#elif PLATFORM_IOS
	"MoltenVK",
#elif PLATFORM_MAC
	"VK_LAYER_KHRONOS_validation",
	"VK_LAYER_GOOGLE_unique_objects",
	"VK_LAYER_GOOGLE_threading",
	"VK_LAYER_LUNARG_core_validation",
	"VK_LAYER_LUNARG_parameter_validation",
	"VK_LAYER_LUNARG_object_tracker",
#elif PLATFORM_ANDROID
	// "VK_LAYER_GOOGLE_threading",
	// "VK_LAYER_LUNARG_parameter_validation",
	// "VK_LAYER_LUNARG_object_tracker",
	// "VK_LAYER_LUNARG_core_validation",
	// "VK_LAYER_GOOGLE_unique_objects",
#elif PLATFORM_LINUX
	"VK_LAYER_GOOGLE_threading",
	"VK_LAYER_LUNARG_parameter_validation",
	"VK_LAYER_LUNARG_object_tracker",
	"VK_LAYER_LUNARG_core_validation",
	"VK_LAYER_GOOGLE_unique_objects",
	"VK_LAYER_LUNARG_core_validation",
#elif PLATFORM_ANDROID

#endif
	nullptr
};

static const char* G_InstanceExtensions[] =
{
#if PLATFORM_WINDOWS

#elif PLATFORM_MAC

#elif PLATFORM_IOS

#elif PLATFORM_LINUX

#elif PLATFORM_ANDROID

#endif
	nullptr
};

static const char* G_DeviceExtensions[] =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME,
	"VK_KHR_maintenance1",

#if PLATFORM_WINDOWS

#elif PLATFORM_MAC

#elif PLATFORM_IOS

#elif PLATFORM_LINUX

#elif PLATFORM_ANDROID

#endif

	nullptr
};

static inline void EnumerateInstanceExtensionProperties(const char* layerName, VulkanLayerExtension& outLayer) noexcept
{
	uint32_t count = 0;
	vkEnumerateInstanceExtensionProperties(layerName, &count, nullptr);
	outLayer.extensionProps.resize(count);
	vkEnumerateInstanceExtensionProperties(layerName, &count, outLayer.extensionProps.data());
}

static inline void EnumerateDeviceExtensionProperties(VkPhysicalDevice device, const char* layerName, VulkanLayerExtension& outLayer) noexcept
{
	uint32_t count = 0;
	vkEnumerateDeviceExtensionProperties(device, layerName, &count, nullptr);
	outLayer.extensionProps.resize(count);
	vkEnumerateDeviceExtensionProperties(device, layerName, &count, outLayer.extensionProps.data());
}

static inline int32_t FindLayerIndexInList(const std::vector<VulkanLayerExtension>& layers, const char* layerName) noexcept
{
	for (size_t i = 0; i < layers.size(); ++i)
	{
		if (strcmp(layers[i].layerProps.layerName, layerName) == 0)
			return i;
	}
	return -1;
}

static inline bool FindLayerInList(const std::vector<VulkanLayerExtension>& layers, const char* layerName) noexcept
{
	return FindLayerIndexInList(layers, layerName) != -1;
}

static inline bool FindLayerExtensionInList(const std::vector<VulkanLayerExtension>& layers, const char* extensionName, const char*& foundLayer) noexcept
{
	for (size_t i = 0; i < layers.size(); ++i)
	{
		for (size_t j = 0; j < layers[i].extensionProps.size(); ++j)
		{
			if (strcmp(layers[i].extensionProps[j].extensionName, extensionName) == 0)
			{
				foundLayer = layers[i].layerProps.layerName;
				return true;
			}
		}
	}
	return false;
}

static inline bool FindLayerExtensionInList(const std::vector<VulkanLayerExtension>& layers, const char* extensionName) noexcept
{
	const char* dummy = nullptr;
	return FindLayerExtensionInList(layers, extensionName, dummy);
}

static inline void TrimDuplicates(std::vector<const char*>& arr) noexcept
{
	for (int32_t i = (int32_t)arr.size() - 1; i >= 0; --i)
	{
		bool found = false;
		for (int32_t j = i - 1; j >= 0; --j)
		{
			if (strcmp(arr[i], arr[j]) == 0) {
				found = true;
				break;
			}
		}
		if (found) {
			arr.erase(arr.begin() + i);
		}
	}
}

VulkanLayerExtension::VulkanLayerExtension() noexcept
{
	memset(&layerProps, 0, sizeof(VkLayerProperties));
}

void VulkanLayerExtension::AddUniqueExtensionNames(std::vector<std::string>& outExtensions) noexcept
{
	for (size_t i = 0; i < extensionProps.size(); ++i)
		StringUtils::AddUnique(outExtensions, extensionProps[i].extensionName);
}

void VulkanLayerExtension::AddUniqueExtensionNames(std::vector<const char*>& outExtensions) noexcept
{
	for (size_t i = 0; i < extensionProps.size(); ++i)
		StringUtils::AddUnique(outExtensions, extensionProps[i].extensionName);
}

void VulkanRHI::getInstanceLayersAndExtensions() noexcept
{
	std::vector<VulkanLayerExtension> globalLayerExtensions(1);
	EnumerateInstanceExtensionProperties(nullptr, globalLayerExtensions[0]);

	std::vector<std::string> foundUniqueExtensions;
	for (int32_t i = 0; i < globalLayerExtensions[0].extensionProps.size(); ++i) 
		StringUtils::AddUnique(foundUniqueExtensions, globalLayerExtensions[0].extensionProps[i].extensionName);

	uint32_t instanceLayerCount = 0;
	vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
	std::vector<VkLayerProperties> globalLayerProperties(instanceLayerCount);
	vkEnumerateInstanceLayerProperties(&instanceLayerCount, globalLayerProperties.data());

	std::vector<std::string> foundUniqueLayers;
	for (int32_t i = 0; i < globalLayerProperties.size(); ++i)
	{
		VulkanLayerExtension layer;
		layer.layerProps = globalLayerProperties[i];
		EnumerateInstanceExtensionProperties(globalLayerProperties[i].layerName, layer);
		layer.AddUniqueExtensionNames(foundUniqueExtensions);
		StringUtils::AddUnique(foundUniqueLayers, globalLayerProperties[i].layerName);
		globalLayerExtensions.push_back(layer);
	}

	for (const std::string& name : foundUniqueLayers)
		Log::Message("- Found instance layer " + name);

	for (const std::string& name : foundUniqueExtensions)
		Log::Message("- Found instance extension " + name);

#ifdef _DEBUG
	for (size_t i = 0; G_ValidationLayersInstance[i] != nullptr; ++i)
	{
		const char* currValidationLayer = G_ValidationLayersInstance[i];
		bool found = FindLayerInList(globalLayerExtensions, currValidationLayer);
		if (found)
			m_instanceLayers.push_back(currValidationLayer);
		else
			Log::Error("Unable to find Vulkan instance validation layer '" + std::string(currValidationLayer) + "'");
	}

	if (FindLayerExtensionInList(globalLayerExtensions, VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
		m_instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

#endif // _DEBUG

	std::vector<const char*> platformExtensions;
	GetInstanceExtensionsPlatform(platformExtensions);

	for (const char* extension : platformExtensions)
	{
		if (FindLayerExtensionInList(globalLayerExtensions, extension)) 
		{
			m_instanceExtensions.push_back(extension);
		}
	}

	for (size_t i = 0; G_InstanceExtensions[i] != nullptr; ++i) 
	{
		if (FindLayerExtensionInList(globalLayerExtensions, G_InstanceExtensions[i]))
		{
			m_instanceExtensions.push_back(G_InstanceExtensions[i]);
		}
	}

	TrimDuplicates(m_instanceLayers);
	if (m_instanceLayers.size() > 0)
	{
		Log::Message("Using instance layers");
		for (const char* layer : m_instanceLayers)
			Log::Message("* " + std::string(layer));
	}
	else
		Log::Message("Not using instance layers");

	TrimDuplicates(m_instanceExtensions);
	if (m_instanceExtensions.size() > 0)
	{
		Log::Message("Using instance extensions");
		for (const char* extension : m_instanceExtensions)
			Log::Message("* " + std::string(extension));
	}
	else
		Log::Message("Not using instance extensions");
}

void VulkanDevice::getDeviceExtensionsAndLayers(std::vector<const char*>& outDeviceExtensions, std::vector<const char*>& outDeviceLayers, bool& bOutDebugMarkers) noexcept
{
	bOutDebugMarkers = false;

	uint32_t count = 0;
	vkEnumerateDeviceLayerProperties(m_physicalDevice, &count, nullptr);
	std::vector<VkLayerProperties> properties(count);
	vkEnumerateDeviceLayerProperties(m_physicalDevice, &count, properties.data());

	std::vector<VulkanLayerExtension> deviceLayerExtensions(count + 1);
	for (int32_t index = 1; index < deviceLayerExtensions.size(); ++index)
		deviceLayerExtensions[index].layerProps = properties[index - 1];

	std::vector<std::string> foundUniqueLayers;
	std::vector<std::string> foundUniqueExtensions;
	for (int32_t index = 0; index < deviceLayerExtensions.size(); ++index)
	{
		if (index == 0)
			EnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, deviceLayerExtensions[index]);
		else
		{
			StringUtils::AddUnique(foundUniqueLayers, deviceLayerExtensions[index].layerProps.layerName);
			EnumerateDeviceExtensionProperties(m_physicalDevice, deviceLayerExtensions[index].layerProps.layerName, deviceLayerExtensions[index]);
		}

		deviceLayerExtensions[index].AddUniqueExtensionNames(foundUniqueExtensions);
	}

	for (const std::string& name : foundUniqueLayers) 
		Log::Message("- Found device layer " + name);

	for (const std::string& name : foundUniqueExtensions) 
	{
		Log::Message("- Found device extension " + name);
	}

#ifdef _DEBUG
	for (uint32_t layerIndex = 0; G_ValidationLayersDevice[layerIndex] != nullptr; ++layerIndex)
	{
		bool bValidationFound = false;
		const char* currValidationLayer = G_ValidationLayersDevice[layerIndex];
		for (int32_t index = 1; index < deviceLayerExtensions.size(); ++index)
		{
			if (strcmp(deviceLayerExtensions[index].layerProps.layerName, currValidationLayer) == 0)
			{
				bValidationFound = true;
				outDeviceLayers.push_back(currValidationLayer);
				break;
			}
		}

		if (!bValidationFound) 
			Log::Error("Unable to find Vulkan device validation layer '" + std::string(currValidationLayer) + "'");
	}
#endif

	std::vector<const char*> availableExtensions;
	for (int32_t extIndex = 0; extIndex < deviceLayerExtensions[0].extensionProps.size(); ++extIndex)
		availableExtensions.push_back(deviceLayerExtensions[0].extensionProps[extIndex].extensionName);

	for (int32_t layerIndex = 0; layerIndex < outDeviceLayers.size(); ++layerIndex)
	{
		int32_t findLayerIndex;
		for (findLayerIndex = 1; findLayerIndex < deviceLayerExtensions.size(); ++findLayerIndex)
		{
			if (strcmp(deviceLayerExtensions[findLayerIndex].layerProps.layerName, outDeviceLayers[layerIndex]) == 0)
				break;
		}

		if (findLayerIndex < deviceLayerExtensions.size())
			deviceLayerExtensions[findLayerIndex].AddUniqueExtensionNames(availableExtensions);
	}

	TrimDuplicates(availableExtensions);

	auto ListContains = [](const std::vector<const char*>& arr, const char* name) -> bool
	{
		for (const char* element : arr)
		{
			if (strcmp(element, name) == 0)
				return true;
		}
		return false;
	};

	//std::vector<const char*> platformExtensions;
	//VulkanPlatform::GetDeviceExtensions(platformExtensions);
	//for (const char* platformExtension : platformExtensions)
	//{
	//	if (ListContains(availableExtensions, platformExtension))
	//	{
	//		outDeviceExtensions.push_back(platformExtension);
	//		break;
	//	}
	//}

	for (uint32_t index = 0; G_DeviceExtensions[index] != nullptr; ++index)
	{
		if (ListContains(availableExtensions, G_DeviceExtensions[index]))
			outDeviceExtensions.push_back(G_DeviceExtensions[index]);
	}

	if (outDeviceExtensions.size() > 0)
	{
		Log::Message("Using device extensions");
		for (const char* extension : outDeviceExtensions)
			Log::Message("* " + std::string(extension));
	}

	if (outDeviceLayers.size() > 0)
	{
		Log::Message("Using device layers");
		for (const char* layer : outDeviceLayers)
			Log::Message("* " + std::string(layer));
	}
}