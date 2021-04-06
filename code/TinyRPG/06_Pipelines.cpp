#include "stdafx.h"
#include "06_Pipelines.h"

//-----------------------------------------------------------------------------
Pipelines::Pipelines(Configuration& configuration) noexcept
	: m_configuration(configuration)
	, m_engine(configuration)
{
}
//-----------------------------------------------------------------------------
void Pipelines::StartGame() noexcept
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
bool Pipelines::init() noexcept
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
void Pipelines::update() noexcept
{
}
//-----------------------------------------------------------------------------
void Pipelines::draw() noexcept
{
	Draw(m_engine.GetCurrTime(), m_engine.GetDeltaTime());
}
//-----------------------------------------------------------------------------
void Pipelines::close() noexcept
{
	DestroyAssets();
	DestroyGUI();
	DestroyDescriptorSetLayout();
	DestroyPipelines();
	DestroyUniformBuffers();
}
//-----------------------------------------------------------------------------