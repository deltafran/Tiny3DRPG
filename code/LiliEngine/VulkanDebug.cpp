#include "stdafx.h"
#ifdef _DEBUG
#include "Log.h"
#include "VulkanRHI.h"
#include "VulkanGlobals.h"

#define VK_DESTORY_DEBUG_REPORT_CALLBACK_EXT_NAME "vkDestroyDebugReportCallbackEXT"
#define VK_CREATE_DEBUG_REPORT_CALLBACK_EXT_NAME  "vkCreateDebugReportCallbackEXT"

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallBack(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t obj,
	size_t location,
	int32_t code,
	const char* layerPrefix,
	const char* msg,
	void* userData) noexcept
{
	std::string prefix("");
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
		prefix += "ERROR:";
	if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
		prefix += "WARNING:";
	if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
		prefix += "PERFORMANCE:";
	if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
		prefix += "INFO:";
	if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
		prefix += "DEBUG:";

	Log::Error(prefix + "[" + layerPrefix + "] Code " + std::to_string(code) + " : " + msg);
	return VK_FALSE;
}

void VulkanRHI::setupDebugLayerCallback() noexcept
{
	VkDebugReportCallbackCreateInfoEXT debugInfo;
	ZeroVulkanStruct(debugInfo, VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT);
	debugInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	debugInfo.pfnCallback = VulkanDebugCallBack;
	debugInfo.pUserData = this;

	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(m_instance, VK_CREATE_DEBUG_REPORT_CALLBACK_EXT_NAME);
	bool success = true;
	if (func != nullptr)
		success = func(m_instance, &debugInfo, nullptr, &m_msgCallback) == VK_SUCCESS;
	else
		success = false;

	if (success)
		Log::Message("Setup debug callback success.");
	else
		Log::Error("Setup debug callback failed.");
}

void VulkanRHI::removeDebugLayerCallback() noexcept
{
	if (m_msgCallback != VK_NULL_HANDLE)
	{
		PFN_vkDestroyDebugReportCallbackEXT destroyMsgCallback = (PFN_vkDestroyDebugReportCallbackEXT)(void*)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugReportCallbackEXT");
		destroyMsgCallback(m_instance, m_msgCallback, nullptr);
	}
}
#endif