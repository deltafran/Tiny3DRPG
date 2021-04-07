#pragma once

// TODO: не работает (в том числе в примере), но работает следующий пример делающий тоже самое

class Texture3D final
{
public:
	Texture3D(Configuration& configuration) noexcept;

	void StartGame() noexcept;
private:
	Texture3D() = delete;
	Texture3D(const Texture3D&) = delete;
	Texture3D(Texture3D&&) = delete;
	Texture3D operator=(const Texture3D&) = delete;
	Texture3D operator=(Texture3D&&) = delete;

	bool init() noexcept;
	void update() noexcept;
	void draw() noexcept;
	void close() noexcept;
	bool isEnd() const noexcept { return m_isEnd || m_engine.IsEnd(); }

	Configuration& m_configuration;
	Engine m_engine;
	bool m_isEnd = false;

	VulkanRHI* m_vulkanRHI = nullptr;

	VulkanContext* m_vkContext = nullptr;

	std::shared_ptr<VulkanDevice> m_VulkanDevice = nullptr;
	VkDevice m_Device = VK_NULL_HANDLE;
	VkPipelineCache m_PipelineCache = VK_NULL_HANDLE;
	VkRenderPass m_RenderPass = VK_NULL_HANDLE;

	struct ImageInfo
	{
		int32_t	width = 0;
		int32_t	height = 0;
		int32_t	comp = 0;
		uint8_t* data = nullptr;
	};

	struct LutDebugBlock
	{
		float bias;
		float padding0;
		float padding1;
		float padding2;
	};

	struct MVPBlock
	{
		Matrix4x4 model;
		Matrix4x4 view;
		Matrix4x4 projection;
	};

	void Draw(float time, float delta)
	{
		int32_t bufferIndex = m_vkContext->AcquireBackbufferIndex();

		bool hovered = UpdateUI(time, delta);
		if (!hovered)
			m_ViewCamera.Update(time, delta);

		UpdateUniformBuffers(time, delta);

		m_vkContext->Present(bufferIndex);
	}

