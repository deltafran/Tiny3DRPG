#pragma once

class Pipelines final
{
public:
	Pipelines(Configuration& configuration) noexcept;

	void StartGame() noexcept;
private:
	Pipelines() = delete;
	Pipelines(const Pipelines&) = delete;
	Pipelines(Pipelines&&) = delete;
	Pipelines operator=(const Pipelines&) = delete;
	Pipelines operator=(Pipelines&&) = delete;

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

	struct MVPBlock
	{
		Matrix4x4 model;
		Matrix4x4 view;
		Matrix4x4 projection;
	};

	struct ParamBlock
	{
		float intensity;
	};

	void Draw(float time, float delta)
	{
		int32_t bufferIndex = m_defContext->AcquireBackbufferIndex();

		bool hovered = UpdateUI(time, delta);
		if (!hovered)
			m_ViewCamera.Update(time, delta);

		UpdateUniformBuffers(time, delta);

		m_defContext->Present(bufferIndex);
	}

	bool UpdateUI(float time, float delta)
	{
		m_GUI->StartFrame();

		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("Pipelines!", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

			ImGui::Checkbox("AutoRotate", &m_AutoRotate);

			ImGui::SliderFloat("Intensity", &(m_ParamData.intensity), 0.0f, 1.0f);
			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		bool hovered = ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsRootWindowOrAnyChildHovered();

		m_GUI->EndFrame();

		if (m_GUI->Update()) {
			SetupCommandBuffers();
		}

		return hovered;
	}

	void LoadAssets()
	{
		VKCommandBuffer* cmdBuffer = VKCommandBuffer::Create(m_defContext->m_VulkanDevice, m_defContext->m_CommandPool);

		m_Model = VKModel::LoadFromFile(
			"data/models/suzanne.obj",
			m_defContext->m_VulkanDevice,
			cmdBuffer,
			{ VertexAttribute::VA_Position, VertexAttribute::VA_Normal }
		);

		delete cmdBuffer;
	}

	void DestroyAssets()
	{
		delete m_Model;
	}

	void SetupCommandBuffers()
	{
		VkCommandBufferBeginInfo cmdBeginInfo;
		ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);

		VkClearValue clearValues[2];
		clearValues[0].color = { {0.2f, 0.2f, 0.2f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo;
		ZeroVulkanStruct(renderPassBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
		renderPassBeginInfo.renderPass = m_vkContext->m_RenderPass;
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = clearValues;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = m_defContext->m_FrameWidth;
		renderPassBeginInfo.renderArea.extent.height = m_defContext->m_FrameHeight;

		for (int32_t i = 0; i < m_defContext->m_CommandBuffers.size(); ++i)
		{
			renderPassBeginInfo.framebuffer = m_vkContext->m_FrameBuffers[i];

			VERIFYVULKANRESULT(vkBeginCommandBuffer(m_defContext->m_CommandBuffers[i], &cmdBeginInfo));
			vkCmdBeginRenderPass(m_defContext->m_CommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			for (int32_t j = 0; j < 3; ++j)
			{
				int32_t ww = 1.0f / 3 * m_defContext->m_FrameWidth;
				int32_t tx = j * ww;

				VkViewport viewport = {};
				viewport.x = tx;
				viewport.y = m_defContext->m_FrameHeight;
				viewport.width = ww;
				viewport.height = -(float)m_defContext->m_FrameHeight;    // flip y axis
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;

				VkRect2D scissor = {};
				scissor.extent.width = ww;
				scissor.extent.height = m_defContext->m_FrameHeight;
				scissor.offset.x = tx;
				scissor.offset.y = 0;

				vkCmdSetViewport(m_defContext->m_CommandBuffers[i], 0, 1, &viewport);
				vkCmdSetScissor(m_defContext->m_CommandBuffers[i], 0, 1, &scissor);

				vkCmdBindPipeline(m_defContext->m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipelines[j]->pipeline);
				vkCmdBindDescriptorSets(m_defContext->m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipelines[j]->pipelineLayout, 0, 1, &m_DescriptorSet, 0, nullptr);

				for (int32_t meshIndex = 0; meshIndex < m_Model->meshes.size(); ++meshIndex) {
					m_Model->meshes[meshIndex]->BindDrawCmd(m_defContext->m_CommandBuffers[i]);
				}
			}

			m_GUI->BindDrawCmd(m_defContext->m_CommandBuffers[i], m_vkContext->m_RenderPass);

			vkCmdEndRenderPass(m_defContext->m_CommandBuffers[i]);
			VERIFYVULKANRESULT(vkEndCommandBuffer(m_defContext->m_CommandBuffers[i]));
		}
	}

	void CreateDescriptorSet()
	{
		VkDescriptorPoolSize poolSize = {};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = 2;

		VkDescriptorPoolCreateInfo descriptorPoolInfo;
		ZeroVulkanStruct(descriptorPoolInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
		descriptorPoolInfo.poolSizeCount = 1;
		descriptorPoolInfo.pPoolSizes = &poolSize;
		descriptorPoolInfo.maxSets = m_Model->meshes.size();
		VERIFYVULKANRESULT(vkCreateDescriptorPool(m_defContext->m_Device, &descriptorPoolInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorPool));

		VkDescriptorSetAllocateInfo allocInfo;
		ZeroVulkanStruct(allocInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO);
		allocInfo.descriptorPool = m_DescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &m_DescriptorSetLayout;
		VERIFYVULKANRESULT(vkAllocateDescriptorSets(m_defContext->m_Device, &allocInfo, &m_DescriptorSet));

		VkWriteDescriptorSet writeDescriptorSet;
		ZeroVulkanStruct(writeDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
		writeDescriptorSet.dstSet = m_DescriptorSet;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet.pBufferInfo = &(m_MVPBuffer->descriptor);
		writeDescriptorSet.dstBinding = 0;
		vkUpdateDescriptorSets(m_defContext->m_Device, 1, &writeDescriptorSet, 0, nullptr);

		ZeroVulkanStruct(writeDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
		writeDescriptorSet.dstSet = m_DescriptorSet;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet.pBufferInfo = &(m_ParamBuffer->descriptor);
		writeDescriptorSet.dstBinding = 1;
		vkUpdateDescriptorSets(m_defContext->m_Device, 1, &writeDescriptorSet, 0, nullptr);
	}
	
	void CreatePipelines()
	{
		m_Pipelines.resize(3);

		VkVertexInputBindingDescription vertexInputBinding = m_Model->GetInputBinding();
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributs = m_Model->GetInputAttributes();

		VKGfxPipelineInfo pipelineInfo0;
		pipelineInfo0.vertShaderModule = vkutils::LoadSPIPVShader(m_defContext->m_Device, "data/shaders/6_Pipelines/pipeline0.vert.spv");
		pipelineInfo0.fragShaderModule = vkutils::LoadSPIPVShader(m_defContext->m_Device, "data/shaders/6_Pipelines/pipeline0.frag.spv");
		m_Pipelines[0] = VKGfxPipeline::Create(m_vulkanRHI->GetDevice(), m_defContext->m_PipelineCache, pipelineInfo0, { vertexInputBinding }, vertexInputAttributs, m_PipelineLayout, m_vkContext->m_RenderPass);

		VKGfxPipelineInfo pipelineInfo1;
		pipelineInfo1.vertShaderModule = vkutils::LoadSPIPVShader(m_defContext->m_Device, "data/shaders/6_Pipelines/pipeline1.vert.spv");
		pipelineInfo1.fragShaderModule = vkutils::LoadSPIPVShader(m_defContext->m_Device, "data/shaders/6_Pipelines/pipeline1.frag.spv");
		m_Pipelines[1] = VKGfxPipeline::Create(m_vulkanRHI->GetDevice(), m_defContext->m_PipelineCache, pipelineInfo1, { vertexInputBinding }, vertexInputAttributs, m_PipelineLayout, m_vkContext->m_RenderPass);

		VKGfxPipelineInfo pipelineInfo2;
		pipelineInfo2.rasterizationState.polygonMode = VkPolygonMode::VK_POLYGON_MODE_LINE;
		pipelineInfo2.vertShaderModule = vkutils::LoadSPIPVShader(m_defContext->m_Device, "data/shaders/6_Pipelines/pipeline2.vert.spv");
		pipelineInfo2.fragShaderModule = vkutils::LoadSPIPVShader(m_defContext->m_Device, "data/shaders/6_Pipelines/pipeline2.frag.spv");
		m_Pipelines[2] = VKGfxPipeline::Create(m_vulkanRHI->GetDevice(), m_defContext->m_PipelineCache, pipelineInfo2, { vertexInputBinding }, vertexInputAttributs, m_PipelineLayout, m_vkContext->m_RenderPass);

		vkDestroyShaderModule(m_defContext->m_Device, pipelineInfo0.vertShaderModule, VULKAN_CPU_ALLOCATOR);
		vkDestroyShaderModule(m_defContext->m_Device, pipelineInfo0.fragShaderModule, VULKAN_CPU_ALLOCATOR);
		vkDestroyShaderModule(m_defContext->m_Device, pipelineInfo1.vertShaderModule, VULKAN_CPU_ALLOCATOR);
		vkDestroyShaderModule(m_defContext->m_Device, pipelineInfo1.fragShaderModule, VULKAN_CPU_ALLOCATOR);
		vkDestroyShaderModule(m_defContext->m_Device, pipelineInfo2.vertShaderModule, VULKAN_CPU_ALLOCATOR);
		vkDestroyShaderModule(m_defContext->m_Device, pipelineInfo2.fragShaderModule, VULKAN_CPU_ALLOCATOR);
	}

	void DestroyPipelines()
	{
		for (size_t i = 0; i < m_Pipelines.size(); ++i)
			delete m_Pipelines[i];
		m_Pipelines.clear();
	}

	void CreateDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding layoutBindings[2] = { };
		layoutBindings[0].binding = 0;
		layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBindings[0].descriptorCount = 1;
		layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		layoutBindings[0].pImmutableSamplers = nullptr;

		layoutBindings[1].binding = 1;
		layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBindings[1].descriptorCount = 1;
		layoutBindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		layoutBindings[1].pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo descSetLayoutInfo;
		ZeroVulkanStruct(descSetLayoutInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
		descSetLayoutInfo.bindingCount = 2;
		descSetLayoutInfo.pBindings = layoutBindings;
		VERIFYVULKANRESULT(vkCreateDescriptorSetLayout(m_defContext->m_Device, &descSetLayoutInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorSetLayout));

		VkPipelineLayoutCreateInfo pipeLayoutInfo;
		ZeroVulkanStruct(pipeLayoutInfo, VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);
		pipeLayoutInfo.setLayoutCount = 1;
		pipeLayoutInfo.pSetLayouts = &m_DescriptorSetLayout;
		VERIFYVULKANRESULT(vkCreatePipelineLayout(m_defContext->m_Device, &pipeLayoutInfo, VULKAN_CPU_ALLOCATOR, &m_PipelineLayout));
	}

	void DestroyDescriptorSetLayout()
	{
		vkDestroyDescriptorSetLayout(m_defContext->m_Device, m_DescriptorSetLayout, VULKAN_CPU_ALLOCATOR);
		vkDestroyPipelineLayout(m_defContext->m_Device, m_PipelineLayout, VULKAN_CPU_ALLOCATOR);
		vkDestroyDescriptorPool(m_defContext->m_Device, m_DescriptorPool, VULKAN_CPU_ALLOCATOR);
	}

	void UpdateUniformBuffers(float time, float delta)
	{
		if (m_AutoRotate)
			m_MVPData.model.AppendRotation(90.0f * delta, Vector3::UpVector);

		m_MVPData.view = m_ViewCamera.GetView();
		m_MVPData.projection = m_ViewCamera.GetProjection();

		m_MVPBuffer->CopyFrom(&m_MVPData, sizeof(MVPBlock));
		m_ParamBuffer->CopyFrom(&m_ParamData, sizeof(ParamBlock));
	}

	void CreateUniformBuffers()
	{
		VKBoundingBox bounds = m_Model->rootNode->GetBounds();
		Vector3 boundSize = bounds.max - bounds.min;
		Vector3 boundCenter = bounds.min + boundSize * 0.5f;

		m_MVPData.model.AppendRotation(180, Vector3::UpVector);

		m_ViewCamera.Perspective(PI / 4, m_configuration.window.windowWidth / 3.0f, m_configuration.window.windowHeight, 0.1f, 1000.0f);
		m_ViewCamera.SetPosition(boundCenter.x, boundCenter.y, boundCenter.z - boundSize.Size() * 2.0f);
		m_ViewCamera.LookAt(boundCenter);

		m_MVPBuffer = VKBuffer::CreateBuffer(
			m_defContext->m_VulkanDevice,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			sizeof(MVPBlock),
			&(m_MVPData)
		);
		m_MVPBuffer->Map();

		m_ParamData.intensity = 0.125f;
		m_ParamBuffer = VKBuffer::CreateBuffer(
			m_defContext->m_VulkanDevice,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			sizeof(ParamBlock),
			&(m_ParamData)
		);
		m_ParamBuffer->Map();
	}

	void DestroyUniformBuffers()
	{
		m_MVPBuffer->UnMap();
		delete m_MVPBuffer;

		m_ParamBuffer->UnMap();
		delete m_ParamBuffer;
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

	typedef std::vector<VKGfxPipeline*>	DVKPipelines;

	bool							m_AutoRotate = true;
	bool 							m_Ready = false;

	VKCamera						m_ViewCamera;

	MVPBlock 						m_MVPData;
	VKBuffer*						m_MVPBuffer;

	ParamBlock						m_ParamData;
	VKBuffer*						m_ParamBuffer = nullptr;

	DVKPipelines					m_Pipelines;
	
	VKModel* m_Model = nullptr;

	VkDescriptorSetLayout 			m_DescriptorSetLayout = VK_NULL_HANDLE;
	VkPipelineLayout 				m_PipelineLayout = VK_NULL_HANDLE;
	VkDescriptorPool                m_DescriptorPool = VK_NULL_HANDLE;
	VkDescriptorSet 				m_DescriptorSet = VK_NULL_HANDLE;

	ImageGUIContext* m_GUI = nullptr;
};