#pragma once

#define NUM_LIGHTS 64

class Material final
{
public:
	Material(Configuration& configuration) noexcept;

	void StartGame() noexcept;
private:
	Material() = delete;
	Material(const Material&) = delete;
	Material(Material&&) = delete;
	Material operator=(const Material&) = delete;
	Material operator=(Material&&) = delete;

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

	struct ViewProjectionBlock
	{
		Matrix4x4 view;
		Matrix4x4 projection;
	};

	struct AttachmentParamBlock
	{
		float		attachmentIndex;
		float		zNear;
		float		zFar;
		float		one;
		float		xMaxFar;
		float		yMaxFar;
		Vector2		padding;
		Matrix4x4	invView;
	};

	struct PointLight
	{
		Vector4 position;
		Vector3 color;
		float	radius;
	};

	struct LightSpawnBlock
	{
		Vector3 position[NUM_LIGHTS];
		Vector3 direction[NUM_LIGHTS];
		float speed[NUM_LIGHTS];
	};

	struct LightDataBlock
	{
		PointLight lights[NUM_LIGHTS];
	};

	void Draw(float time, float delta)
	{
		int32_t bufferIndex = m_defContext->AcquireBackbufferIndex();

		bool hovered = UpdateUI(time, delta);
		if (!hovered)
			m_ViewCamera.Update(time, delta);

		UpdateUniform(time, delta);

		// model
		m_Material0->BeginFrame();
		for (int32_t i = 0; i < m_Model->meshes.size(); ++i) {
			m_Material0->BeginObject();
			m_Material0->SetLocalUniform("uboModel", &(m_Model->meshes[i]->linkNode->GetGlobalMatrix()), sizeof(Matrix4x4));
			m_Material0->SetLocalUniform("uboViewProj", &m_ViewProjData, sizeof(m_ViewProjData));
			m_Material0->EndObject();
		}
		m_Material0->EndFrame();

		// postprocess
		m_Material1->BeginFrame();
		m_Material1->BeginObject();
		m_Material1->SetLocalUniform("paramData", &m_VertFragParam, sizeof(AttachmentParamBlock));
		m_Material1->SetLocalUniform("lightDatas", &m_LightDatas, sizeof(LightDataBlock));
		m_Material1->SetInputAttachment("inputColor", m_AttachsColor[bufferIndex]);
		m_Material1->SetInputAttachment("inputNormal", m_AttachsNormal[bufferIndex]);
		m_Material1->SetInputAttachment("inputDepth", m_AttachsDepth[bufferIndex]);
		m_Material1->EndObject();
		m_Material1->EndFrame();

		SetupCommandBuffers(bufferIndex);

		m_defContext->Present(bufferIndex);
	}

	void UpdateUniform(float time, float delta)
	{
		m_ViewCamera.Perspective(PI / 2, m_configuration.window.windowWidth, m_configuration.window.windowHeight, m_VertFragParam.zNear, m_VertFragParam.zFar);
		m_ViewProjData.view = m_ViewCamera.GetView();
		m_ViewProjData.projection = m_ViewCamera.GetProjection();

		m_VertFragParam.invView = m_ViewCamera.GetView();
		m_VertFragParam.invView.SetInverse();
		m_VertFragParam.yMaxFar = m_VertFragParam.zFar * math::Tan(math::DegreesToRadians(45.0f) / 2);
		m_VertFragParam.xMaxFar = m_VertFragParam.yMaxFar * (float)m_configuration.window.windowWidth / (float)m_configuration.window.windowHeight;

		for (int32_t i = 0; i < NUM_LIGHTS; ++i)
		{
			float bias = math::Sin(time * m_LightInfos.speed[i]) / 5.0f;
			m_LightDatas.lights[i].position.x = m_LightInfos.position[i].x + bias * m_LightInfos.direction[i].x * 500.0f;
			m_LightDatas.lights[i].position.y = m_LightInfos.position[i].y + bias * m_LightInfos.direction[i].y * 500.0f;
			m_LightDatas.lights[i].position.z = m_LightInfos.position[i].z + bias * m_LightInfos.direction[i].z * 500.0f;
		}
	}

