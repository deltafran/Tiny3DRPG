#include "stdafx.h"
#include "VulkanQueue.h"
#include "VulkanDevice.h"

VulkanQueue::VulkanQueue(VulkanDevice* device, uint32_t familyIndex) noexcept
    : m_familyIndex(familyIndex)
    , m_device(device)
{
    vkGetDeviceQueue(m_device->GetInstanceHandle(), m_familyIndex, 0, &m_queue);
}