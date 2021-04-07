#include "stdafx.h"
#include "12_OptimizeShaderAndLayout.h"

//-----------------------------------------------------------------------------
OptimizeShaderAndLayout::OptimizeShaderAndLayout(Configuration& configuration) noexcept
	: m_configuration(configuration)
	, m_engine(configuration)
{
}
//-----------------------------------------------------------------------------
void OptimizeShaderAndLayout::StartGame() noexcept
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
bool OptimizeShaderAndLayout::init() noexcept
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
	m_defContext = &m_engine.GetRendererSystem().GetDefaultVulkanContext();
	m_vkContext = &m_engine.GetRendererSystem().GetVulkanContext();

	m_VulkanDevice = m_defContext->m_VulkanDevice;
	m_Device = m_defContext->m_Device;
	m_PipelineCache = m_defContext->m_PipelineCache;
	m_RenderPass = m_vkContext->m_RenderPass;

	LoadAssets();
	CreateGUI();
	CreateUniformBuffers();
	CreateDescriptorSet();
	CreatePipelines();
	SetupCommandBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void OptimizeShaderAndLayout::update() noexcept
{
}
//-----------------------------------------------------------------------------
void OptimizeShaderAndLayout::draw() noexcept
{
	Draw(m_engine.GetCurrTime(), m_engine.GetDeltaTime());
}
//-----------------------------------------------------------------------------
void OptimizeShaderAndLayout::close() noexcept
{
	DestroyAssets();
	DestroyGUI();
	DestroyPipelines();
	DestroyUniformBuffers();
}
//-----------------------------------------------------------------------------