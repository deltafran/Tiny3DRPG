#pragma once

enum FXAATypes
{
	Normal = 0,
	Default,
	Fast,
	High,
	Best,
	Count,
};

class FXAA final
{
public:
	FXAA(Configuration& configuration) noexcept;

	void StartGame() noexcept;
private:
	FXAA() = delete;
	FXAA(const FXAA&) = delete;
	FXAA(FXAA&&) = delete;
	FXAA operator=(const FXAA&) = delete;
	FXAA operator=(FXAA&&) = delete;

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

	struct ModelViewProjectionBlock
	{
		Matrix4x4 model;
		Matrix4x4 view;
		Matrix4x4 projection;
	};

	struct FXAAParamBlock
	{
		Vector2 frame;
		Vector2 padding;
	};
		
	void Draw(float time, float delta)
	{
		int32_t bufferIndex = m_vkContext->AcquireBackbufferIndex();

		bool hovered = UpdateUI(time, delta);
		if (!hovered)
			m_ViewCamera.Update(time, delta);

		m_MVPData.view = m_ViewCamera.GetView();
		m_MVPData.projection = m_ViewCamera.GetProjection();

		if (m_AutoRotate) {
			m_LineModel->rootNode->localMatrix.AppendRotation(delta * 15.0f, Vector3::UpVector);
		}

		// model
		m_LineMaterial->BeginFrame();
		m_MVPData.model = m_LineModel->meshes[0]->linkNode->GetGlobalMatrix();
		m_LineMaterial->BeginObject();
		m_LineMaterial->SetLocalUniform("uboMVP", &m_MVPData, sizeof(ModelViewProjectionBlock));
		m_LineMaterial->EndObject();
		m_LineMaterial->EndFrame();

		// filter
		VKMaterial* material = m_FilterMaterials[m_Select];
		material->BeginFrame();
		material->BeginObject();
		material->SetLocalUniform("fxaaParam", &m_FXAAParam, sizeof(FXAAParamBlock));
		material->EndObject();
		material->EndFrame();

		SetupCommandBuffers(bufferIndex);

		m_vkContext->Present(bufferIndex);
	}

	bool UpdateUI(float time, float delta)
	{
		m_GUI->StartFrame();

		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("FXAADemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

			ImGui::Checkbox("Auto Spin", &m_AutoRotate);

			int select = m_Select;
			ImGui::Combo("Quality", &select, m_FilterNames.data(), m_FilterNames.size());
			m_Select = (FXAATypes)select;

			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		bool hovered = ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsRootWindowOrAnyChildHovered();

		m_GUI->EndFrame();
		m_GUI->Update();

		return hovered;
	}

	void GenerateLineSphere(std::vector<float>& outVertices, int32_t sphslices, float scale)
	{
		int32_t count = 0;
		int32_t slices = sphslices;
		int32_t stacks = slices;

		outVertices.resize((slices + 1) * stacks * 3 * 2);

		float ds = 1.0f / sphslices;
		float dt = 1.0f / sphslices;
		float t = 1.0;
		float drho = PI / stacks;
		float dtheta = 2.0 * PI / slices;

		for (int32_t i = 0; i < stacks; ++i)
		{
			float rho = i * drho;
			float s = 0.0;
			for (int32_t j = 0; j <= slices; ++j) {
				float theta = (j == slices) ? 0.0f : j * dtheta;
				float x = -sin(theta) * sin(rho) * scale;
				float z = cos(theta) * sin(rho) * scale;
				float y = -cos(rho) * scale;

				outVertices[count + 0] = x;
				outVertices[count + 1] = y;
				outVertices[count + 2] = z;
				count += 3;

				x = -sin(theta) * sin(rho + drho) * scale;
				z = cos(theta) * sin(rho + drho) * scale;
				y = -cos(rho + drho) * scale;

				outVertices[count + 0] = x;
				outVertices[count + 1] = y;
				outVertices[count + 2] = z;
				count += 3;

				s += ds;
			}
			t -= dt;
		}
	}

	void DestroyMaterials()
	{
		delete m_LineShader;
		delete m_LineMaterial;

		delete m_NormalShader;
		delete m_NormalMaterial;

		delete m_FXAADefaultMaterial;
		delete m_FXAADefaultShader;

		delete m_FXAAFastShader;
		delete m_FXAAFastMaterial;

		delete m_FXAAHighShader;
		delete m_FXAAHighMaterial;

		delete m_FXAABestMaterial;
		delete m_FXAABestShader;
	}

	void CreateMaterials()
	{
		m_LineShader = VKShader::Create(
			m_VulkanDevice,
			true,
			"data/shaders/28_FXAA/obj.vert.spv",
			"data/shaders/28_FXAA/obj.frag.spv"
		);

		float range0 = m_VulkanDevice->GetLimits().lineWidthRange[0];
		float range1 = m_VulkanDevice->GetLimits().lineWidthRange[1];
		float lineWidth = math::Clamp(3.0f, range0, range1);
		lineWidth = math::Min(lineWidth, m_VulkanDevice->GetLimits().lineWidthRange[1]);

		m_LineMaterial = VKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_LineShader
		);
		m_LineMaterial->pipelineInfo.inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		m_LineMaterial->pipelineInfo.rasterizationState.cullMode = VK_CULL_MODE_NONE;
		m_LineMaterial->pipelineInfo.rasterizationState.lineWidth = lineWidth;
		m_LineMaterial->pipelineInfo.rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		m_LineMaterial->PreparePipeline();

		// fxaa-default
		m_FXAADefaultShader = VKShader::Create(
			m_VulkanDevice,
			true,
			"data/shaders/28_FXAA/FXAA.vert.spv",
			"data/shaders/28_FXAA/FXAA_Default.frag.spv"
		);

		// fxaa-fast
		m_FXAAFastShader = VKShader::Create(
			m_VulkanDevice,
			true,
			"data/shaders/28_FXAA/FXAA.vert.spv",
			"data/shaders/28_FXAA/FXAA_Fastest.frag.spv"
		);

		// fxaa-hight
		m_FXAAHighShader = VKShader::Create(
			m_VulkanDevice,
			true,
			"data/shaders/28_FXAA/FXAA.vert.spv",
			"data/shaders/28_FXAA/FXAA_High_Quality.frag.spv"
		);

		// fxaa-best
		m_FXAABestShader = VKShader::Create(
			m_VulkanDevice,
			true,
			"data/shaders/28_FXAA/FXAA.vert.spv",
			"data/shaders/28_FXAA/FXAA_Extreme_Quality.frag.spv"
		);

		// faxaa-none
		m_NormalShader = VKShader::Create(
			m_VulkanDevice,
			true,
			"data/shaders/28_FXAA/Normal.vert.spv",
			"data/shaders/28_FXAA/Normal.frag.spv"
		);

		// fxaa-none
		m_NormalMaterial = VKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_NormalShader
		);
		m_NormalMaterial->PreparePipeline();
		m_NormalMaterial->SetTexture("sourceTexture", m_RTColor);