	bool UpdateUI(float time, float delta)
	{
		m_GUI->StartFrame();

		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("Texture3D", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
			ImGui::Text("3D LUT");

			ImGui::SliderFloat("DebugLut", &m_LutDebugData.bias, 0.0f, 1.0f);

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
		VKCommandBuffer* cmdBuffer = VKCommandBuffer::Create(m_VulkanDevice, m_vkContext->m_CommandPool);

		m_Model = VKModel::LoadFromFile(
			"data/models/plane_z.obj",
			m_VulkanDevice,
			cmdBuffer,
			{
				VertexAttribute::VA_Position,
				VertexAttribute::VA_UV0
			}
		);

		// 64mb 
		// map image0 -> image1
		int32_t lutSize = 256;
		uint8_t* lutRGBA = new uint8_t[lutSize * lutSize * 4 * lutSize];
		for (int32_t x = 0; x < lutSize; ++x)
		{
			for (int32_t y = 0; y < lutSize; ++y)
			{
				for (int32_t z = 0; z < lutSize; ++z)
				{
					int idx = (x + y * lutSize + z * lutSize * lutSize) * 4;
					int32_t r = x * 1.0f / (lutSize - 1) * 255;
					int32_t g = y * 1.0f / (lutSize - 1) * 255;
					int32_t b = z * 1.0f / (lutSize - 1) * 255;
					r = 0.393f * r + 0.769f * g + 0.189f * b;
					g = 0.349f * r + 0.686f * g + 0.168f * b;
					b = 0.272f * r + 0.534f * g + 0.131f * b;
					lutRGBA[idx + 0] = math::Min(r, 255);
					lutRGBA[idx + 1] = math::Min(g, 255);
					lutRGBA[idx + 2] = math::Min(b, 255);
					lutRGBA[idx + 3] = 255;
				}
			}
		}

		m_TexOrigin = VKTexture::Create2D("data/textures/game0.jpg", m_VulkanDevice, cmdBuffer);
		m_Tex3DLut = VKTexture::Create3D(VK_FORMAT_R8G8B8A8_UNORM, lutRGBA, lutSize * lutSize * 4 * lutSize, lutSize, lutSize, lutSize, m_VulkanDevice, cmdBuffer);

		delete cmdBuffer;
	}

	void DestroyAssets()
	{
		delete m_Model;
		delete m_TexOrigin;
		delete m_Tex3DLut;
	}

	void SetupCommandBuffers()
	{
		VkCommandBufferBeginInfo cmdBeginInfo;
		ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);

		VkClearValue clearValues[2];
		clearValues[0].color = { {0.2f, 0.2f, 0.4f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo;
		ZeroVulkanStruct(renderPassBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
		renderPassBeginInfo.renderPass = m_RenderPass;
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = clearValues;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = m_vkContext->m_FrameWidth;
		renderPassBeginInfo.renderArea.extent.height = m_vkContext->m_FrameHeight;

		int32_t ww = m_vkContext->m_FrameWidth / 2.0f;
		int32_t hh = m_vkContext->m_FrameHeight / 2.0f;

		VkViewport viewport = {};
		viewport.x = 0;
		viewport.y = hh;
		viewport.width = ww;
		viewport.height = -(float)hh;    // flip y axis
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.extent.width = ww;
		scissor.extent.height = hh;
		scissor.offset.x = 0;
		scissor.offset.y = 0;

		for (int32_t i = 0; i < m_vkContext->m_CommandBuffers.size(); ++i)
		{
			renderPassBeginInfo.framebuffer = m_vkContext->m_FrameBuffers[i];

			VERIFYVULKANRESULT(vkBeginCommandBuffer(m_vkContext->m_CommandBuffers[i], &cmdBeginInfo));
			vkCmdBeginRenderPass(m_vkContext->m_CommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			// 0
			viewport.x = 0;
			viewport.y = hh;
			scissor.offset.x = 0;
			scissor.offset.y = 0;
			vkCmdSetViewport(m_vkContext->m_CommandBuffers[i], 0, 1, &viewport);
			vkCmdSetScissor(m_vkContext->m_CommandBuffers[i], 0, 1, &scissor);
			vkCmdBindPipeline(m_vkContext->m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline0->pipeline);
			vkCmdBindDescriptorSets(m_vkContext->m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline0->pipelineLayout, 0, 1, &m_DescriptorSet0, 0, nullptr);
			for (int32_t meshIndex = 0; meshIndex < m_Model->meshes.size(); ++meshIndex) {
				m_Model->meshes[meshIndex]->BindDrawCmd(m_vkContext->m_CommandBuffers[i]);
			}

			// 1
			viewport.x = ww;
			viewport.y = hh;
			scissor.offset.x = ww;
			scissor.offset.y = 0;
			vkCmdSetViewport(m_vkContext->m_CommandBuffers[i], 0, 1, &viewport);
			vkCmdSetScissor(m_vkContext->m_CommandBuffers[i], 0, 1, &scissor);
			vkCmdBindPipeline(m_vkContext->m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline1->pipeline);
			vkCmdBindDescriptorSets(m_vkContext->m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline1->pipelineLayout, 0, 1, &m_DescriptorSet1, 0, nullptr);
			for (int32_t meshIndex = 0; meshIndex < m_Model->meshes.size(); ++meshIndex) {
				m_Model->meshes[meshIndex]->BindDrawCmd(m_vkContext->m_CommandBuffers[i]);
			}

			// 2
			viewport.x = 0;
			viewport.y = hh * 2;
			scissor.offset.x = 0;
			scissor.offset.y = hh;
			vkCmdSetViewport(m_vkContext->m_CommandBuffers[i], 0, 1, &viewport);
			vkCmdSetScissor(m_vkContext->m_CommandBuffers[i], 0, 1, &scissor);
			vkCmdBindPipeline(m_vkContext->m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline2->pipeline);
			vkCmdBindDescriptorSets(m_vkContext->m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline2->pipelineLayout, 0, 1, &m_DescriptorSet2, 0, nullptr);
			for (int32_t meshIndex = 0; meshIndex < m_Model->meshes.size(); ++meshIndex) {
				m_Model->meshes[meshIndex]->BindDrawCmd(m_vkContext->m_CommandBuffers[i]);
			}

			// 3
			viewport.x = ww;
			viewport.y = hh * 2;
			scissor.offset.x = ww;
			scissor.offset.y = hh;
			vkCmdSetViewport(m_vkContext->m_CommandBuffers[i], 0, 1, &viewport);
			vkCmdSetScissor(m_vkContext->m_CommandBuffers[i], 0, 1, &scissor);
			vkCmdBindPipeline(m_vkContext->m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline3->pipeline);
			vkCmdBindDescriptorSets(m_vkContext->m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline3->pipelineLayout, 0, 1, &m_DescriptorSet3, 0, nullptr);
			for (int32_t meshIndex = 0; meshIndex < m_Model->meshes.size(); ++meshIndex) {
				m_Model->meshes[meshIndex]->BindDrawCmd(m_vkContext->m_CommandBuffers[i]);
			}

			m_GUI->BindDrawCmd(m_vkContext->m_CommandBuffers[i], m_RenderPass);

			vkCmdEndRenderPass(m_vkContext->m_CommandBuffers[i]);
			VERIFYVULKANRESULT(vkEndCommandBuffer(m_vkContext->m_CommandBuffers[i]));
		}
	}

	void CreateDescriptorSet()
	{
		VkDescriptorPoolSize poolSizes[2];
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = 2;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = 2;

		VkDescriptorPoolCreateInfo descriptorPoolInfo;
		ZeroVulkanStruct(descriptorPoolInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
		descriptorPoolInfo.poolSizeCount = 2;
		descriptorPoolInfo.pPoolSizes = poolSizes;
		descriptorPoolInfo.maxSets = 4;
		VERIFYVULKANRESULT(vkCreateDescriptorPool(m_Device, &descriptorPoolInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorPool));

		std::vector<VkDescriptorSet*> descriptorSets = {
			&m_DescriptorSet0,
			&m_DescriptorSet1,
			&m_DescriptorSet2,
			&m_DescriptorSet3
		};
		std::vector<VKTexture*> textures = {
			m_TexOrigin,
			m_TexOrigin,
			m_TexOrigin,
			m_TexOrigin
		};

		for (int32_t i = 0; i < descriptorSets.size(); ++i)
		{
			VkDescriptorSetAllocateInfo allocInfo;
			ZeroVulkanStruct(allocInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO);
			allocInfo.descriptorPool = m_DescriptorPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = &m_DescriptorSetLayout;
			VERIFYVULKANRESULT(vkAllocateDescriptorSets(m_Device, &allocInfo, descriptorSets[i]));

			VkWriteDescriptorSet writeDescriptorSet;
			ZeroVulkanStruct(writeDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
			writeDescriptorSet.dstSet = *(descriptorSets[i]);
			writeDescriptorSet.descriptorCount = 1;
			writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescriptorSet.pBufferInfo = &(m_MVPBuffer->descriptor);
			writeDescriptorSet.dstBinding = 0;
			vkUpdateDescriptorSets(m_Device, 1, &writeDescriptorSet, 0, nullptr);

			ZeroVulkanStruct(writeDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
			writeDescriptorSet.dstSet = *(descriptorSets[i]);
			writeDescriptorSet.descriptorCount = 1;
			writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writeDescriptorSet.pBufferInfo = nullptr;
			writeDescriptorSet.pImageInfo = &(textures[i]->descriptorInfo);
			writeDescriptorSet.dstBinding = 1;
			vkUpdateDescriptorSets(m_Device, 1, &writeDescriptorSet, 0, nullptr);

			ZeroVulkanStruct(writeDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
			writeDescriptorSet.dstSet = *(descriptorSets[i]);
			writeDescriptorSet.descriptorCount = 1;
			writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writeDescriptorSet.pBufferInfo = nullptr;
			writeDescriptorSet.pImageInfo = &(m_Tex3DLut->descriptorInfo);
			writeDescriptorSet.dstBinding = 2;
			vkUpdateDescriptorSets(m_Device, 1, &writeDescriptorSet, 0, nullptr);

			ZeroVulkanStruct(writeDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
			writeDescriptorSet.dstSet = *(descriptorSets[i]);
			writeDescriptorSet.descriptorCount = 1;
			writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescriptorSet.pBufferInfo = &(m_LutDebugBuffer->descriptor);
			writeDescriptorSet.dstBinding = 3;
			vkUpdateDescriptorSets(m_Device, 1, &writeDescriptorSet, 0, nullptr);
		}
	}

	void CreatePipelines()
	{
		VkVertexInputBindingDescription vertexInputBinding = m_Model->GetInputBinding();
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributs = m_Model->GetInputAttributes();

		VKGfxPipelineInfo pipelineInfo0;
		pipelineInfo0.vertShaderModule = vkutils::LoadSPIPVShader(m_Device, "data/shaders/11_Texture3D/texture.vert.spv");
		pipelineInfo0.fragShaderModule = vkutils::LoadSPIPVShader(m_Device, "data/shaders/11_Texture3D/texture.frag.spv");
		m_Pipeline0 = VKGfxPipeline::Create(m_VulkanDevice, m_PipelineCache, pipelineInfo0, { vertexInputBinding }, vertexInputAttributs, m_PipelineLayout, m_RenderPass);

		VKGfxPipelineInfo pipelineInfo1;
		pipelineInfo1.vertShaderModule = vkutils::LoadSPIPVShader(m_Device, "data/shaders/11_Texture3D/lut.vert.spv");
		pipelineInfo1.fragShaderModule = vkutils::LoadSPIPVShader(m_Device, "data/shaders/11_Texture3D/lut.frag.spv");
		m_Pipeline1 = VKGfxPipeline::Create(m_VulkanDevice, m_PipelineCache, pipelineInfo1, { vertexInputBinding }, vertexInputAttributs, m_PipelineLayout, m_RenderPass);

		VKGfxPipelineInfo pipelineInfo2;
		pipelineInfo2.vertShaderModule = vkutils::LoadSPIPVShader(m_Device, "data/shaders/11_Texture3D/debug0.vert.spv");
		pipelineInfo2.fragShaderModule = vkutils::LoadSPIPVShader(m_Device, "data/shaders/11_Texture3D/debug0.frag.spv");
		m_Pipeline2 = VKGfxPipeline::Create(m_VulkanDevice, m_PipelineCache, pipelineInfo2, { vertexInputBinding }, vertexInputAttributs, m_PipelineLayout, m_RenderPass);

		VKGfxPipelineInfo pipelineInfo3;
		pipelineInfo3.vertShaderModule = vkutils::LoadSPIPVShader(m_Device, "data/shaders/11_Texture3D/debug1.vert.spv");
		pipelineInfo3.fragShaderModule = vkutils::LoadSPIPVShader(m_Device, "data/shaders/11_Texture3D/debug1.frag.spv");
		m_Pipeline3 = VKGfxPipeline::Create(m_VulkanDevice, m_PipelineCache, pipelineInfo3, { vertexInputBinding }, vertexInputAttributs, m_PipelineLayout, m_RenderPass);

		vkDestroyShaderModule(m_Device, pipelineInfo0.vertShaderModule, VULKAN_CPU_ALLOCATOR);
		vkDestroyShaderModule(m_Device, pipelineInfo0.fragShaderModule, VULKAN_CPU_ALLOCATOR);

		vkDestroyShaderModule(m_Device, pipelineInfo1.vertShaderModule, VULKAN_CPU_ALLOCATOR);
		vkDestroyShaderModule(m_Device, pipelineInfo1.fragShaderModule, VULKAN_CPU_ALLOCATOR);

		vkDestroyShaderModule(m_Device, pipelineInfo2.vertShaderModule, VULKAN_CPU_ALLOCATOR);
		vkDestroyShaderModule(m_Device, pipelineInfo2.fragShaderModule, VULKAN_CPU_ALLOCATOR);

		vkDestroyShaderModule(m_Device, pipelineInfo3.vertShaderModule, VULKAN_CPU_ALLOCATOR);
		vkDestroyShaderModule(m_Device, pipelineInfo3.fragShaderModule, VULKAN_CPU_ALLOCATOR);
	}

	void DestroyPipelines()
	{
		delete m_Pipeline0;
		delete m_Pipeline1;
		delete m_Pipeline2;
		delete m_Pipeline3;
	}

	void CreateDescriptorSetLayout()
	{
		std::vector<VkDescriptorSetLayoutBinding> layoutBindings(4);
		layoutBindings[0].binding = 0;
		layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBindings[0].descriptorCount = 1;
		layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		layoutBindings[0].pImmutableSamplers = nullptr;

		layoutBindings[1].binding = 1;
		layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layoutBindings[1].descriptorCount = 1;
		layoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutBindings[1].pImmutableSamplers = nullptr;

		layoutBindings[2].binding = 2;
		layoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layoutBindings[2].descriptorCount = 1;
		layoutBindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutBindings[2].pImmutableSamplers = nullptr;

		layoutBindings[3].binding = 3;
		layoutBindings[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBindings[3].descriptorCount = 1;
		layoutBindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutBindings[3].pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo descSetLayoutInfo;
		ZeroVulkanStruct(descSetLayoutInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
		descSetLayoutInfo.bindingCount = layoutBindings.size();
		descSetLayoutInfo.pBindings = layoutBindings.data();
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

		m_LutDebugBuffer->CopyFrom(&m_LutDebugData, sizeof(LutDebugBlock));
	}

	void CreateUniformBuffers()
	{
		VKBoundingBox bounds = m_Model->rootNode->GetBounds();
		Vector3 boundSize = bounds.max - bounds.min;
		Vector3 boundCenter = bounds.min + boundSize * 0.5f;
		boundCenter.z = -1.0f;

		m_MVPData.model.AppendRotation(180, Vector3::UpVector);
		m_MVPData.model.AppendScale(Vector3(1.0f, 0.5f, 1.0f));

		m_ViewCamera.Perspective(PI / 4, m_configuration.window.windowWidth, m_configuration.window.windowHeight, 0.1f, 1500.0f);
		m_ViewCamera.SetPosition(boundCenter);

		m_MVPBuffer = DVKBuffer::CreateBuffer(
			m_VulkanDevice,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			sizeof(MVPBlock),
			&(m_MVPData)
		);
		m_MVPBuffer->Map();

		// lut debug data
		m_LutDebugData.bias = 0;
		m_LutDebugBuffer = DVKBuffer::CreateBuffer(
			m_VulkanDevice,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			sizeof(LutDebugBlock),
			&(m_LutDebugData)
		);
		m_LutDebugBuffer->Map();
	}

	void DestroyUniformBuffers()
	{
		m_MVPBuffer->UnMap();
		delete m_MVPBuffer;

		m_LutDebugBuffer->UnMap();
		delete m_LutDebugBuffer;
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

	bool							m_AutoRotate = true;
	bool 							m_Ready = false;

	VKCamera						m_ViewCamera;

	MVPBlock 						m_MVPData;
	DVKBuffer* m_MVPBuffer;

	LutDebugBlock                   m_LutDebugData;
	DVKBuffer* m_LutDebugBuffer = nullptr;;

	VKTexture* m_TexOrigin = nullptr;
	VKTexture* m_Tex3DLut = nullptr;

	VKGfxPipeline* m_Pipeline0 = nullptr;
	VKGfxPipeline* m_Pipeline1 = nullptr;
	VKGfxPipeline* m_Pipeline2 = nullptr;
	VKGfxPipeline* m_Pipeline3 = nullptr;

	VKModel* m_Model = nullptr;

	VkDescriptorPool                m_DescriptorPool = VK_NULL_HANDLE;

	VkDescriptorSetLayout 			m_DescriptorSetLayout = VK_NULL_HANDLE;
	VkPipelineLayout 				m_PipelineLayout = VK_NULL_HANDLE;

	VkDescriptorSet 				m_DescriptorSet0 = VK_NULL_HANDLE;
	VkDescriptorSet 				m_DescriptorSet1 = VK_NULL_HANDLE;
	VkDescriptorSet 				m_DescriptorSet2 = VK_NULL_HANDLE;
	VkDescriptorSet 				m_DescriptorSet3 = VK_NULL_HANDLE;

	ImageGUIContext* m_GUI = nullptr;
};