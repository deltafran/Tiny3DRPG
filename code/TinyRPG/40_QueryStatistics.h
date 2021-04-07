#pragma once

#pragma once

#define QUERY_STATS_COUNT 8

class QueryStatistics final
{
public:
	QueryStatistics(Configuration& configuration) noexcept;

	void StartGame() noexcept;
private:
	QueryStatistics() = delete;
	QueryStatistics(const QueryStatistics&) = delete;
	QueryStatistics(QueryStatistics&&) = delete;
	QueryStatistics operator=(const QueryStatistics&) = delete;
	QueryStatistics operator=(QueryStatistics&&) = delete;

	bool init() noexcept;
	void update() noexcept;
	void draw() noexcept;
	void close() noexcept;
	bool isEnd() const noexcept { return m_isEnd || m_engine.IsEnd(); }

	Configuration& m_configuration;
	Engine m_engine;
	bool m_isEnd = false;

	VulkanRHI* m_vulkanRHI = nullptr;
	DefaultVulkanContext* m_defContext = nullptr;
	VulkanContext* m_vkContext = nullptr;

	std::shared_ptr<VulkanDevice> m_VulkanDevice = nullptr;
	VkDevice m_Device = VK_NULL_HANDLE;
	VkPipelineCache m_PipelineCache = VK_NULL_HANDLE;
	VkRenderPass m_RenderPass = VK_NULL_HANDLE;

	struct ModelViewProjectionBlock
	{
		Matrix4x4 model;
		Matrix4x4 view;
		Matrix4x4 proj;
	};

	void Draw(float time, float delta)
	{
		int32_t bufferIndex = m_defContext->AcquireBackbufferIndex();

		m_defContext->UpdateFPS(time, delta);
		bool hovered = UpdateUI(time, delta);
		if (!hovered) {
			m_ViewCamera.Update(time, delta);
		}

		vkGetQueryPoolResults(
			m_Device,
			m_QueryPool,
			0, 1, sizeof(uint64_t) * QUERY_STATS_COUNT, m_QueryStats, sizeof(uint64_t),
			VK_QUERY_RESULT_64_BIT
		);

		SetupCommandBuffers(bufferIndex);

		m_defContext->Present(bufferIndex);
	}

	bool UpdateUI(float time, float delta)
	{
		m_GUI->StartFrame();

		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("QueryStatisticsDemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

			for (int32_t i = 0; i < m_StatNames.size(); ++i)
			{
				ImGui::Text("%s : %d", m_StatNames[i], m_QueryStats[i]);
			}

			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / m_defContext->m_LastFPS, m_defContext->m_LastFPS);
			ImGui::End();
		}

		bool hovered = ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsRootWindowOrAnyChildHovered();

		m_GUI->EndFrame();
		m_GUI->Update();

