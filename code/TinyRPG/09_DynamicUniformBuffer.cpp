#include "stdafx.h"
#include "09_DynamicUniformBuffer.h"

//-----------------------------------------------------------------------------
DynamicUniformBuffer::DynamicUniformBuffer(Configuration& configuration) noexcept
	: m_configuration(configuration)
	, m_engine(configuration)
{
}
//-----------------------------------------------------------------------------
void DynamicUniformBuffer::StartGame() noexcept
{
	if (init())
	{
		while (!isEnd())
		{
			m_engine.Update();
			update();
			m_engine.BeginFrame();
			draw();
			m_engine.EndFrame();
		}
		close();
		m_engine.Close();
		Log::Close();
	}
}
//-----------------------------------------------------------------------------
bool DynamicUniformBuffer::init() noexcept
{
	if (!m_configuration.logFileName.empty())
	{
		if (!Log::Open(m_configuration.logFileName))
			return false;
	}

	Log::Message("Start Lili Engine");

	if (!m_engine.Init())
		return false;

	m_vulkanRHI = &m_engine.GetRendererSystem().GetVulkanRHI();

	m_vkContext = &m_engine.GetRendererSystem().GetVulkanContext();

	m_VulkanDevice = m_vkContext->m_VulkanDevice;
	m_Device = m_vkContext->m_Device;
	m_PipelineCache = m_vkContext->m_PipelineCache;
	m_RenderPass = m_vkContext->m_RenderPass;

	LoadAssets();
	CreateGUI();
	CreateUniformBuffers();
	CreateDescriptorSetLayout();
	CreateDescriptorSet();
	CreatePipelines();
	SetupCommandBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void DynamicUniformBuffer::update() noexcept
{
}
//-----------------------------------------------------------------------------
void DynamicUniformBuffer::draw() noexcept
{
	Draw(m_engine.GetCurrTime(), m_engine.GetDeltaTime());
}
//-----------------------------------------------------------------------------
void DynamicUniformBuffer::close() noexcept
{
	DestroyAssets();
	DestroyGUI();
	DestroyDescriptorSetLayout();
	DestroyPipelines();
	DestroyUniformBuffers();
}
//-----------------------------------------------------------------------------