		// fxaa-default
		m_FXAADefaultMaterial = VKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_FXAADefaultShader
		);
		m_FXAADefaultMaterial->PreparePipeline();
		m_FXAADefaultMaterial->SetTexture("sourceTexture", m_RTColor);

		// fxaa-fast
		m_FXAAFastMaterial = VKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_FXAAFastShader
		);
		m_FXAAFastMaterial->PreparePipeline();
		m_FXAAFastMaterial->SetTexture("sourceTexture", m_RTColor);

		// fxaa-high
		m_FXAAHighMaterial = VKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_FXAAHighShader
		);
		m_FXAAHighMaterial->PreparePipeline();
		m_FXAAHighMaterial->SetTexture("sourceTexture", m_RTColor);

		// fxaa-best
		m_FXAABestMaterial = VKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_FXAABestShader
		);
		m_FXAABestMaterial->PreparePipeline();
		m_FXAABestMaterial->SetTexture("sourceTexture", m_RTColor);

		m_FilterNames.resize(FXAATypes::Count);
		m_FilterNames[FXAATypes::Normal] = "None";
		m_FilterNames[FXAATypes::Default] = "FXAA-Default";
		m_FilterNames[FXAATypes::Fast] = "FXAA-Fast";
		m_FilterNames[FXAATypes::High] = "FXAA-High";
		m_FilterNames[FXAATypes::Best] = "FXAA-Best";

		m_FilterMaterials.resize(FXAATypes::Count);
		m_FilterMaterials[FXAATypes::Normal] = m_NormalMaterial;
		m_FilterMaterials[FXAATypes::Default] = m_FXAADefaultMaterial;
		m_FilterMaterials[FXAATypes::Fast] = m_FXAAFastMaterial;
		m_FilterMaterials[FXAATypes::High] = m_FXAAHighMaterial;
		m_FilterMaterials[FXAATypes::Best] = m_FXAABestMaterial;
	}

	void CreateRenderTarget()
	{
		m_RTColor = VKTexture::CreateRenderTarget(
			m_VulkanDevice,
			PixelFormatToVkFormat(m_vulkanRHI->GetPixelFormat(), false),
			VK_IMAGE_ASPECT_COLOR_BIT,
			m_vkContext->m_FrameWidth, m_vkContext->m_FrameHeight,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		);

		m_RTDepth = VKTexture::CreateRenderTarget(
			m_VulkanDevice,
			PixelFormatToVkFormat(m_vkContext->m_DepthFormat, false),
			VK_IMAGE_ASPECT_DEPTH_BIT,
			m_vkContext->m_FrameWidth, m_vkContext->m_FrameHeight,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		);

		VKRenderPassInfo passInfo(
			m_RTColor, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
			m_RTDepth, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE
		);
		m_RenderTarget = VKRenderTarget::Create(m_VulkanDevice, passInfo);
	}

	void DestroyRenderTarget()
	{
		delete m_RenderTarget;
	}

	void LoadAssets()
	{
		VKCommandBuffer* cmdBuffer = VKCommandBuffer::Create(m_VulkanDevice, m_vkContext->m_CommandPool);

		m_Quad = VKDefaultRes::fullQuad;

		// LineSphere
		std::vector<float> vertices;
		GenerateLineSphere(vertices, 40, 1.0f);

		// model
		m_LineModel = VKModel::Create(
			m_VulkanDevice,
			cmdBuffer,
			vertices,
			{},
			{ VertexAttribute::VA_Position }
		);

		delete cmdBuffer;
	}

	void DestroyAssets()
	{
		delete m_LineModel;

		delete m_RTColor;
		delete m_RTDepth;
	}

	void SetupCommandBuffers(int32_t backBufferIndex)
	{
		VkViewport viewport = {};
		viewport.x = 0;
		viewport.y = m_vkContext->m_FrameHeight;
		viewport.width = m_vkContext->m_FrameWidth;
		viewport.height = -(float)m_vkContext->m_FrameHeight;    // flip y axis
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.extent.width = m_vkContext->m_FrameWidth;
		scissor.extent.height = m_vkContext->m_FrameHeight;
		scissor.offset.x = 0;
		scissor.offset.y = 0;

		VkCommandBuffer commandBuffer = m_vkContext->m_CommandBuffers[backBufferIndex];

		VkCommandBufferBeginInfo cmdBeginInfo;
		ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
		VERIFYVULKANRESULT(vkBeginCommandBuffer(commandBuffer, &cmdBeginInfo));

		// render target pass
		{
			m_RenderTarget->BeginRenderPass(commandBuffer);

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_LineMaterial->GetPipeline());
			for (int32_t j = 0; j < m_LineModel->meshes.size(); ++j) {
				m_LineMaterial->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, j);
				m_LineModel->meshes[j]->BindDrawCmd(commandBuffer);
			}

			m_RenderTarget->EndRenderPass(commandBuffer);
		}

		// fxaa pass
		{
			std::vector<VkClearValue> clearValues;
			clearValues.resize(2);
			clearValues[0].color = { { 0.2f, 0.2f, 0.2f, 1.0f } };
			clearValues[1].depthStencil = { 1.0f, 0 };

			VkRenderPassBeginInfo renderPassBeginInfo;
			ZeroVulkanStruct(renderPassBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
			renderPassBeginInfo.renderPass = m_RenderPass;
			renderPassBeginInfo.framebuffer = m_vkContext->m_FrameBuffers[backBufferIndex];
			renderPassBeginInfo.clearValueCount = clearValues.size();
			renderPassBeginInfo.pClearValues = clearValues.data();
			renderPassBeginInfo.renderArea.offset.x = 0;
			renderPassBeginInfo.renderArea.offset.y = 0;
			renderPassBeginInfo.renderArea.extent.width = m_vkContext->m_FrameWidth;
			renderPassBeginInfo.renderArea.extent.height = m_vkContext->m_FrameHeight;
			vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

			{
				VKMaterial* material = m_FilterMaterials[m_Select];
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->GetPipeline());
				material->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
				m_Quad->meshes[0]->BindDrawCmd(commandBuffer);
			}

			m_GUI->BindDrawCmd(commandBuffer, m_RenderPass);
			vkCmdEndRenderPass(commandBuffer);
		}

		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));
	}

	void InitParmas()
	{
		m_FXAAParam.frame.x = 1.0f / m_vkContext->m_FrameWidth;
		m_FXAAParam.frame.y = 1.0f / m_vkContext->m_FrameHeight;

		m_ViewCamera.SetPosition(0, 0.0f, -3.0f);
		m_ViewCamera.Perspective(PI / 4, m_configuration.window.windowWidth, m_configuration.window.windowHeight, 0.10f, 3000.0f);
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
	VKCamera						m_ViewCamera;
	ImageGUIContext*				m_GUI = nullptr;

	ModelViewProjectionBlock	m_MVPData;
	FXAAParamBlock				m_FXAAParam;

	VKModel* m_LineModel = nullptr;
	VKShader* m_LineShader = nullptr;
	VKMaterial* m_LineMaterial = nullptr;

	VKMaterial* m_NormalMaterial = nullptr;
	VKShader* m_NormalShader = nullptr;

	VKMaterial* m_FXAADefaultMaterial = nullptr;
	VKShader* m_FXAADefaultShader = nullptr;

	VKMaterial* m_FXAAFastMaterial = nullptr;
	VKShader* m_FXAAFastShader = nullptr;

	VKMaterial* m_FXAAHighMaterial = nullptr;
	VKShader* m_FXAAHighShader = nullptr;

	VKMaterial* m_FXAABestMaterial = nullptr;
	VKShader* m_FXAABestShader = nullptr;

	VKRenderTarget* m_RenderTarget = nullptr;
	VKTexture* m_RTColor = nullptr;
	VKTexture* m_RTDepth = nullptr;
	VKModel* m_Quad = nullptr;

	FXAATypes							m_Select = FXAATypes::Normal;
	std::vector<VKMaterial*>	m_FilterMaterials;
	std::vector<const char*>			m_FilterNames;
	bool								m_AutoRotate = false;
};