		return hovered;
	}

	void LoadAssets()
	{
		VKCommandBuffer* cmdBuffer = VKCommandBuffer::Create(m_VulkanDevice, m_defContext->m_CommandPool);

		VkQueryPoolCreateInfo queryPoolCreateInfo;
		ZeroVulkanStruct(queryPoolCreateInfo, VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO);
		queryPoolCreateInfo.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
		queryPoolCreateInfo.queryCount = QUERY_STATS_COUNT;
		queryPoolCreateInfo.pipelineStatistics =
			VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT |
			VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT |
			VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT |
			VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT |
			VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT |
			VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT |
			VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT |
			VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT;
		VERIFYVULKANRESULT(vkCreateQueryPool(m_Device, &queryPoolCreateInfo, VULKAN_CPU_ALLOCATOR, &m_QueryPool));

		m_StatNames.resize(QUERY_STATS_COUNT);
		m_StatNames[0] = "Vertex count";
		m_StatNames[1] = "Primitives count";
		m_StatNames[2] = "Vert shader invocations";
		m_StatNames[3] = "Clipping invocations";
		m_StatNames[4] = "Clipping primtives";
		m_StatNames[5] = "Frag shader invocations";
		m_StatNames[6] = "Tessellation control shader patches";
		m_StatNames[7] = "Tessellation evaluation shader invocations";

		m_Model = VKModel::LoadFromFile(
			"data/models/Room/miniHouse_FBX.FBX",
			m_VulkanDevice,
			cmdBuffer,
			{
				VertexAttribute::VA_Position,
				VertexAttribute::VA_Normal
			}
		);

		m_Shader = VKShader::Create(
			m_VulkanDevice,
			true,
			"data/shaders/40_QueryStatistics/Solid.vert.spv",
			"data/shaders/40_QueryStatistics/Solid.frag.spv"
		);

		m_Material = VKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_Shader
		);
		m_Material->PreparePipeline();

		delete cmdBuffer;
	}

	void DestroyAssets()
	{
		delete m_Model;

		delete m_Material;
		delete m_Shader;

		vkDestroyQueryPool(m_Device, m_QueryPool, VULKAN_CPU_ALLOCATOR);
	}

	void SetupCommandBuffers(int32_t backBufferIndex)
	{
		VkViewport viewport = {};
		viewport.x = 0;
		viewport.y = m_defContext->m_FrameHeight;
		viewport.width = m_defContext->m_FrameWidth;
		viewport.height = -(float)m_defContext->m_FrameHeight;    // flip y axis
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.extent.width = m_defContext->m_FrameWidth;
		scissor.extent.height = m_defContext->m_FrameHeight;
		scissor.offset.x = 0;
		scissor.offset.y = 0;

		VkCommandBuffer commandBuffer = m_defContext->m_CommandBuffers[backBufferIndex];

		VkCommandBufferBeginInfo cmdBeginInfo;
		ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
		VERIFYVULKANRESULT(vkBeginCommandBuffer(commandBuffer, &cmdBeginInfo));

		vkCmdResetQueryPool(commandBuffer, m_QueryPool, 0, QUERY_STATS_COUNT);

		VkClearValue clearValues[2];
		clearValues[0].color = { { 0.2f, 0.2f, 0.4f, 1.0f } };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo;
		ZeroVulkanStruct(renderPassBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
		renderPassBeginInfo.renderPass = m_RenderPass;
		renderPassBeginInfo.framebuffer = m_vkContext->m_FrameBuffers[backBufferIndex];
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = clearValues;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = m_defContext->m_FrameWidth;
		renderPassBeginInfo.renderArea.extent.height = m_defContext->m_FrameHeight;
		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		vkCmdBeginQuery(commandBuffer, m_QueryPool, 0, 0);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Material->GetPipeline());

		m_Material->BeginFrame();
		for (int32_t i = 0; i < m_Model->meshes.size(); ++i)
		{
			m_MVPParam.model = m_Model->meshes[i]->linkNode->GetGlobalMatrix();
			m_MVPParam.view = m_ViewCamera.GetView();
			m_MVPParam.proj = m_ViewCamera.GetProjection();

			m_Material->BeginObject();
			m_Material->SetLocalUniform("uboMVP", &m_MVPParam, sizeof(ModelViewProjectionBlock));
			m_Material->EndObject();

			m_Material->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, i);
			m_Model->meshes[i]->BindDrawCmd(commandBuffer);
		}
		m_Material->EndFrame();
		vkCmdEndQuery(commandBuffer, m_QueryPool, 0);

		m_GUI->BindDrawCmd(commandBuffer, m_RenderPass);
		vkCmdEndRenderPass(commandBuffer);

		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));
	}

	void InitParmas()
	{
		m_ViewCamera.SetPosition(0, 500, -700.0f);
		m_ViewCamera.LookAt(0, 250, 0);
		m_ViewCamera.Perspective(PI / 4, m_configuration.window.windowWidth, m_configuration.window.windowHeight, 1.0f, 1500.0f);

		memset(m_QueryStats, 65535, sizeof(uint64_t) * QUERY_STATS_COUNT);
	}

	void CreateGUI()
	{
		m_GUI = new ImageGUIContext();
		m_GUI->Init("data/fonts/Ubuntu-Regular.ttf", m_vulkanRHI->GetDevice(), m_configuration.window.windowWidth, m_configuration.window.windowHeight, m_vulkanRHI->GetSwapChain()->GetWidth(), m_vulkanRHI->GetSwapChain()->GetHeight());
	}

	void DestroyGUI()
	{
		m_GUI->Destroy();
		delete m_GUI;
	}

	bool 							m_Ready = false;
	VkQueryPool					m_QueryPool;

	VKModel* m_Model = nullptr;
	VKMaterial* m_Material = nullptr;
	VKShader* m_Shader = nullptr;

	VKCamera		    m_ViewCamera;

	uint64_t					m_QueryStats[QUERY_STATS_COUNT];
	std::vector<const char*>	m_StatNames;
	ModelViewProjectionBlock	m_MVPParam;

	ImageGUIContext* m_GUI = nullptr;
};