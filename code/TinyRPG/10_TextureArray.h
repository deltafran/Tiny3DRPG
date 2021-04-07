#pragma once
#pragma once
#pragma once

class TextureArray final
{
public:
	TextureArray(Configuration& configuration) noexcept;

	void StartGame() noexcept;
private:
	TextureArray() = delete;
	TextureArray(const TextureArray&) = delete;
	TextureArray(TextureArray&&) = delete;
	TextureArray operator=(const TextureArray&) = delete;
	TextureArray operator=(TextureArray&&) = delete;

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

	struct ModelBlock
	{
		Matrix4x4 model;
	};

	struct ParamBlock
	{
		float step;
		float debug;
		float padding0;
		float padding1;
	};

	struct ViewProjectionBlock
	{
		Matrix4x4 view;
		Matrix4x4 projection;
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
			ImGui::Begin("TextureArray", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
			ImGui::Text("Terrain");

			ImGui::Checkbox("AutoRotate", &m_AutoRotate);

			bool debug = m_ParamData.debug != 0;
			ImGui::Checkbox("Debug", &debug);
			m_ParamData.debug = debug ? 1 : 0;

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
			"data/models/StHelen.x",
			m_VulkanDevice,
			nullptr,
			{
				VertexAttribute::VA_Position,
				VertexAttribute::VA_UV0,
				VertexAttribute::VA_Normal,
				VertexAttribute::VA_Tangent,
				VertexAttribute::VA_Custom0
			}
		);

		for (int32_t i = 0; i < m_Model->meshes.size(); ++i)
		{
			VKMesh* mesh = m_Model->meshes[i];
			for (int32_t j = 0; j < mesh->primitives.size(); ++j)
			{
				VKPrimitive* primitive = mesh->primitives[j];
				int32_t stride = primitive->vertices.size() / primitive->vertexCount;
				for (int32_t v = 0; v < primitive->vertexCount; ++v)
				{
					float vy = primitive->vertices[v * stride + 1];

					float& tex0Index = primitive->vertices[v * stride + stride - 4];
					float& tex0Alpha = primitive->vertices[v * stride + stride - 3];
					float& tex1Index = primitive->vertices[v * stride + stride - 2];
					float& tex1Alpha = primitive->vertices[v * stride + stride - 1];

					const float snowLine = 0.8f;
					const float terrainLine = 0.3f;
					const float rocksLine = -0.1f;

					const float snowTerrainBandSize = 3.0f;
					const float terrainRocksBandSize = 0.5f;
					const float rocksLavaBandSize = 1.f;

					if (vy >= snowLine)
					{
						tex0Index = 0.0f;
						tex1Index = 1.0f;
						tex0Alpha = math::Min(1.0f, (vy - snowLine) / snowTerrainBandSize);
						tex1Alpha = 1.0f - tex0Alpha;
					}
					else if (vy >= terrainLine)
					{
						tex0Index = 2.0f;
						tex1Index = 1.0f;
						tex1Alpha = math::Min(1.0f, (vy - terrainLine) / terrainRocksBandSize);
						tex0Alpha = 1.0f - tex1Alpha;
					}
					else if (vy >= rocksLine)
					{
						tex0Index = 2.0f;
						tex1Index = 3.0f;
						tex0Alpha = math::Min(1.0f, (vy - rocksLine) / rocksLavaBandSize);
						tex1Alpha = 1.0f - tex0Alpha;
					}
					else
					{
						tex0Index = 2.0f;
						tex1Index = 3.0f;
						tex0Alpha = 0.0f;
						tex1Alpha = 1.0f;
					}
				}
			}
		}

		for (int32_t i = 0; i < m_Model->meshes.size(); ++i)
		{
			VKMesh* mesh = m_Model->meshes[i];
			for (int32_t j = 0; j < mesh->primitives.size(); ++j)
			{
				VKPrimitive* primitive = mesh->primitives[j];
				primitive->vertexBuffer = VKVertexBuffer::Create(m_VulkanDevice, cmdBuffer, primitive->vertices, m_Model->attributes);
				primitive->indexBuffer = VKIndexBuffer::Create(m_VulkanDevice, cmdBuffer, primitive->indices);
			}
		}

