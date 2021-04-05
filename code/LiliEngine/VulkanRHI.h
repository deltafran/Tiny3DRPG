#pragma once

#include "PixelFormat.h"
#include "WindowInfo.h"

class VulkanDevice;
class VulkanQueue;
class VulkanSwapChain;

class VulkanRHI final
{
public:
	VulkanRHI() = default;

	bool Init(const WindowInfo& m_windowInfo, int32_t width, int32_t height) noexcept;
	void Close() noexcept;

	inline void AddAppDeviceExtensions(const char* name) noexcept
	{
		m_appDeviceExtensions.push_back(name);
	}

	inline void AddAppInstanceExtensions(const char* name) noexcept
	{
		m_appInstanceExtensions.push_back(name);
	}

	inline void SetPhysicalDeviceFeatures(VkPhysicalDeviceFeatures2* deviceFeatures) noexcept
	{
		m_physicalDeviceFeatures2 = deviceFeatures;
	}

	inline const VkInstance& GetInstance() const noexcept
	{
		return m_instance;
	}

	inline std::shared_ptr<VulkanDevice> GetDevice() const noexcept
	{
		return m_device;
	}

	inline std::shared_ptr<VulkanSwapChain> GetSwapChain() const noexcept
	{
		return m_swapChain;
	}

	inline const std::vector<VkImage>& GetBackbufferImages() const noexcept
	{
		return m_backbufferImages;
	}

	inline const std::vector<VkImageView>& GetBackbufferViews() const noexcept
	{
		return m_backbufferViews;
	}

	inline const PixelFormat& GetPixelFormat() const noexcept
	{
		return m_pixelFormat;
	}

private:
	VulkanRHI(const VulkanRHI&) = delete;
	VulkanRHI(VulkanRHI&&) = delete;
	VulkanRHI& operator=(const VulkanRHI&) = delete;
	VulkanRHI& operator=(VulkanRHI&&) = delete;

	bool createInstance() noexcept;
	void getInstanceLayersAndExtensions() noexcept;
#ifdef _DEBUG
	void setupDebugLayerCallback() noexcept;
	void removeDebugLayerCallback() noexcept;
#endif
	bool selectAndInitDevice() noexcept;
	bool recreateSwapChain(int32_t width, int32_t height) noexcept;
	void destorySwapChain() noexcept;


#ifdef _DEBUG
	VkDebugReportCallbackEXT m_msgCallback = VK_NULL_HANDLE;
#endif

	VkInstance m_instance = VK_NULL_HANDLE;
	std::vector<const char*> m_instanceLayers;
	std::vector<const char*> m_instanceExtensions;
	std::vector<const char*> m_appDeviceExtensions;
	std::vector<const char*> m_appInstanceExtensions;
	VkPhysicalDeviceFeatures2* m_physicalDeviceFeatures2 = nullptr;

	std::shared_ptr<VulkanDevice> m_device = nullptr;

	std::shared_ptr<VulkanSwapChain> m_swapChain = nullptr;
	PixelFormat m_pixelFormat = PF_B8G8R8A8;
	std::vector<VkImage> m_backbufferImages;
	std::vector<VkImageView> m_backbufferViews;

	const WindowInfo* m_windowInfo = nullptr;
};