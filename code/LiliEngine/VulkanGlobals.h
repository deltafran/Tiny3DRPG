#pragma once

#include "Log.h"

#define VULKAN_CPU_ALLOCATOR nullptr

#define VERIFYVULKANRESULT(VkFunction)				{ const VkResult scopedResult = VkFunction; if (scopedResult != VK_SUCCESS) { Log::Error("VKResult=" + std::to_string(scopedResult) + ",Function=" + #VkFunction + ",File=" + __FILE__ + ",Line=" + std::to_string(__LINE__)); }}
#define VERIFYVULKANRESULT_EXPANDED(VkFunction)		{ const VkResult scopedResult = VkFunction; if (scopedResult < VK_SUCCESS)  { Log::Error("VKResult=" + std::to_string(scopedResult) + ",Function=" + #VkFunction + ",File=" + __FILE__ + ",Line=" + std::to_string(__LINE__)); }}

template<class T>
static FORCEINLINE void ZeroVulkanStruct(T& vkStruct, VkStructureType vkType)
{
	vkStruct.sType = vkType;
	memset(((uint8_t*)&vkStruct) + sizeof(VkStructureType), 0, sizeof(T) - sizeof(VkStructureType));
}

#define VK_FORMAT_RANGE_SIZE (VK_FORMAT_ASTC_12x12_SRGB_BLOCK + 1)

#define MLOG(...)   { char __str__buf__[2048]; sprintf_s(__str__buf__, "%-6s", "LOG:");   OutputDebugStringA(__str__buf__); sprintf_s(__str__buf__, "%-40s:%-5d", __func__, __LINE__); OutputDebugStringA(__str__buf__); sprintf_s(__str__buf__, __VA_ARGS__); OutputDebugStringA(__str__buf__); sprintf_s(__str__buf__, "\n"); OutputDebugStringA(__str__buf__); }
#define MLOGE(...)  { char __str__buf__[2048]; sprintf_s(__str__buf__, "%-6s", "ERROR:"); OutputDebugStringA(__str__buf__); sprintf_s(__str__buf__, "%-40s:%-5d", __func__, __LINE__); OutputDebugStringA(__str__buf__); sprintf_s(__str__buf__, __VA_ARGS__); OutputDebugStringA(__str__buf__); sprintf_s(__str__buf__, "\n"); OutputDebugStringA(__str__buf__); }