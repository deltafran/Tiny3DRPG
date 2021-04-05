#pragma once

class VulkanDevice;

class VulkanQueue final
{
public:
    VulkanQueue(VulkanDevice* device, uint32_t familyIndex) noexcept;

    inline uint32_t GetFamilyIndex() const noexcept
    {
        return m_familyIndex;
    }

    inline VkQueue GetHandle() const noexcept
    {
        return m_queue;
    }

private:
    VkQueue m_queue = VK_NULL_HANDLE;
    uint32_t m_familyIndex;
    VulkanDevice* m_device;
};