	void CreateFrameBuffers()
	{
		DestroyFrameBuffers();

		int32_t fwidth = m_vulkanRHI->GetSwapChain()->GetWidth();
		int32_t fheight = m_vulkanRHI->GetSwapChain()->GetHeight();
		VkDevice device = m_vulkanRHI->GetDevice()->GetInstanceHandle();

		VkImageView attachments[4];

		VkFramebufferCreateInfo frameBufferCreateInfo;
		ZeroVulkanStruct(frameBufferCreateInfo, VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO);
		frameBufferCreateInfo.renderPass = m_RenderPass;
		frameBufferCreateInfo.attachmentCount = 4;
		frameBufferCreateInfo.pAttachments = attachments;
		frameBufferCreateInfo.width = fwidth;
		frameBufferCreateInfo.height = fheight;
		frameBufferCreateInfo.layers = 1;

		const std::vector<VkImageView>& backbufferViews = m_vulkanRHI->GetBackbufferViews();

		m_FrameBuffers.resize(backbufferViews.size());
		for (uint32_t i = 0; i < m_FrameBuffers.size(); ++i) {
			attachments[0] = backbufferViews[i];
			attachments[1] = m_AttachsColor[i]->imageView;
			attachments[2] = m_AttachsNormal[i]->imageView;
			attachments[3] = m_AttachsDepth[i]->imageView;
			VERIFYVULKANRESULT(vkCreateFramebuffer(device, &frameBufferCreateInfo, VULKAN_CPU_ALLOCATOR, &m_FrameBuffers[i]));
		}
	}

	void DestroyFrameBuffers()
	{
		VkDevice device = m_vulkanRHI->GetDevice()->GetInstanceHandle();
		for (int32_t i = 0; i < m_FrameBuffers.size(); ++i)
			vkDestroyFramebuffer(device, m_FrameBuffers[i], VULKAN_CPU_ALLOCATOR);
		m_FrameBuffers.clear();
	}

	void DestroyAttachments()
	{
		for (int32_t i = 0; i < m_AttachsDepth.size(); ++i)
		{
			VKTexture* texture = m_AttachsDepth[i];
			delete texture;
		}
		m_AttachsDepth.clear();

		for (int32_t i = 0; i < m_AttachsColor.size(); ++i)
		{
			VKTexture* texture = m_AttachsColor[i];
			delete texture;
		}
		m_AttachsColor.clear();

		for (int32_t i = 0; i < m_AttachsNormal.size(); ++i)
		{
			VKTexture* texture = m_AttachsNormal[i];
			delete texture;
		}
		m_AttachsNormal.clear();
	}

	void CreateAttachments()
	{
		auto swapChain = m_vulkanRHI->GetSwapChain();
		int32_t fwidth = swapChain->GetWidth();
		int32_t fheight = swapChain->GetHeight();
		int32_t numBuffer = swapChain->GetBackBufferCount();

		m_AttachsColor.resize(numBuffer);
		m_AttachsNormal.resize(numBuffer);
		m_AttachsDepth.resize(numBuffer);

		for (int32_t i = 0; i < m_AttachsColor.size(); ++i)
		{
			m_AttachsColor[i] = VKTexture::CreateAttachment(
				m_VulkanDevice,
				PixelFormatToVkFormat(m_vulkanRHI->GetPixelFormat(), false),
				VK_IMAGE_ASPECT_COLOR_BIT,
				fwidth, fheight,
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
			);
		}

		for (int32_t i = 0; i < m_AttachsNormal.size(); ++i)
		{
			m_AttachsNormal[i] = VKTexture::CreateAttachment(
				m_VulkanDevice,
				VK_FORMAT_R8G8B8A8_UNORM,
				VK_IMAGE_ASPECT_COLOR_BIT,
				fwidth, fheight,
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
			);
		}

		for (int32_t i = 0; i < m_AttachsDepth.size(); ++i)
		{
			m_AttachsDepth[i] = VKTexture::CreateAttachment(
				m_VulkanDevice,
				PixelFormatToVkFormat(m_vkContext->m_DepthFormat, false),
				VK_IMAGE_ASPECT_DEPTH_BIT,
				fwidth, fheight,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
			);
		}
	}

