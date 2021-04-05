#pragma once

#include "VulkanQueue.h"
#include "VulkanGlobals.h"
#include "PixelFormat.h"

class VulkanFenceManager;
class VulkanDeviceMemoryManager;

class VulkanDevice final
{
public:
	VulkanDevice(VkPhysicalDevice physicalDevice) noexcept;
    ~VulkanDevice();

    bool QueryGPU(int32_t deviceIndex) noexcept;

    void InitGPU(int32_t deviceIndex) noexcept;

    void CreateDevice() noexcept;

    void Destroy() noexcept;

    inline std::shared_ptr<VulkanQueue> GetGraphicsQueue() noexcept
    {
        return m_gfxQueue;
    }

    inline std::shared_ptr<VulkanQueue> GetComputeQueue() noexcept
    {
        return m_computeQueue;
    }

    inline std::shared_ptr<VulkanQueue> GetTransferQueue() noexcept
    {
        return m_transferQueue;
    }

    inline std::shared_ptr<VulkanQueue> GetPresentQueue() noexcept
    {
        return m_presentQueue;
    }

    inline VkPhysicalDevice GetPhysicalHandle() const noexcept
    {
        return m_physicalDevice;
    }

    inline const VkPhysicalDeviceProperties& GetDeviceProperties() const noexcept
    {
        return m_physicalDeviceProperties;
    }

    inline const VkPhysicalDeviceLimits& GetLimits() const noexcept
    {
        return m_physicalDeviceProperties.limits;
    }

    inline const VkPhysicalDeviceFeatures& GetPhysicalFeatures() const noexcept
    {
        return m_physicalDeviceFeatures;
    }

    inline VkDevice GetInstanceHandle() const noexcept
    {
        return m_device;
    }

    inline const VkFormatProperties* GetFormatProperties() const noexcept
    {
        return m_formatProperties;
    }

    inline VulkanFenceManager& GetFenceManager() noexcept
    {
        return *m_fenceManager;
    }

    inline VulkanDeviceMemoryManager& GetMemoryManager() noexcept
    {
        return *m_memoryManager;
    }

    inline void AddAppDeviceExtensions(const char* name) noexcept
    {
        m_appDeviceExtensions.push_back(name);
    }

    inline void SetPhysicalDeviceFeatures(VkPhysicalDeviceFeatures2* deviceFeatures) noexcept
    {
        m_physicalDeviceFeatures2 = deviceFeatures;
    }

    bool IsFormatSupported(VkFormat format) noexcept;

    const VkComponentMapping& GetFormatComponentMapping(PixelFormat format) const noexcept;

    void SetupPresentQueue(VkSurfaceKHR surface);

private:
	VulkanDevice() = delete;
	VulkanDevice(const VulkanDevice&) = delete;
	VulkanDevice(VulkanDevice&&) = delete;
	VulkanDevice& operator=(const VulkanDevice&) = delete;
	VulkanDevice& operator=(VulkanDevice&&) = delete;

    void setupFormats() noexcept;
    void mapFormatSupport(PixelFormat format, VkFormat vkFormat) noexcept;
    void mapFormatSupport(PixelFormat format, VkFormat vkormat, int32_t blockBytes) noexcept;
    void setComponentMapping(PixelFormat format, VkComponentSwizzle r, VkComponentSwizzle g, VkComponentSwizzle b, VkComponentSwizzle a) noexcept;
    void getDeviceExtensionsAndLayers(std::vector<const char*>& outDeviceExtensions, std::vector<const char*>& outDeviceLayers, bool& bOutDebugMarkers) noexcept;

    VkDevice                                m_device = VK_NULL_HANDLE;
    VkPhysicalDevice                        m_physicalDevice = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties              m_physicalDeviceProperties;
    VkPhysicalDeviceFeatures                m_physicalDeviceFeatures;
    std::vector<VkQueueFamilyProperties>    m_queueFamilyProps;

    VkFormatProperties                      m_formatProperties[VK_FORMAT_RANGE_SIZE];
    std::map<VkFormat, VkFormatProperties>  m_extensionFormatProperties;
    VkComponentMapping                      m_pixelFormatComponentMapping[PF_MAX];

    std::shared_ptr<VulkanQueue>            m_gfxQueue = nullptr;
    std::shared_ptr<VulkanQueue>            m_computeQueue = nullptr;
    std::shared_ptr<VulkanQueue>            m_transferQueue = nullptr;
    std::shared_ptr<VulkanQueue>            m_presentQueue = nullptr;

    VulkanFenceManager*                     m_fenceManager = nullptr;
    VulkanDeviceMemoryManager*              m_memoryManager = nullptr;

    std::vector<const char*>				m_appDeviceExtensions;
    VkPhysicalDeviceFeatures2*              m_physicalDeviceFeatures2 = nullptr;
};