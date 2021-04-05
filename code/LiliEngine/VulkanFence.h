#pragma once

class VulkanDevice;
class VulkanFenceManager;

class VulkanFence final
{
public:
	enum class State
	{
		NotReady,
		Signaled,
	};

	VulkanFence(VulkanDevice* device, VulkanFenceManager* owner, bool createSignaled) noexcept;

	inline VkFence GetHandle() const noexcept
	{
		return m_vkFence;
	}

	inline bool IsSignaled() const noexcept
	{
		return m_state == State::Signaled;
	}

	VulkanFenceManager* GetOwner() noexcept
	{
		return m_owner;
	}

private:
	~VulkanFence();
	friend class VulkanFenceManager;

	VkFence             m_vkFence = VK_NULL_HANDLE;
	State               m_state;
	VulkanFenceManager* m_owner;
};

class VulkanFenceManager final
{
public:
	~VulkanFenceManager();

	void Init(VulkanDevice* device) noexcept;

	void Destory() noexcept;

	VulkanFence* CreateFence(bool createSignaled = false) noexcept;

	bool WaitForFence(VulkanFence* fence, uint64_t timeInNanoseconds) noexcept;

	void ResetFence(VulkanFence* fence) noexcept;

	void ReleaseFence(VulkanFence*& fence) noexcept;

	void WaitAndReleaseFence(VulkanFence*& fence, uint64_t timeInNanoseconds) noexcept;

	inline bool IsFenceSignaled(VulkanFence* fence) noexcept
	{
		if (fence->IsSignaled())
			return true;
		return checkFenceState(fence);
	}

private:
	bool checkFenceState(VulkanFence* fence) noexcept;
	void destoryFence(VulkanFence* fence) noexcept;

	VulkanDevice* m_device = nullptr;
	std::vector<VulkanFence*> m_freeFences;
	std::vector<VulkanFence*> m_usedFences;
};

class VulkanSemaphore final
{
public:
	VulkanSemaphore(VulkanDevice* device) noexcept;
	~VulkanSemaphore();

	inline VkSemaphore GetHandle() const noexcept
	{
		return m_vkSemaphore;
	}

private:
	VkSemaphore m_vkSemaphore = VK_NULL_HANDLE;
	VulkanDevice* m_device;
};