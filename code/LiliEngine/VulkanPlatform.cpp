#include "stdafx.h"
#include "VulkanGlobals.h"
#include "WindowInfo.h"

#if PLATFORM_WINDOWS
static const char* G_ValidationLayersInstance[] =
{
	VK_KHR_SURFACE_EXTENSION_NAME,
	VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
	nullptr
};

const char**getRequiredInstanceExtensions(uint32_t* count) noexcept
{
	*count = 2;
	return G_ValidationLayersInstance;
}

void GetInstanceExtensionsPlatform(std::vector<const char*>& outExtensions) noexcept
{
	uint32_t count;
	const char** extensions = getRequiredInstanceExtensions(&count);
	for (uint32_t i = 0; i < count; ++i)
		outExtensions.push_back(extensions[i]);
}

void CreateVKSurfaceFromWindows(const WindowInfo &windowInfo, VkInstance instance, VkSurfaceKHR* outSurface)
{
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
	ZeroVulkanStruct(surfaceCreateInfo, VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR);
	surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
	surfaceCreateInfo.hwnd = windowInfo.hwnd;
	VERIFYVULKANRESULT(vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, VULKAN_CPU_ALLOCATOR, outSurface));
}
#endif