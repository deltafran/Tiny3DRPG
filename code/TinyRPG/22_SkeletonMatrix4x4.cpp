#include "stdafx.h"
#include "22_SkeletonMatrix4x4.h"

//-----------------------------------------------------------------------------
SkeletonMatrix4x4::SkeletonMatrix4x4(Configuration& configuration) noexcept
	: m_configuration(configuration)
	, m_engine(configuration)
{
}
//-----------------------------------------------------------------------------
void SkeletonMatrix4x4::StartGame() noexcept
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
bool SkeletonMatrix4x4::init() noexcept
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
	InitParmas();
	CreateGUI();

	return true;
}
//-----------------------------------------------------------------------------
void SkeletonMatrix4x4::update() noexcept
{
}
//-----------------------------------------------------------------------------
void SkeletonMatrix4x4::draw() noexcept
{
	Draw(m_engine.GetCurrTime(), m_engine.GetDeltaTime());
}
//-----------------------------------------------------------------------------
void SkeletonMatrix4x4::close() noexcept
{
	DestroyAssets();
	DestroyGUI();
}
//-----------------------------------------------------------------------------