#pragma once

class OptimizeShaderAndLayout final
{
public:
	OptimizeShaderAndLayout(Configuration& configuration) noexcept;

	void StartGame() noexcept;
private:
	OptimizeShaderAndLayout() = delete;
	OptimizeShaderAndLayout(const OptimizeShaderAndLayout&) = delete;
	OptimizeShaderAndLayout(OptimizeShaderAndLayout&&) = delete;
	OptimizeShaderAndLayout operator=(const OptimizeShaderAndLayout&) = delete;
	OptimizeShaderAndLayout operator=(OptimizeShaderAndLayout&&) = delete;

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
		m_ShaderTexture = VKShader::Create(
			m_VulkanDevice,
			"data/shaders/12_OptimizeShaderAndLayout/texture.vert.spv",
			"data/shaders/12_OptimizeShaderAndLayout/texture.frag.spv"
		);
		m_ShaderLut = VKShader::Create(
			m_VulkanDevice,
			"data/shaders/12_OptimizeShaderAndLayout/lut.vert.spv",
			"data/shaders/12_OptimizeShaderAndLayout/lut.frag.spv"
		);
		m_ShaderLutDebug0 = VKShader::Create(
			m_VulkanDevice,
			"data/shaders/12_OptimizeShaderAndLayout/debug0.vert.spv",
			"data/shaders/12_OptimizeShaderAndLayout/debug0.frag.spv"
		);
		m_ShaderLutDebug1 = VKShader::Create(
			m_VulkanDevice,
			"data/shaders/12_OptimizeShaderAndLayout/debug1.vert.spv",
			"data/shaders/12_OptimizeShaderAndLayout/debug1.frag.spv"
		);

		VKCommandBuffer* cmdBuffer = VKCommandBuffer::Create(m_VulkanDevice, m_vkContext->m_CommandPool);

		m_Model = VKModel::LoadFromFile(
			"data/models/plane_z.obj",
			m_VulkanDevice,
			cmdBuffer,
			m_ShaderTexture->perVertexAttributes
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

		delete m_ShaderTexture;
		delete m_ShaderLut;
		delete m_ShaderLutDebug0;
		delete m_ShaderLutDebug1;
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
			vkCmdBindDescriptorSets(m_vkContext->m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline0->pipelineLayout, 0, m_DescriptorSet0->descriptorSets.size(), m_DescriptorSet0->descriptorSets.data(), 0, nullptr);
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
			vkCmdBindDescriptorSets(m_vkContext->m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline1->pipelineLayout, 0, m_DescriptorSet1->descriptorSets.size(), m_DescriptorSet1->descriptorSets.data(), 0, nullptr);
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
			vkCmdBindDescriptorSets(m_vkContext->m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline2->pipelineLayout, 0, m_DescriptorSet2->descriptorSets.size(), m_DescriptorSet2->descriptorSets.data(), 0, nullptr);
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
			vkCmdBindDescriptorSets(m_vkContext->m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline3->pipelineLayout, 0, m_DescriptorSet3->descriptorSets.size(), m_DescriptorSet3->descriptorSets.data(), 0, nullptr);
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
		m_DescriptorSet0 = m_ShaderTexture->AllocateDescriptorSet();
		m_DescriptorSet0->WriteBuffer("uboMVP", m_MVPBuffer);
		m_DescriptorSet0->WriteImage("diffuseMap", m_TexOrigin);

		m_DescriptorSet1 = m_ShaderLut->AllocateDescriptorSet();
		m_DescriptorSet1->WriteBuffer("uboMVP", m_MVPBuffer);
		m_DescriptorSet1->WriteImage("diffuseMap", m_TexOrigin);
		m_DescriptorSet1->WriteImage("lutMap", m_Tex3DLut);

		m_DescriptorSet2 = m_ShaderLutDebug0->AllocateDescriptorSet();
		m_DescriptorSet2->WriteBuffer("uboMVP", m_MVPBuffer);
		m_DescriptorSet2->WriteImage("diffuseMap", m_TexOrigin);
		m_DescriptorSet2->WriteImage("lutMap", m_Tex3DLut);
		m_DescriptorSet2->WriteBuffer("uboLutDebug", m_LutDebugBuffer);

		m_DescriptorSet3 = m_ShaderLutDebug1->AllocateDescriptorSet();
		m_DescriptorSet3->WriteBuffer("uboMVP", m_MVPBuffer);
		m_DescriptorSet3->WriteImage("diffuseMap", m_TexOrigin);
		m_DescriptorSet3->WriteImage("lutMap", m_Tex3DLut);
		m_DescriptorSet3->WriteBuffer("uboLutDebug", m_LutDebugBuffer);
	}

	void CreatePipelines()
	{
		VkVertexInputBindingDescription vertexInputBinding = m_Model->GetInputBinding();
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributs = m_Model->GetInputAttributes();

		VKGfxPipelineInfo pipelineInfo0;
		pipelineInfo0.shader = m_ShaderTexture;
		m_Pipeline0 = VKGfxPipeline::Create(m_VulkanDevice, m_PipelineCache, pipelineInfo0, { vertexInputBinding }, vertexInputAttributs, m_ShaderTexture->pipelineLayout, m_RenderPass);

		VKGfxPipelineInfo pipelineInfo1;
		pipelineInfo1.shader = m_ShaderLut;
		m_Pipeline1 = VKGfxPipeline::Create(m_VulkanDevice, m_PipelineCache, pipelineInfo1, { vertexInputBinding }, vertexInputAttributs, m_ShaderLut->pipelineLayout, m_RenderPass);

		VKGfxPipelineInfo pipelineInfo2;
		pipelineInfo2.shader = m_ShaderLutDebug0;
		m_Pipeline2 = VKGfxPipeline::Create(m_VulkanDevice, m_PipelineCache, pipelineInfo2, { vertexInputBinding }, vertexInputAttributs, m_ShaderLutDebug0->pipelineLayout, m_RenderPass);

		VKGfxPipelineInfo pipelineInfo3;
		pipelineInfo3.shader = m_ShaderLutDebug1;
		m_Pipeline3 = VKGfxPipeline::Create(m_VulkanDevice, m_PipelineCache, pipelineInfo3, { vertexInputBinding }, vertexInputAttributs, m_ShaderLutDebug1->pipelineLayout, m_RenderPass);
	}

	void DestroyPipelines()
	{
		delete m_Pipeline0;
		delete m_Pipeline1;
		delete m_Pipeline2;
		delete m_Pipeline3;

		delete m_DescriptorSet0;
		delete m_DescriptorSet1;
		delete m_DescriptorSet2;
		delete m_DescriptorSet3;
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

	VKShader* m_ShaderTexture = nullptr;
	VKShader* m_ShaderLut = nullptr;
	VKShader* m_ShaderLutDebug0 = nullptr;
	VKShader* m_ShaderLutDebug1 = nullptr;

	VKModel* m_Model = nullptr;

	VKDescriptorSet* m_DescriptorSet0 = nullptr;
	VKDescriptorSet* m_DescriptorSet1 = nullptr;
	VKDescriptorSet* m_DescriptorSet2 = nullptr;
	VKDescriptorSet* m_DescriptorSet3 = nullptr;

	ImageGUIContext* m_GUI = nullptr;
};