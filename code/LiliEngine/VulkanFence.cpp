#include "stdafx.h"
#include "VulkanFence.h"
#include "VulkanGlobals.h"
#include "VulkanDevice.h"
#include "Log.h"

// VulkanFence

VulkanFence::VulkanFence(VulkanDevice* device, VulkanFenceManager* owner, bool createSignaled) noexcept
	: m_state(createSignaled ? VulkanFence::State::Signaled : VulkanFence::State::NotReady)
	, m_owner(owner)
{
	VkFenceCreateInfo createInfo;
	ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
	createInfo.flags = createSignaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
	vkCreateFence(device->GetInstanceHandle(), &createInfo, VULKAN_CPU_ALLOCATOR, &m_vkFence);
}

VulkanFence::~VulkanFence()
{
	if (m_vkFence != VK_NULL_HANDLE)
		Log::Error("Didn't get properly destroyed by FenceManager!");
}

// VulkanFenceManager

VulkanFenceManager::~VulkanFenceManager()
{
	if (m_usedFences.size() > 0)
		Log::Error("No all fences are done!");
}

void VulkanFenceManager::Init(VulkanDevice* device) noexcept
{
	m_device = device;
}

void VulkanFenceManager::Destory() noexcept
{
	if (m_usedFences.size() > 0)
		Log::Error("No all fences are done!");

	for (int32_t i = 0; i < m_freeFences.size(); ++i)
		destoryFence(m_freeFences[i]);
}

VulkanFence* VulkanFenceManager::CreateFence(bool createSignaled) noexcept
{
	if (m_freeFences.size() > 0)
	{
		VulkanFence* fence = m_freeFences.back();
		m_freeFences.pop_back();
		m_usedFences.push_back(fence);
		if (createSignaled)
			fence->m_state = VulkanFence::State::Signaled;
		return fence;
	}

	VulkanFence* newFence = new VulkanFence(m_device, this, createSignaled);
	m_usedFences.push_back(newFence);
	return newFence;
}

bool VulkanFenceManager::WaitForFence(VulkanFence* fence, uint64_t timeInNanoseconds) noexcept
{
	VkResult result = vkWaitForFences(m_device->GetInstanceHandle(), 1, &fence->m_vkFence, true, timeInNanoseconds);
	switch (result)
	{
	case VK_SUCCESS:
		fence->m_state = VulkanFence::State::Signaled;
		return true;
	case VK_TIMEOUT:
		return false;
	default:
		Log::Error("Unkow error " + std::to_string((int32_t)result));
		return false;
	}
}

void VulkanFenceManager::ResetFence(VulkanFence* fence) noexcept
{
	if (fence->m_state != VulkanFence::State::NotReady)
	{
		vkResetFences(m_device->GetInstanceHandle(), 1, &fence->m_vkFence);
		fence->m_state = VulkanFence::State::NotReady;
	}
}

void VulkanFenceManager::ReleaseFence(VulkanFence*& fence) noexcept
{
	ResetFence(fence);
	for (int32_t i = 0; i < m_usedFences.size(); ++i)
	{
		if (m_usedFences[i] == fence)
		{
			m_usedFences.erase(m_usedFences.begin() + i);
			break;
		}
	}
	m_freeFences.push_back(fence);
	fence = nullptr;
}

void VulkanFenceManager::WaitAndReleaseFence(VulkanFence*& fence, uint64_t timeInNanoseconds) noexcept
{
	if (!fence->IsSignaled())
		WaitForFence(fence, timeInNanoseconds);

	ReleaseFence(fence);
}

bool VulkanFenceManager::checkFenceState(VulkanFence* fence) noexcept
{
	VkResult result = vkGetFenceStatus(m_device->GetInstanceHandle(), fence->m_vkFence);
	switch (result)
	{
	case VK_SUCCESS:
		fence->m_state = VulkanFence::State::Signaled;
		break;
	case VK_NOT_READY:
		break;
	default:
		break;
	}
	return false;
}

void VulkanFenceManager::destoryFence(VulkanFence* fence) noexcept
{
	vkDestroyFence(m_device->GetInstanceHandle(), fence->m_vkFence, VULKAN_CPU_ALLOCATOR);
	fence->m_vkFence = VK_NULL_HANDLE;
	delete fence;
}

// VulkanSemaphore
VulkanSemaphore::VulkanSemaphore(VulkanDevice* device) noexcept
	: m_device(device)
{
	VkSemaphoreCreateInfo createInfo;
	ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);
	vkCreateSemaphore(device->GetInstanceHandle(), &createInfo, VULKAN_CPU_ALLOCATOR, &m_vkSemaphore);
}

VulkanSemaphore::~VulkanSemaphore()
{
	if (m_vkSemaphore == VK_NULL_HANDLE)
		Log::Error("Failed destory VkSemaphore.");

	vkDestroySemaphore(m_device->GetInstanceHandle(), m_vkSemaphore, VULKAN_CPU_ALLOCATOR);
}