	void CreateRenderPass()
	{
		DestroyRenderPass();
		DestroyAttachments();
		CreateAttachments();

		VkDevice device = m_vulkanRHI->GetDevice()->GetInstanceHandle();
		PixelFormat pixelFormat = m_vulkanRHI->GetPixelFormat();

		std::vector<VkAttachmentDescription> attachments(4);
		// swap chain attachment
		attachments[0].format = PixelFormatToVkFormat(pixelFormat, false);
		attachments[0].samples = m_vkContext->m_SampleCount;
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		// color attachment
		attachments[1].format = PixelFormatToVkFormat(pixelFormat, false);
		attachments[1].samples = m_vkContext->m_SampleCount;
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		// normal attachment
		attachments[2].format = VK_FORMAT_R8G8B8A8_UNORM;
		attachments[2].samples = m_vkContext->m_SampleCount;
		attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[2].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		// depth stencil attachment
		attachments[3].format = PixelFormatToVkFormat(m_vkContext->m_DepthFormat, false);
		attachments[3].samples = m_vkContext->m_SampleCount;
		attachments[3].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[3].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[3].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorReferences[2];
		colorReferences[0].attachment = 1;
		colorReferences[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorReferences[1].attachment = 2;
		colorReferences[1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference swapReference = { };
		swapReference.attachment = 0;
		swapReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthReference = { };
		depthReference.attachment = 3;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference inputReferences[3];
		inputReferences[0].attachment = 1;
		inputReferences[0].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		inputReferences[1].attachment = 2;
		inputReferences[1].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		inputReferences[2].attachment = 3;
		inputReferences[2].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		std::vector<VkSubpassDescription> subpassDescriptions(2);
		subpassDescriptions[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescriptions[0].colorAttachmentCount = 2;
		subpassDescriptions[0].pColorAttachments = colorReferences;
		subpassDescriptions[0].pDepthStencilAttachment = &depthReference;

		subpassDescriptions[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescriptions[1].colorAttachmentCount = 1;
		subpassDescriptions[1].pColorAttachments = &swapReference;
		subpassDescriptions[1].inputAttachmentCount = 3;
		subpassDescriptions[1].pInputAttachments = inputReferences;

		std::vector<VkSubpassDependency> dependencies(3);
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = 1;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[2].srcSubpass = 1;
		dependencies[2].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[2].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[2].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo renderPassInfo;
		ZeroVulkanStruct(renderPassInfo, VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO);
		renderPassInfo.attachmentCount = attachments.size();
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = subpassDescriptions.size();
		renderPassInfo.pSubpasses = subpassDescriptions.data();
		renderPassInfo.dependencyCount = dependencies.size();
		renderPassInfo.pDependencies = dependencies.data();
		VERIFYVULKANRESULT(vkCreateRenderPass(device, &renderPassInfo, VULKAN_CPU_ALLOCATOR, &m_RenderPass));
	}

	void DestroyRenderPass()
	{
		VkDevice device = m_vulkanRHI->GetDevice()->GetInstanceHandle();
		if (m_RenderPass != VK_NULL_HANDLE) {
			vkDestroyRenderPass(device, m_RenderPass, VULKAN_CPU_ALLOCATOR);
			m_RenderPass = VK_NULL_HANDLE;
		}
	}

	bool UpdateUI(float time, float delta)
	{
		m_GUI->StartFrame();

		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("MaterialDemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

			int32_t index = m_VertFragParam.attachmentIndex;
			ImGui::SliderInt("Index", &index, 0, 3);
			m_VertFragParam.attachmentIndex = index;

			if (ImGui::Button("Random"))
			{
				VKBoundingBox bounds = m_Model->rootNode->GetBounds();

				for (int32_t i = 0; i < NUM_LIGHTS; ++i)
				{
					m_LightDatas.lights[i].position.x = math::RandRange(bounds.min.x, bounds.max.x);
					m_LightDatas.lights[i].position.y = math::RandRange(bounds.min.y, bounds.max.y);
					m_LightDatas.lights[i].position.z = math::RandRange(bounds.min.z, bounds.max.z);

					m_LightDatas.lights[i].color.x = math::RandRange(0.0f, 1.0f);
					m_LightDatas.lights[i].color.y = math::RandRange(0.0f, 1.0f);
					m_LightDatas.lights[i].color.z = math::RandRange(0.0f, 1.0f);

					m_LightDatas.lights[i].radius = math::RandRange(50.0f, 200.0f);

					m_LightInfos.position[i] = m_LightDatas.lights[i].position;
					m_LightInfos.direction[i] = m_LightInfos.position[i];
					m_LightInfos.direction[i].Normalize();
					m_LightInfos.speed[i] = 1.0f + math::RandRange(0.0f, 5.0f);
				}
			}

			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		bool hovered = ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsRootWindowOrAnyChildHovered();

		m_GUI->EndFrame();
		m_GUI->Update();

		return hovered;
	}

	void LoadAssets()
	{
		// shader0
		m_Shader0 = VKShader::Create(
			m_VulkanDevice,
			true,
			"data/shaders/16_Material/obj.vert.spv",
			"data/shaders/16_Material/obj.frag.spv"
		);

		// model
		VKCommandBuffer* cmdBuffer = VKCommandBuffer::Create(m_VulkanDevice, m_defContext->m_CommandPool);
		m_Model = VKModel::LoadFromFile(
			"data/models/Room/miniHouse_FBX.FBX",
			m_VulkanDevice,
			cmdBuffer,
			m_Shader0->perVertexAttributes
		);
		delete cmdBuffer;

		// buffer material
		m_Material0 = VKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_Shader0
		);
		// renderpass
		m_Material0->pipelineInfo.colorAttachmentCount = 2;
		m_Material0->PreparePipeline();

		// shader1
		m_Shader1 = VKShader::Create(
			m_VulkanDevice,
			true,
			"data/shaders/16_Material/quad.vert.spv",
			"data/shaders/16_Material/quad.frag.spv"
		);
		// deferred material
		m_Material1 = VKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_Shader1
		);
		m_Material1->pipelineInfo.depthStencilState.depthTestEnable = VK_FALSE;
		m_Material1->pipelineInfo.depthStencilState.depthWriteEnable = VK_FALSE;
		m_Material1->pipelineInfo.depthStencilState.stencilTestEnable = VK_FALSE;
		m_Material1->pipelineInfo.shader = m_Shader1;
		m_Material1->pipelineInfo.subpass = 1;
		m_Material1->PreparePipeline();
	}

	void DestroyAssets()
	{
		delete m_Model;

		delete m_Shader0;
		delete m_Shader1;

		delete m_Material0;
		delete m_Material1;
	}

	void SetupCommandBuffers(int32_t backBufferIndex)
	{
		VkCommandBufferBeginInfo cmdBeginInfo;
		ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);

		VkClearValue clearValues[4];
		clearValues[0].color = { { 0.2f, 0.2f, 0.4f, 0.0f } };
		clearValues[1].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
		clearValues[2].color = { { 0.2f, 0.2f, 0.2f, 0.0f } };
		clearValues[3].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo;
		ZeroVulkanStruct(renderPassBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
		renderPassBeginInfo.renderPass = m_RenderPass;
		renderPassBeginInfo.framebuffer = m_FrameBuffers[backBufferIndex];
		renderPassBeginInfo.clearValueCount = 4;
		renderPassBeginInfo.pClearValues = clearValues;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = m_defContext->m_FrameWidth;
		renderPassBeginInfo.renderArea.extent.height = m_defContext->m_FrameHeight;

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

		VERIFYVULKANRESULT(vkBeginCommandBuffer(commandBuffer, &cmdBeginInfo));
		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		// pass0
		{
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Material0->GetPipeline());
			for (int32_t meshIndex = 0; meshIndex < m_Model->meshes.size(); ++meshIndex) {
				m_Material0->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, meshIndex);
				m_Model->meshes[meshIndex]->BindDrawCmd(commandBuffer);
			}
		}

		vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);

		// pass1
		{
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Material1->GetPipeline());
			m_Material1->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
			VKDefaultRes::fullQuad->meshes[0]->BindDrawCmd(commandBuffer);
		}