		m_Texture = VKTexture::Create2DArray(
			{
				"data/textures/terrain/snow.jpg",
				"data/textures/terrain/terrain.jpg",
				"data/textures/terrain/rocks.jpg",
				"data/textures/terrain/lava.jpg",

				"data/textures/terrain/snow_normal.jpg",
				"data/textures/terrain/terrain_normal.jpg",
				"data/textures/terrain/rocks_normal.jpg",
				"data/textures/terrain/lava_normal.jpg",

				"data/textures/terrain/mosaic-red.jpg",
				"data/textures/terrain/mosaic-green.jpg",
				"data/textures/terrain/mosaic-blue.jpg",
				"data/textures/terrain/mosaic-white.jpg"
			},
			m_VulkanDevice,
			cmdBuffer
		);

		delete cmdBuffer;
	}

	void DestroyAssets()
	{
		delete m_Model;
		delete m_Texture;
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
		renderPassBeginInfo.renderArea.extent.width = m_defContext->m_FrameWidth;
		renderPassBeginInfo.renderArea.extent.height = m_defContext->m_FrameHeight;

		uint32_t alignment = m_VulkanDevice->GetLimits().minUniformBufferOffsetAlignment;
		uint32_t modelAlign = Align(sizeof(ModelBlock), alignment);

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

			for (int32_t meshIndex = 0; meshIndex < m_Model->meshes.size(); ++meshIndex)
			{
				uint32_t dynamicOffsets[1] = {
					meshIndex * modelAlign
				};
				vkCmdBindDescriptorSets(m_defContext->m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->pipelineLayout, 0, 1, &m_DescriptorSet, 1, dynamicOffsets);
				m_Model->meshes[meshIndex]->BindDrawCmd(m_defContext->m_CommandBuffers[i]);
			}

			m_GUI->BindDrawCmd(m_defContext->m_CommandBuffers[i], m_RenderPass);

			vkCmdEndRenderPass(m_defContext->m_CommandBuffers[i]);
			VERIFYVULKANRESULT(vkEndCommandBuffer(m_defContext->m_CommandBuffers[i]));
		}
	}

	void CreateDescriptorSet()
	{
		std::vector<VkDescriptorPoolSize> poolSizes(4);
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = 1;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		poolSizes[1].descriptorCount = 1;
		poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[2].descriptorCount = 1;
		poolSizes[3].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[3].descriptorCount = 1;

		VkDescriptorPoolCreateInfo descriptorPoolInfo;
		ZeroVulkanStruct(descriptorPoolInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
		descriptorPoolInfo.poolSizeCount = poolSizes.size();
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
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
		writeDescriptorSet.pBufferInfo = &(m_ViewProjBuffer->descriptor);
		writeDescriptorSet.dstBinding = 0;
		vkUpdateDescriptorSets(m_Device, 1, &writeDescriptorSet, 0, nullptr);

		ZeroVulkanStruct(writeDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
		writeDescriptorSet.dstSet = m_DescriptorSet;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		writeDescriptorSet.pBufferInfo = &(m_ModelBuffer->descriptor);
		writeDescriptorSet.dstBinding = 1;
		vkUpdateDescriptorSets(m_Device, 1, &writeDescriptorSet, 0, nullptr);

		ZeroVulkanStruct(writeDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
		writeDescriptorSet.dstSet = m_DescriptorSet;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSet.pImageInfo = &(m_Texture->descriptorInfo);
		writeDescriptorSet.dstBinding = 2;
		vkUpdateDescriptorSets(m_Device, 1, &writeDescriptorSet, 0, nullptr);

		ZeroVulkanStruct(writeDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
		writeDescriptorSet.dstSet = m_DescriptorSet;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet.pBufferInfo = &(m_ParamBuffer->descriptor);
		writeDescriptorSet.dstBinding = 3;
		vkUpdateDescriptorSets(m_Device, 1, &writeDescriptorSet, 0, nullptr);
	}

	void CreatePipelines()
	{
		VkVertexInputBindingDescription vertexInputBinding = m_Model->GetInputBinding();
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributs = m_Model->GetInputAttributes();

		VKGfxPipelineInfo pipelineInfo;
		pipelineInfo.vertShaderModule = vkutils::LoadSPIPVShader(m_Device, "data/shaders/10_TextureArray/obj.vert.spv");
		pipelineInfo.fragShaderModule = vkutils::LoadSPIPVShader(m_Device, "data/shaders/10_TextureArray/obj.frag.spv");
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
		std::vector<VkDescriptorSetLayoutBinding> layoutBindings(4);
		layoutBindings[0].binding = 0;
		layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBindings[0].descriptorCount = 1;
		layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		layoutBindings[0].pImmutableSamplers = nullptr;

		layoutBindings[1].binding = 1;
		layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		layoutBindings[1].descriptorCount = 1;
		layoutBindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
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
		uint32_t alignment = m_VulkanDevice->GetLimits().minUniformBufferOffsetAlignment;
		uint32_t modelAlign = Align(sizeof(ModelBlock), alignment);

		if (m_AutoRotate)
		{
			for (int32_t i = 0; i < m_Model->meshes.size(); ++i)
			{
				ModelBlock* modelBlock = (ModelBlock*)(m_ModelDatas.data() + modelAlign * i);
				modelBlock->model.AppendRotation(45.0f * delta, Vector3::UpVector);
			}
			m_ModelBuffer->CopyFrom(m_ModelDatas.data(), m_ModelBuffer->size);
		}

		m_ViewProjData.view = m_ViewCamera.GetView();
		m_ViewProjData.projection = m_ViewCamera.GetProjection();

		m_ViewProjBuffer->CopyFrom(&m_ViewProjData, sizeof(ViewProjectionBlock));

		m_ParamBuffer->CopyFrom(&m_ParamData, m_ParamBuffer->size);
	}

	void CreateUniformBuffers()
	{
		uint32_t alignment = m_VulkanDevice->GetLimits().minUniformBufferOffsetAlignment;
		// world matrix dynamicbuffer
		uint32_t modelAlign = Align(sizeof(ModelBlock), alignment);
		m_ModelDatas.resize(modelAlign * m_Model->meshes.size());
		for (int32_t i = 0; i < m_Model->meshes.size(); ++i)
		{
			ModelBlock* modelBlock = (ModelBlock*)(m_ModelDatas.data() + modelAlign * i);
			modelBlock->model = m_Model->meshes[i]->linkNode->GetGlobalMatrix();
		}

		m_ModelBuffer = VKBuffer::CreateBuffer(
			m_VulkanDevice,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_ModelDatas.size(),
			m_ModelDatas.data()
		);
		m_ModelBuffer->Map();

		m_ViewCamera.Perspective(PI / 4, m_configuration.window.windowWidth, m_configuration.window.windowHeight, 1.0f, 3000.0f);
		m_ViewCamera.SetPosition(0.0f, 10.0f, -10.0f);
		m_ViewCamera.LookAt(0, 0, 0);

		m_ViewProjBuffer = VKBuffer::CreateBuffer(
			m_VulkanDevice,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			sizeof(ViewProjectionBlock),
			&(m_ViewProjData)
		);
		m_ViewProjBuffer->Map();

		// params
		m_ParamData.step = 4;
		m_ParamData.debug = 0;
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
		m_ViewProjBuffer->UnMap();
		delete m_ViewProjBuffer;
		m_ViewProjBuffer = nullptr;

		m_ModelBuffer->UnMap();
		delete m_ModelBuffer;
		m_ModelBuffer = nullptr;

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

	bool							m_AutoRotate = true;
	bool 							m_Ready = false;

	VKCamera						m_ViewCamera;

	std::vector<uint8_t>              m_ModelDatas;
	VKBuffer* m_ModelBuffer = nullptr;

	ParamBlock                      m_ParamData;
	VKBuffer* m_ParamBuffer = nullptr;

	VKBuffer* m_ViewProjBuffer = nullptr;
	ViewProjectionBlock				m_ViewProjData;

	VKGfxPipeline* m_Pipeline = nullptr;
	VKTexture* m_Texture = nullptr;
	VKModel* m_Model = nullptr;

	VkDescriptorSetLayout 			m_DescriptorSetLayout = VK_NULL_HANDLE;
	VkPipelineLayout 				m_PipelineLayout = VK_NULL_HANDLE;
	VkDescriptorPool                m_DescriptorPool = VK_NULL_HANDLE;
	VkDescriptorSet 				m_DescriptorSet = VK_NULL_HANDLE;

	ImageGUIContext* m_GUI = nullptr;
};