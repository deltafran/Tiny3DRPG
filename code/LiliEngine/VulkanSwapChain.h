#pragma once

#include "PixelFormat.h"

class VulkanQueue;
class VulkanFence;
class VulkanDevice;
struct WindowInfo;

class VulkanSwapChain
{
public:
	enum class SwapStatus
	{
		Healthy = 0,
		OutOfDate = -1,
		SurfaceLost = -2,
	};

	VulkanSwapChain(const WindowInfo& windowInfo, VkInstance instance, std::shared_ptr<VulkanDevice> device, PixelFormat& outPixelFormat, uint32_t width, uint32_t height, uint32_t* outDesiredNumBackBuffers, std::vector<VkImage>& outImages, int8_t lockToVsync);

	~VulkanSwapChain();

	SwapStatus Present(std::shared_ptr<VulkanQueue> gfxQueue, std::shared_ptr<VulkanQueue> presentQueue, VkSemaphore* complete);

	int32_t AcquireImageIndex(VkSemaphore* outSemaphore);

	inline int8_t DoesLockToVsync()
	{
		return m_LockToVsync;
	}

	inline VkSwapchainKHR GetInstanceHandle()
	{
		return m_SwapChain;
	}

	inline int32_t GetWidth() const
	{
		return m_SwapChainInfo.imageExtent.width;
	}

	inline int32_t GetHeight() const
	{
		return m_SwapChainInfo.imageExtent.height;
	}

	inline int32_t GetBackBufferCount() const
	{
		return m_BackBufferCount;
	}

	inline const VkSwapchainCreateInfoKHR& GetInfo() const
	{
		return m_SwapChainInfo;
	}

	inline VkFormat GetColorFormat() const
	{
		return m_ColorFormat;
	}

private:
	friend class VulkanViewport;

	VkInstance						m_Instance;
	VkSwapchainKHR					m_SwapChain;
	VkSurfaceKHR					m_Surface;
	VkSwapchainCreateInfoKHR		m_SwapChainInfo;
	VkFormat						m_ColorFormat;
	int32_t							m_BackBufferCount;

	std::shared_ptr<VulkanDevice>	m_Device;
	std::vector<VkSemaphore>		m_ImageAcquiredSemaphore;

	int32_t							m_CurrentImageIndex;
	int32_t							m_SemaphoreIndex;
	uint32_t						m_NumPresentCalls;
	uint32_t						m_NumAcquireCalls;
	int8_t							m_LockToVsync;
	uint32_t						m_PresentID;
};