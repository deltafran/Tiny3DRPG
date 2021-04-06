#pragma once

class Texture final
{
public:
	Texture(Configuration& configuration) noexcept;

	void StartGame() noexcept;
private:
	Texture() = delete;
	Texture(const Texture&) = delete;
	Texture(Texture&&) = delete;
	Texture operator=(const Texture&) = delete;
	Texture operator=(Texture&&) = delete;

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

	struct MVPBlock
	{
		Matrix4x4 model;
		Matrix4x4 view;
		Matrix4x4 projection;
	};

	struct ParamBlock
	{
		Vector3 lightDir;
		float curvature;

		Vector3 lightColor;
		float exposure;

		Vector2 curvatureScaleBias;
		float blurredLevel;
		float padding;
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
			ImGui::Begin("Texture", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
			ImGui::Text("SSS Demo");

			for (int32_t i = 0; i < m_Model->meshes.size(); ++i)
			{
				VKMesh* mesh = m_Model->meshes[i];
				ImGui::Text("%-20s Tri:%d", mesh->linkNode->name.c_str(), mesh->triangleCount);
			}

			ImGui::SliderFloat("Curvature", &(m_ParamData.curvature), 0.0f, 10.0f);
			ImGui::SliderFloat2("CurvatureBias", (float*)&(m_ParamData.curvatureScaleBias), 0.0f, 1.0f);

			ImGui::SliderFloat("BlurredLevel", &(m_ParamData.blurredLevel), 0.0f, 12.0f);
			ImGui::SliderFloat("Exposure", &(m_ParamData.exposure), 0.0f, 10.0f);

			ImGui::SliderFloat3("LightDirection", (float*)&(m_ParamData.lightDir), -10.0f, 10.0f);
			ImGui::ColorEdit3("LightColor", (float*)&(m_ParamData.lightColor));

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
		VKCommandBuffer* cmdBuffer = VKCommandBuffer::Create(m_VulkanDevice, m_defContext->m_CommandPool);

		m_Model = VKModel::LoadFromFile(
			"data/models/head.obj",
			m_VulkanDevice,
			cmdBuffer,
			{ VertexAttribute::VA_Position, VertexAttribute::VA_UV0, VertexAttribute::VA_Normal, VertexAttribute::VA_Tangent }
		);

		m_TexDiffuse = VKTexture::Create2D("data/textures/head_diffuse.jpg", m_VulkanDevice, cmdBuffer);
		m_TexNormal = VKTexture::Create2D("data/textures/head_normal.jpg", m_VulkanDevice, cmdBuffer);
		m_TexCurvature = VKTexture::Create2D("data/textures/curvatureLUT.png", m_VulkanDevice, cmdBuffer);
		m_TexPreIntegrated = VKTexture::Create2D("data/textures/preIntegratedLUT.png", m_VulkanDevice, cmdBuffer);

		delete cmdBuffer;
	}

	void DestroyAssets()
	{
		delete m_Model;

		delete m_TexDiffuse;
		delete m_TexNormal;
		delete m_TexCurvature;
		delete m_TexPreIntegrated;
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
		renderPassBeginInfo.renderPass = m_RenderPass;
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

			vkCmdSetViewport(m_defContext->m_CommandBuffers[i], 0, 1, &viewport);
			vkCmdSetScissor(m_defContext->m_CommandBuffers[i], 0, 1, &scissor);

			vkCmdBindPipeline(m_defContext->m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->pipeline);
			vkCmdBindDescriptorSets(m_defContext->m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->pipelineLayout, 0, 1, &m_DescriptorSet, 0, nullptr);

			for (int32_t meshIndex = 0; meshIndex < m_Model->meshes.size(); ++meshIndex) {
				m_Model->meshes[meshIndex]->BindDrawCmd(m_defContext->m_CommandBuffers[i]);
			}

			m_GUI->BindDrawCmd(m_defContext->m_CommandBuffers[i], m_RenderPass);

			vkCmdEndRenderPass(m_defContext->m_CommandBuffers[i]);
			VERIFYVULKANRESULT(vkEndCommandBuffer(m_defContext->m_CommandBuffers[i]));
		}
	}

