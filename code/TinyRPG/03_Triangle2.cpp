#include "stdafx.h"
#include "03_Triangle2.h"
//-----------------------------------------------------------------------------
Triangle2::Triangle2(Configuration& configuration) noexcept
	: m_configuration(configuration)
	, m_engine(configuration)
{
}
//-----------------------------------------------------------------------------
void Triangle2::StartGame() noexcept
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
bool Triangle2::init() noexcept
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

	CreateGUI();
	CreateMeshBuffers();
	CreateUniformBuffers();
	CreateDescriptorPool();
	CreateDescriptorSetLayout();
	CreateDescriptorSet();
	CreatePipelines();
	SetupCommandBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void Triangle2::update() noexcept
{
}
//-----------------------------------------------------------------------------
void Triangle2::draw() noexcept
{
	Draw(m_engine.GetCurrTime(), m_engine.GetDeltaTime());
}
//-----------------------------------------------------------------------------
void Triangle2::close() noexcept
{
	DestroyGUI();
	DestroyDescriptorSetLayout();
	DestroyDescriptorPool();
	DestroyPipelines();
	DestroyUniformBuffers();
	DestroyMeshBuffers();
}
//-----------------------------------------------------------------------------