#include "stdafx.h"
#include "GameApplication.h"
//-----------------------------------------------------------------------------
GameApplication::GameApplication(Configuration &configuration) noexcept
	: m_configuration(configuration)
	, m_engine(configuration)
{
}
//-----------------------------------------------------------------------------
void GameApplication::StartGame() noexcept
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
bool GameApplication::init() noexcept
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

	CreateSemaphores();
	CreateFences();
	CreateCommandBuffers();
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
void GameApplication::update() noexcept
{	
}
//-----------------------------------------------------------------------------
void GameApplication::draw() noexcept
{
	UpdateUniformBuffers(m_engine.GetCurrTime(), m_engine.GetDeltaTime());

	VkQueue queue = m_vulkanRHI->GetDevice()->GetPresentQueue()->GetHandle();
	VkDevice device = m_vulkanRHI->GetDevice()->GetInstanceHandle();
	int32_t backBufferIndex = m_vulkanRHI->GetSwapChain()->AcquireImageIndex(&m_PresentComplete);

	VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pWaitDstStageMask = &waitStageMask;
	submitInfo.pWaitSemaphores = &m_PresentComplete;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &m_RenderComplete;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pCommandBuffers = &(m_CommandBuffers[backBufferIndex]);
	submitInfo.commandBufferCount = 1;

	vkResetFences(device, 1, &(m_Fences[backBufferIndex]));
	VERIFYVULKANRESULT(vkQueueSubmit(queue, 1, &submitInfo, m_Fences[backBufferIndex]));
	vkWaitForFences(device, 1, &(m_Fences[backBufferIndex]), true, 200 * 1000 * 1000);

	// present
	m_vulkanRHI->GetSwapChain()->Present(
		m_vulkanRHI->GetDevice()->GetGraphicsQueue(),
		m_vulkanRHI->GetDevice()->GetPresentQueue(),
		&m_RenderComplete
	);
}
//-----------------------------------------------------------------------------
void GameApplication::close() noexcept
{
	DestroyCommandBuffers();
	DestroyDescriptorSetLayout();
	DestroyDescriptorPool();
	DestroyPipelines();
	DestroyUniformBuffers();
	DestroyMeshBuffers();
	DestorySemaphores();
	DestroyFences();
}
//-----------------------------------------------------------------------------