	void CreateDescriptorSet()
	{
		VkDescriptorPoolSize poolSizes[2];
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = 2;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = 4;

		VkDescriptorPoolCreateInfo descriptorPoolInfo;
		ZeroVulkanStruct(descriptorPoolInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
		descriptorPoolInfo.poolSizeCount = 2;
		descriptorPoolInfo.pPoolSizes = poolSizes;
		descriptorPoolInfo.maxSets = 1;
		VERIFYVULKANRESULT(vkCreateDescriptorPool(m_Device, &descriptorPoolInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorPool));

		VkDescriptorSetAllocateInfo allocInfo;
		ZeroVulkanStruct(allocInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO);
		allocInfo.descriptorPool = m_DescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &m_DescriptorSetLayout;
		VERIFYVULKANRESULT(vkAllocateDescriptorSets(m_Device, &allocInfo, &m_DescriptorSet));

		VkWriteDescriptorSet writeDescriptorSet;

		ZeroVulkanStruct(writeDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
		writeDescriptorSet.dstSet = m_DescriptorSet;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet.pBufferInfo = &(m_MVPBuffer->descriptor);
		writeDescriptorSet.dstBinding = 0;
		vkUpdateDescriptorSets(m_Device, 1, &writeDescriptorSet, 0, nullptr);

		ZeroVulkanStruct(writeDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
		writeDescriptorSet.dstSet = m_DescriptorSet;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet.pBufferInfo = &(m_ParamBuffer->descriptor);
		writeDescriptorSet.dstBinding = 1;
		vkUpdateDescriptorSets(m_Device, 1, &writeDescriptorSet, 0, nullptr);

		std::vector<VKTexture*> textures = { m_TexDiffuse, m_TexNormal, m_TexCurvature, m_TexPreIntegrated };
		for (int32_t i = 0; i < 4; ++i)
		{
			ZeroVulkanStruct(writeDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
			writeDescriptorSet.dstSet = m_DescriptorSet;
			writeDescriptorSet.descriptorCount = 1;
			writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writeDescriptorSet.pBufferInfo = nullptr;
			writeDescriptorSet.pImageInfo = &(textures[i]->descriptorInfo);
			writeDescriptorSet.dstBinding = 2 + i;
			vkUpdateDescriptorSets(m_Device, 1, &writeDescriptorSet, 0, nullptr);
		}
	}

	void CreatePipelines()
	{
		VkVertexInputBindingDescription vertexInputBinding = m_Model->GetInputBinding();
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributs = m_Model->GetInputAttributes();

		VKGfxPipelineInfo pipelineInfo;
		pipelineInfo.vertShaderModule = vkutils::LoadSPIPVShader(m_Device, "data/shaders/7_Texture/texture.vert.spv");
		pipelineInfo.fragShaderModule = vkutils::LoadSPIPVShader(m_Device, "data/shaders/7_Texture/texture.frag.spv");
		m_Pipeline = VKGfxPipeline::Create(m_VulkanDevice, m_PipelineCache, pipelineInfo, { vertexInputBinding }, vertexInputAttributs, m_PipelineLayout, m_RenderPass);

		vkDestroyShaderModule(m_Device, pipelineInfo.vertShaderModule, VULKAN_CPU_ALLOCATOR);
		vkDestroyShaderModule(m_Device, pipelineInfo.fragShaderModule, VULKAN_CPU_ALLOCATOR);
	}

	void DestroyPipelines()
	{
		delete m_Pipeline;
		m_Pipeline = nullptr;
	}

	void CreateDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding layoutBindings[6] = { };
		layoutBindings[0].binding = 0;
		layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBindings[0].descriptorCount = 1;
		layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		layoutBindings[0].pImmutableSamplers = nullptr;

		layoutBindings[1].binding = 1;
		layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBindings[1].descriptorCount = 1;
		layoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutBindings[1].pImmutableSamplers = nullptr;

		for (int32_t i = 0; i < 4; ++i)
		{
			layoutBindings[2 + i].binding = 2 + i;
			layoutBindings[2 + i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			layoutBindings[2 + i].descriptorCount = 1;
			layoutBindings[2 + i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			layoutBindings[2 + i].pImmutableSamplers = nullptr;
		}

		VkDescriptorSetLayoutCreateInfo descSetLayoutInfo;
		ZeroVulkanStruct(descSetLayoutInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
		descSetLayoutInfo.bindingCount = 6;
		descSetLayoutInfo.pBindings = layoutBindings;
		VERIFYVULKANRESULT(vkCreateDescriptorSetLayout(m_Device, &descSetLayoutInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorSetLayout));

		VkPipelineLayoutCreateInfo pipeLayoutInfo;
		ZeroVulkanStruct(pipeLayoutInfo, VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);
		pipeLayoutInfo.setLayoutCount = 1;
		pipeLayoutInfo.pSetLayouts = &m_DescriptorSetLayout;
		VERIFYVULKANRESULT(vkCreatePipelineLayout(m_Device, &pipeLayoutInfo, VULKAN_CPU_ALLOCATOR, &m_PipelineLayout));
	}

	void DestroyDescriptorSetLayout()
	{
		vkDestroyDescriptorSetLayout(m_Device, m_DescriptorSetLayout, VULKAN_CPU_ALLOCATOR);
		vkDestroyPipelineLayout(m_Device, m_PipelineLayout, VULKAN_CPU_ALLOCATOR);
		vkDestroyDescriptorPool(m_Device, m_DescriptorPool, VULKAN_CPU_ALLOCATOR);
	}

	void UpdateUniformBuffers(float time, float delta)
	{
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

		m_ViewCamera.Perspective(PI / 4, m_configuration.window.windowWidth, m_configuration.window.windowHeight, 0.1f, 1000.0f);
		m_ViewCamera.SetPosition(boundCenter.x, boundCenter.y, boundCenter.z - boundSize.Size() * 1.0f);
		m_ViewCamera.LookAt(boundCenter);

		m_MVPBuffer = VKBuffer::CreateBuffer(
			m_VulkanDevice,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			sizeof(MVPBlock),
			&(m_MVPData)
		);
		m_MVPBuffer->Map();

		m_ParamData.blurredLevel = 2.0;
		m_ParamData.curvature = 3.5;
		m_ParamData.curvatureScaleBias.x = 0.101;
		m_ParamData.curvatureScaleBias.y = -0.001;
		m_ParamData.exposure = 1.0;
		m_ParamData.lightColor.Set(240.0f / 255.0f, 200.0f / 255.0f, 166.0f / 255.0f);
		m_ParamData.lightDir.Set(1, 0, -1.0);
		m_ParamData.lightDir.Normalize();
		m_ParamData.padding = 0.0;
		m_ParamBuffer = VKBuffer::CreateBuffer(
			m_VulkanDevice,
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
		m_MVPBuffer = nullptr;

		m_ParamBuffer->UnMap();
		delete m_ParamBuffer;
		m_ParamBuffer = nullptr;
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

	VKTexture*						m_TexDiffuse = nullptr;
	VKTexture*						m_TexNormal = nullptr;
	VKTexture*						m_TexCurvature = nullptr;
	VKTexture*						m_TexPreIntegrated = nullptr;

	VKGfxPipeline*					m_Pipeline = nullptr;

	VKModel*						m_Model = nullptr;

	VkDescriptorSetLayout 			m_DescriptorSetLayout = VK_NULL_HANDLE;
	VkPipelineLayout 				m_PipelineLayout = VK_NULL_HANDLE;
	VkDescriptorPool                m_DescriptorPool = VK_NULL_HANDLE;
	VkDescriptorSet 				m_DescriptorSet = VK_NULL_HANDLE;

	ImageGUIContext*				m_GUI = nullptr;
};