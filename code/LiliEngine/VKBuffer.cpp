#include "stdafx.h"
#include "VKBuffer.h"

VKBuffer::~VKBuffer()
{
	if (m_buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(m_device, m_buffer, nullptr);
		m_buffer = VK_NULL_HANDLE;
	}
	if (m_memory != VK_NULL_HANDLE) 
	{
		vkFreeMemory(m_device, m_memory, nullptr);
		m_memory = VK_NULL_HANDLE;
	}
}