#include "stdafx.h"
#include "11_Texture3D.h"

//-----------------------------------------------------------------------------
Texture3D::Texture3D(Configuration& configuration) noexcept
	: m_configuration(configuration)
	, m_engine(configuration)
{
}
//-----------------------------------------------------------------------------
void Texture3D::StartGame() noexcept
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
bool Texture3D::init() noexcept
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
	CreateDescriptorSetLayout();
	CreateDescriptorSet();
	CreatePipelines();
	SetupCommandBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void Texture3D::update() noexcept
{
}
//-----------------------------------------------------------------------------
void Texture3D::draw() noexcept
{
	Draw(m_engine.GetCurrTime(), m_engine.GetDeltaTime());
}
//-----------------------------------------------------------------------------
void Texture3D::close() noexcept
{
	DestroyAssets();
	DestroyGUI();
	DestroyDescriptorSetLayout();
	DestroyPipelines();
	DestroyUniformBuffers();
}
//-----------------------------------------------------------------------------