		m_GUI->BindDrawCmd(commandBuffer, m_RenderPass, 1);

		vkCmdEndRenderPass(commandBuffer);
		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));
	}

	void InitParmas()
	{
		VKBoundingBox bounds = m_Model->rootNode->GetBounds();
		Vector3 boundSize = bounds.max - bounds.min;
		Vector3 boundCenter = bounds.min + boundSize * 0.5f;

		// light datas
		for (int32_t i = 0; i < NUM_LIGHTS; ++i)
		{
			m_LightDatas.lights[i].position.x = math::RandRange(bounds.min.x, bounds.max.x);
			m_LightDatas.lights[i].position.y = math::RandRange(bounds.min.y, bounds.max.y);
			m_LightDatas.lights[i].position.z = math::RandRange(bounds.min.z, bounds.max.z);
			m_LightDatas.lights[i].position.w = 1.0f;

			m_LightDatas.lights[i].color.x = math::RandRange(0.0f, 1.0f);
			m_LightDatas.lights[i].color.y = math::RandRange(0.0f, 1.0f);
			m_LightDatas.lights[i].color.z = math::RandRange(0.0f, 1.0f);

			m_LightDatas.lights[i].radius = math::RandRange(50.0f, 200.0f);

			m_LightInfos.position[i] = m_LightDatas.lights[i].position;
			m_LightInfos.direction[i] = m_LightInfos.position[i];
			m_LightInfos.direction[i].Normalize();
			m_LightInfos.speed[i] = 1.0f + math::RandRange(0.0f, 5.0f);
		}

		// param
		m_VertFragParam.attachmentIndex = 0;
		m_VertFragParam.zNear = 10.0f;
		m_VertFragParam.zFar = 3000.0f;
		m_VertFragParam.one = 1.0f;
		m_VertFragParam.yMaxFar = m_VertFragParam.zFar * math::Tan(math::DegreesToRadians(75.0f) / 2);
		m_VertFragParam.xMaxFar = m_VertFragParam.yMaxFar * (float)m_configuration.window.windowWidth / (float)m_configuration.window.windowHeight;

		m_ViewCamera.Perspective(PI / 4, m_configuration.window.windowWidth, m_configuration.window.windowHeight, m_VertFragParam.zNear, m_VertFragParam.zFar);
		m_ViewCamera.SetPosition(boundCenter.x, boundCenter.y + 1000, boundCenter.z - boundSize.Size());
		m_ViewCamera.LookAt(boundCenter);

		m_VertFragParam.invView = m_ViewProjData.view;
		m_VertFragParam.invView.SetInverse();
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

	typedef std::vector<VKTexture*>			VKTextureArray;

	std::vector<VkFramebuffer>	m_FrameBuffers;

	bool							m_AutoRotate = true;
	bool 							m_Ready = false;

	VKCamera						m_ViewCamera;

	ViewProjectionBlock				m_ViewProjData;

	AttachmentParamBlock			m_VertFragParam;
	LightDataBlock					m_LightDatas;
	LightSpawnBlock					m_LightInfos;

	VKModel* m_Model = nullptr;

	VKShader* m_Shader0 = nullptr;
	VKMaterial* m_Material0 = nullptr;

	VKShader* m_Shader1 = nullptr;
	VKMaterial* m_Material1 = nullptr;

	VKTextureArray					m_AttachsDepth;
	VKTextureArray					m_AttachsColor;
	VKTextureArray                 m_AttachsNormal;

	ImageGUIContext* m_GUI = nullptr;
};