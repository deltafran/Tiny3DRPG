#pragma once

struct GPUBuffer
{
	VkDeviceMemory 	memory = VK_NULL_HANDLE;
	VkBuffer 		buffer = VK_NULL_HANDLE;
};

typedef GPUBuffer 	IndexBuffer;
typedef GPUBuffer 	VertexBuffer;
typedef GPUBuffer 	UBOBuffer;

struct Vertex
{
	float position[3];
	float color[3];
};

struct UBOData
{
	Matrix4x4 model;
	Matrix4x4 view;
	Matrix4x4 projection;
};

class GameApplication final
{
public:
	GameApplication(Configuration &configuration) noexcept;

	void StartGame() noexcept;
private:
	GameApplication() = delete;
	GameApplication(const GameApplication&) = delete;
	GameApplication(GameApplication&&) = delete;
	GameApplication operator=(const GameApplication&) = delete;
	GameApplication operator=(GameApplication&&) = delete;

	bool init() noexcept;
	void update() noexcept;
	void draw() noexcept;
	void close() noexcept;
	bool isEnd() const noexcept { return m_isEnd || m_engine.IsEnd(); }

	VkShaderModule LoadSPIPVShader(const std::string& filepath)
	{
		VkDevice device = m_vulkanRHI->GetDevice()->GetInstanceHandle();

		uint8_t* dataPtr = nullptr;
		uint32_t dataSize = 0;
		FileManager::ReadFile(filepath, dataPtr, dataSize);

		VkShaderModuleCreateInfo moduleCreateInfo;
		ZeroVulkanStruct(moduleCreateInfo, VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO);
		moduleCreateInfo.codeSize = dataSize;
		moduleCreateInfo.pCode = (uint32_t*)dataPtr;

		VkShaderModule shaderModule;
		VERIFYVULKANRESULT(vkCreateShaderModule(device, &moduleCreateInfo, VULKAN_CPU_ALLOCATOR, &shaderModule));
		delete[] dataPtr;

		return shaderModule;
	}

	void CreateDescriptorSet()
	{
		VkDevice device = m_vulkanRHI->GetDevice()->GetInstanceHandle();

		VkDescriptorSetAllocateInfo allocInfo;
		ZeroVulkanStruct(allocInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO);
		allocInfo.descriptorPool = m_DescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &m_DescriptorSetLayout;
		VERIFYVULKANRESULT(vkAllocateDescriptorSets(device, &allocInfo, &m_DescriptorSet));

		VkWriteDescriptorSet writeDescriptorSet;
		ZeroVulkanStruct(writeDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
		writeDescriptorSet.dstSet = m_DescriptorSet;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet.pBufferInfo = &m_MVPDescriptor;
		writeDescriptorSet.dstBinding = 0;
		vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
	}

	void CreateDescriptorPool()
	{
		VkDevice device = m_vulkanRHI->GetDevice()->GetInstanceHandle();

		VkDescriptorPoolSize poolSize = {};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = 1;

		VkDescriptorPoolCreateInfo descriptorPoolInfo;
		ZeroVulkanStruct(descriptorPoolInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
		descriptorPoolInfo.poolSizeCount = 1;
		descriptorPoolInfo.pPoolSizes = &poolSize;
		descriptorPoolInfo.maxSets = 1;
		VERIFYVULKANRESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorPool));
	}

	void DestroyDescriptorPool()
	{
		vkDestroyDescriptorPool(m_vulkanRHI->GetDevice()->GetInstanceHandle(), m_DescriptorPool, VULKAN_CPU_ALLOCATOR);
	}

	void CreatePipelines()
	{
		VkDevice device = m_vulkanRHI->GetDevice()->GetInstanceHandle();

		VkPipelineCacheCreateInfo createInfo;
		ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO);
		VERIFYVULKANRESULT(vkCreatePipelineCache(device, &createInfo, VULKAN_CPU_ALLOCATOR, &m_PipelineCache));

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
		ZeroVulkanStruct(inputAssemblyState, VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO);
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		VkPipelineRasterizationStateCreateInfo rasterizationState;
		ZeroVulkanStruct(rasterizationState, VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO);
		rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationState.cullMode = VK_CULL_MODE_NONE;
		rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationState.depthClampEnable = VK_FALSE;
		rasterizationState.rasterizerDiscardEnable = VK_FALSE;
		rasterizationState.depthBiasEnable = VK_FALSE;
		rasterizationState.lineWidth = 1.0f;

		VkPipelineColorBlendAttachmentState blendAttachmentState[1] = {};
		blendAttachmentState[0].colorWriteMask = (
			VK_COLOR_COMPONENT_R_BIT |
			VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT
			);
		blendAttachmentState[0].blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlendState;
		ZeroVulkanStruct(colorBlendState, VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO);
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments = blendAttachmentState;

		VkPipelineViewportStateCreateInfo viewportState;
		ZeroVulkanStruct(viewportState, VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO);
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		std::vector<VkDynamicState> dynamicStateEnables;
		dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
		dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);
		VkPipelineDynamicStateCreateInfo dynamicState;
		ZeroVulkanStruct(dynamicState, VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO);
		dynamicState.dynamicStateCount = (uint32_t)dynamicStateEnables.size();
		dynamicState.pDynamicStates = dynamicStateEnables.data();

		VkPipelineDepthStencilStateCreateInfo depthStencilState;
		ZeroVulkanStruct(depthStencilState, VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO);
		depthStencilState.depthTestEnable = VK_TRUE;
		depthStencilState.depthWriteEnable = VK_TRUE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
		depthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
		depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
		depthStencilState.stencilTestEnable = VK_FALSE;
		depthStencilState.front = depthStencilState.back;

		VkPipelineMultisampleStateCreateInfo multisampleState;
		ZeroVulkanStruct(multisampleState, VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO);
		multisampleState.rasterizationSamples = m_engine.GetRendererSystem().GetVulkanContext().GetSampleCount();
		multisampleState.pSampleMask = nullptr;

		// (triangle.vert):
		// layout (location = 0) in vec3 inPos;
		// layout (location = 1) in vec3 inColor;
		// Attribute location 0: Position
		// Attribute location 1: Color
		// vertex input bindding
		VkVertexInputBindingDescription vertexInputBinding = {};
		vertexInputBinding.binding = 0; // Vertex Buffer 0
		vertexInputBinding.stride = sizeof(Vertex); // Position + Color
		vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		std::vector<VkVertexInputAttributeDescription> vertexInputAttributs(2);
		// position
		vertexInputAttributs[0].binding = 0;
		vertexInputAttributs[0].location = 0; // triangle.vert : layout (location = 0)
		vertexInputAttributs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexInputAttributs[0].offset = 0;
		// color
		vertexInputAttributs[1].binding = 0;
		vertexInputAttributs[1].location = 1; // triangle.vert : layout (location = 1)
		vertexInputAttributs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexInputAttributs[1].offset = 12;

		VkPipelineVertexInputStateCreateInfo vertexInputState;
		ZeroVulkanStruct(vertexInputState, VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO);
		vertexInputState.vertexBindingDescriptionCount = 1;
		vertexInputState.pVertexBindingDescriptions = &vertexInputBinding;
		vertexInputState.vertexAttributeDescriptionCount = 2;
		vertexInputState.pVertexAttributeDescriptions = vertexInputAttributs.data();

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages(2);
		ZeroVulkanStruct(shaderStages[0], VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
		ZeroVulkanStruct(shaderStages[1], VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].module = LoadSPIPVShader("data/shaders/2_Triangle/triangle.vert.spv");
		shaderStages[0].pName = "main";
		shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].module = LoadSPIPVShader("data/shaders/2_Triangle/triangle.frag.spv");
		shaderStages[1].pName = "main";

		VkGraphicsPipelineCreateInfo pipelineCreateInfo;
		ZeroVulkanStruct(pipelineCreateInfo, VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO);
		pipelineCreateInfo.layout = m_PipelineLayout;
		pipelineCreateInfo.renderPass = m_engine.GetRendererSystem().GetVulkanContext().GetRenderPass();
		pipelineCreateInfo.stageCount = (uint32_t)shaderStages.size();
		pipelineCreateInfo.pStages = shaderStages.data();
		pipelineCreateInfo.pVertexInputState = &vertexInputState;
		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineCreateInfo.pRasterizationState = &rasterizationState;
		pipelineCreateInfo.pColorBlendState = &colorBlendState;
		pipelineCreateInfo.pMultisampleState = &multisampleState;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pDepthStencilState = &depthStencilState;
		pipelineCreateInfo.pDynamicState = &dynamicState;
		VERIFYVULKANRESULT(vkCreateGraphicsPipelines(device, m_PipelineCache, 1, &pipelineCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Pipeline));

		vkDestroyShaderModule(device, shaderStages[0].module, VULKAN_CPU_ALLOCATOR);
		vkDestroyShaderModule(device, shaderStages[1].module, VULKAN_CPU_ALLOCATOR);
	}

	void DestroyPipelines()
	{
		VkDevice device = m_vulkanRHI->GetDevice()->GetInstanceHandle();
		vkDestroyPipeline(device, m_Pipeline, VULKAN_CPU_ALLOCATOR);
		vkDestroyPipelineCache(device, m_PipelineCache, VULKAN_CPU_ALLOCATOR);
	}

	void CreateDescriptorSetLayout()
	{
		VkDevice device = m_vulkanRHI->GetDevice()->GetInstanceHandle();

		VkDescriptorSetLayoutBinding layoutBinding;
		layoutBinding.binding = 0;
		layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBinding.descriptorCount = 1;
		layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		layoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo descSetLayoutInfo;
		ZeroVulkanStruct(descSetLayoutInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
		descSetLayoutInfo.bindingCount = 1;
		descSetLayoutInfo.pBindings = &layoutBinding;
		VERIFYVULKANRESULT(vkCreateDescriptorSetLayout(device, &descSetLayoutInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorSetLayout));

		VkPipelineLayoutCreateInfo pipeLayoutInfo;
		ZeroVulkanStruct(pipeLayoutInfo, VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);
		pipeLayoutInfo.setLayoutCount = 1;
		pipeLayoutInfo.pSetLayouts = &m_DescriptorSetLayout;
		VERIFYVULKANRESULT(vkCreatePipelineLayout(device, &pipeLayoutInfo, VULKAN_CPU_ALLOCATOR, &m_PipelineLayout));
	}

	void DestroyCommandBuffers()
	{
		VkDevice device = m_vulkanRHI->GetDevice()->GetInstanceHandle();
		for (size_t i = 0; i < m_CommandBuffers.size(); ++i) {
			vkFreeCommandBuffers(device, m_CommandPool, 1, &(m_CommandBuffers[i]));
		}

		vkDestroyCommandPool(device, m_CommandPool, VULKAN_CPU_ALLOCATOR);
	}

	void CreateCommandBuffers()
	{
		VkDevice device = m_vulkanRHI->GetDevice()->GetInstanceHandle();

		VkCommandPoolCreateInfo cmdPoolInfo;
		ZeroVulkanStruct(cmdPoolInfo, VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO);
		cmdPoolInfo.queueFamilyIndex = m_vulkanRHI->GetDevice()->GetPresentQueue()->GetFamilyIndex();
		cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		VERIFYVULKANRESULT(vkCreateCommandPool(device, &cmdPoolInfo, VULKAN_CPU_ALLOCATOR, &m_CommandPool));

		VkCommandBufferAllocateInfo cmdBufferInfo;
		ZeroVulkanStruct(cmdBufferInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);
		cmdBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBufferInfo.commandBufferCount = 1;
		cmdBufferInfo.commandPool = m_CommandPool;

		m_CommandBuffers.resize(m_vulkanRHI->GetSwapChain()->GetBackBufferCount());
		for (size_t i = 0; i < m_CommandBuffers.size(); ++i) {
			vkAllocateCommandBuffers(device, &cmdBufferInfo, &(m_CommandBuffers[i]));
		}
	}

	void DestroyDescriptorSetLayout()
	{
		VkDevice device = m_vulkanRHI->GetDevice()->GetInstanceHandle();
		vkDestroyDescriptorSetLayout(device, m_DescriptorSetLayout, VULKAN_CPU_ALLOCATOR);
		vkDestroyPipelineLayout(device, m_PipelineLayout, VULKAN_CPU_ALLOCATOR);
	}

	void UpdateUniformBuffers(float time, float delta)
	{
		VkDevice device = m_vulkanRHI->GetDevice()->GetInstanceHandle();
		m_MVPData.model.AppendRotation(90.0f * delta, Vector3::UpVector);
		uint8_t* pData = nullptr;
		VERIFYVULKANRESULT(vkMapMemory(device, m_MVPBuffer.memory, 0, sizeof(UBOData), 0, (void**)&pData));
		std::memcpy(pData, &m_MVPData, sizeof(UBOData));
		vkUnmapMemory(device, m_MVPBuffer.memory);
	}

	void CreateUniformBuffers()
	{
		VkDevice device = m_vulkanRHI->GetDevice()->GetInstanceHandle();

		VkBufferCreateInfo bufferInfo;
		ZeroVulkanStruct(bufferInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
		bufferInfo.size = sizeof(UBOData);
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		VERIFYVULKANRESULT(vkCreateBuffer(device, &bufferInfo, VULKAN_CPU_ALLOCATOR, &m_MVPBuffer.buffer));

		VkMemoryRequirements memReqInfo;
		vkGetBufferMemoryRequirements(device, m_MVPBuffer.buffer, &memReqInfo);
		uint32_t memoryTypeIndex = 0;
		m_vulkanRHI->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memReqInfo.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &memoryTypeIndex);

		VkMemoryAllocateInfo allocInfo;
		ZeroVulkanStruct(allocInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);
		allocInfo.allocationSize = memReqInfo.size;
		allocInfo.memoryTypeIndex = memoryTypeIndex;
		VERIFYVULKANRESULT(vkAllocateMemory(device, &allocInfo, VULKAN_CPU_ALLOCATOR, &m_MVPBuffer.memory));
		VERIFYVULKANRESULT(vkBindBufferMemory(device, m_MVPBuffer.buffer, m_MVPBuffer.memory, 0));

		m_MVPDescriptor.buffer = m_MVPBuffer.buffer;
		m_MVPDescriptor.offset = 0;
		m_MVPDescriptor.range = sizeof(UBOData);

		m_MVPData.model.SetIdentity();
		m_MVPData.model.SetOrigin(Vector3(0, 0, 0));

		m_MVPData.view.SetIdentity();
		m_MVPData.view.SetOrigin(Vector4(0, 0, -2.5f));
		m_MVPData.view.SetInverse();

		m_MVPData.projection.SetIdentity();
		m_MVPData.projection.Perspective(math::DegreesToRadians(75.0f), (float)m_configuration.window.windowWidth, (float)m_configuration.window.windowHeight, 0.01f, 3000.0f);
	}

	void DestroyUniformBuffers()
	{
		VkDevice device = m_vulkanRHI->GetDevice()->GetInstanceHandle();

		vkDestroyBuffer(device, m_MVPBuffer.buffer, VULKAN_CPU_ALLOCATOR);
		vkFreeMemory(device, m_MVPBuffer.memory, VULKAN_CPU_ALLOCATOR);
	}

	void CreateMeshBuffers()
	{
		VkDevice device = m_vulkanRHI->GetDevice()->GetInstanceHandle();
		VkQueue queue = m_vulkanRHI->GetDevice()->GetPresentQueue()->GetHandle();

		std::vector<Vertex> vertices = {
			{ {  1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
			{ { -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
			{ {  0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
		};

		std::vector<uint16_t> indices = { 0, 1, 2 };
		m_IndicesCount = (uint32_t)indices.size();

		VertexBuffer tempVertexBuffer;
		IndexBuffer  tempIndexBuffer;

		void* dataPtr = nullptr;
		VkMemoryRequirements memReqInfo;
		VkMemoryAllocateInfo memAllocInfo;
		ZeroVulkanStruct(memAllocInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);

		// vertex buffer
		VkBufferCreateInfo vertexBufferInfo;
		ZeroVulkanStruct(vertexBufferInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
		vertexBufferInfo.size = vertices.size() * sizeof(Vertex);
		vertexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VERIFYVULKANRESULT(vkCreateBuffer(device, &vertexBufferInfo, VULKAN_CPU_ALLOCATOR, &tempVertexBuffer.buffer));

		vkGetBufferMemoryRequirements(device, tempVertexBuffer.buffer, &memReqInfo);
		uint32_t memoryTypeIndex = 0;
		m_vulkanRHI->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memReqInfo.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &memoryTypeIndex);
		memAllocInfo.allocationSize = memReqInfo.size;
		memAllocInfo.memoryTypeIndex = memoryTypeIndex;
		VERIFYVULKANRESULT(vkAllocateMemory(device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &tempVertexBuffer.memory));
		VERIFYVULKANRESULT(vkBindBufferMemory(device, tempVertexBuffer.buffer, tempVertexBuffer.memory, 0));

		VERIFYVULKANRESULT(vkMapMemory(device, tempVertexBuffer.memory, 0, memAllocInfo.allocationSize, 0, &dataPtr));
		std::memcpy(dataPtr, vertices.data(), vertexBufferInfo.size);
		vkUnmapMemory(device, tempVertexBuffer.memory);

		// local device vertex buffer
		vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		VERIFYVULKANRESULT(vkCreateBuffer(device, &vertexBufferInfo, VULKAN_CPU_ALLOCATOR, &m_VertexBuffer.buffer));

		vkGetBufferMemoryRequirements(device, m_VertexBuffer.buffer, &memReqInfo);
		m_vulkanRHI->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memReqInfo.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryTypeIndex);
		memAllocInfo.allocationSize = memReqInfo.size;
		memAllocInfo.memoryTypeIndex = memoryTypeIndex;
		VERIFYVULKANRESULT(vkAllocateMemory(device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &m_VertexBuffer.memory));
		VERIFYVULKANRESULT(vkBindBufferMemory(device, m_VertexBuffer.buffer, m_VertexBuffer.memory, 0));

		// index buffer
		VkBufferCreateInfo indexBufferInfo;
		ZeroVulkanStruct(indexBufferInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
		indexBufferInfo.size = m_IndicesCount * sizeof(uint16_t);
		indexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VERIFYVULKANRESULT(vkCreateBuffer(device, &indexBufferInfo, VULKAN_CPU_ALLOCATOR, &tempIndexBuffer.buffer));

		vkGetBufferMemoryRequirements(device, tempIndexBuffer.buffer, &memReqInfo);
		m_vulkanRHI->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memReqInfo.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &memoryTypeIndex);
		memAllocInfo.allocationSize = memReqInfo.size;
		memAllocInfo.memoryTypeIndex = memoryTypeIndex;
		VERIFYVULKANRESULT(vkAllocateMemory(device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &tempIndexBuffer.memory));
		VERIFYVULKANRESULT(vkBindBufferMemory(device, tempIndexBuffer.buffer, tempIndexBuffer.memory, 0));

		VERIFYVULKANRESULT(vkMapMemory(device, tempIndexBuffer.memory, 0, memAllocInfo.allocationSize, 0, &dataPtr));
		std::memcpy(dataPtr, indices.data(), indexBufferInfo.size);
		vkUnmapMemory(device, tempIndexBuffer.memory);

		indexBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		VERIFYVULKANRESULT(vkCreateBuffer(device, &indexBufferInfo, VULKAN_CPU_ALLOCATOR, &m_IndicesBuffer.buffer));

		vkGetBufferMemoryRequirements(device, m_IndicesBuffer.buffer, &memReqInfo);
		m_vulkanRHI->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memReqInfo.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryTypeIndex);
		memAllocInfo.allocationSize = memReqInfo.size;
		memAllocInfo.memoryTypeIndex = memoryTypeIndex;
		VERIFYVULKANRESULT(vkAllocateMemory(device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &m_IndicesBuffer.memory));
		VERIFYVULKANRESULT(vkBindBufferMemory(device, m_IndicesBuffer.buffer, m_IndicesBuffer.memory, 0));

		VkCommandBuffer xferCmdBuffer;
		VkCommandBufferAllocateInfo xferCmdBufferInfo;
		ZeroVulkanStruct(xferCmdBufferInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);
		xferCmdBufferInfo.commandPool = m_CommandPool;
		xferCmdBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		xferCmdBufferInfo.commandBufferCount = 1;
		VERIFYVULKANRESULT(vkAllocateCommandBuffers(device, &xferCmdBufferInfo, &xferCmdBuffer));

		VkCommandBufferBeginInfo cmdBufferBeginInfo;
		ZeroVulkanStruct(cmdBufferBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
		VERIFYVULKANRESULT(vkBeginCommandBuffer(xferCmdBuffer, &cmdBufferBeginInfo));

		VkBufferCopy copyRegion = {};
		copyRegion.size = vertices.size() * sizeof(Vertex);
		vkCmdCopyBuffer(xferCmdBuffer, tempVertexBuffer.buffer, m_VertexBuffer.buffer, 1, &copyRegion);

		copyRegion.size = indices.size() * sizeof(uint16_t);
		vkCmdCopyBuffer(xferCmdBuffer, tempIndexBuffer.buffer, m_IndicesBuffer.buffer, 1, &copyRegion);

		VERIFYVULKANRESULT(vkEndCommandBuffer(xferCmdBuffer));

		VkSubmitInfo submitInfo;
		ZeroVulkanStruct(submitInfo, VK_STRUCTURE_TYPE_SUBMIT_INFO);
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &xferCmdBuffer;

		VkFenceCreateInfo fenceInfo;
		ZeroVulkanStruct(fenceInfo, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
		fenceInfo.flags = 0;

		VkFence fence = VK_NULL_HANDLE;
		VERIFYVULKANRESULT(vkCreateFence(device, &fenceInfo, VULKAN_CPU_ALLOCATOR, &fence));
		VERIFYVULKANRESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));
		VERIFYVULKANRESULT(vkWaitForFences(device, 1, &fence, VK_TRUE, MAX_int64));

		vkDestroyFence(device, fence, VULKAN_CPU_ALLOCATOR);
		vkFreeCommandBuffers(device, m_CommandPool, 1, &xferCmdBuffer);

		vkDestroyBuffer(device, tempVertexBuffer.buffer, VULKAN_CPU_ALLOCATOR);
		vkFreeMemory(device, tempVertexBuffer.memory, VULKAN_CPU_ALLOCATOR);
		vkDestroyBuffer(device, tempIndexBuffer.buffer, VULKAN_CPU_ALLOCATOR);
		vkFreeMemory(device, tempIndexBuffer.memory, VULKAN_CPU_ALLOCATOR);
	}

	void DestroyMeshBuffers()
	{
		VkDevice device = m_vulkanRHI->GetDevice()->GetInstanceHandle();
		vkDestroyBuffer(device, m_VertexBuffer.buffer, VULKAN_CPU_ALLOCATOR);
		vkFreeMemory(device, m_VertexBuffer.memory, VULKAN_CPU_ALLOCATOR);
		vkDestroyBuffer(device, m_IndicesBuffer.buffer, VULKAN_CPU_ALLOCATOR);
		vkFreeMemory(device, m_IndicesBuffer.memory, VULKAN_CPU_ALLOCATOR);
	}

	void CreateSemaphores()
	{
		VkDevice device = m_vulkanRHI->GetDevice()->GetInstanceHandle();
		VkSemaphoreCreateInfo createInfo;
		ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);
		vkCreateSemaphore(device, &createInfo, VULKAN_CPU_ALLOCATOR, &m_RenderComplete);
	}

	void DestorySemaphores()
	{
		VkDevice device = m_vulkanRHI->GetDevice()->GetInstanceHandle();
		vkDestroySemaphore(device, m_RenderComplete, VULKAN_CPU_ALLOCATOR);
	}

	void CreateFences()
	{
		VkDevice device = m_vulkanRHI->GetDevice()->GetInstanceHandle();
		int32_t frameCount = m_vulkanRHI->GetSwapChain()->GetBackBufferCount();

		VkFenceCreateInfo fenceCreateInfo;
		ZeroVulkanStruct(fenceCreateInfo, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		m_Fences.resize(frameCount);
		for (int32_t i = 0; i < m_Fences.size(); ++i) {
			VERIFYVULKANRESULT(vkCreateFence(device, &fenceCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Fences[i]));
		}
	}

	void DestroyFences()
	{
		VkDevice device = m_vulkanRHI->GetDevice()->GetInstanceHandle();
		for (int32_t i = 0; i < m_Fences.size(); ++i) {
			vkDestroyFence(device, m_Fences[i], VULKAN_CPU_ALLOCATOR);
		}
	}

	void SetupCommandBuffers()
	{
		VkCommandBufferBeginInfo cmdBeginInfo;
		ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);

		VkClearValue clearValues[2];
		clearValues[0].color = { {0.2f, 0.2f, 0.2f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		int32_t fwidth = m_vulkanRHI->GetSwapChain()->GetWidth();
		int32_t fheight = m_vulkanRHI->GetSwapChain()->GetHeight();

		VkRenderPassBeginInfo renderPassBeginInfo;
		ZeroVulkanStruct(renderPassBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
		renderPassBeginInfo.renderPass = m_engine.GetRendererSystem().GetVulkanContext().GetRenderPass();
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = clearValues;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = fwidth;
		renderPassBeginInfo.renderArea.extent.height = fheight;

		for (int32_t i = 0; i < m_CommandBuffers.size(); ++i)
		{
			renderPassBeginInfo.framebuffer = m_engine.GetRendererSystem().GetVulkanContext().GetFrameBuffers()[i];

			VkViewport viewport = {};
			viewport.x = 0;
			viewport.y = fheight;
			viewport.width = (float)fwidth;
			viewport.height = -(float)fheight;    // flip y axis
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkRect2D scissor = {};
			scissor.extent.width = fwidth;
			scissor.extent.height = fheight;
			scissor.offset.x = 0;
			scissor.offset.y = 0;

			VkDeviceSize offsets[1] = { 0 };

			VERIFYVULKANRESULT(vkBeginCommandBuffer(m_CommandBuffers[i], &cmdBeginInfo));

			vkCmdBeginRenderPass(m_CommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdSetViewport(m_CommandBuffers[i], 0, 1, &viewport);
			vkCmdSetScissor(m_CommandBuffers[i], 0, 1, &scissor);
			vkCmdBindDescriptorSets(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_DescriptorSet, 0, nullptr);
			vkCmdBindPipeline(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
			vkCmdBindVertexBuffers(m_CommandBuffers[i], 0, 1, &m_VertexBuffer.buffer, offsets);
			vkCmdBindIndexBuffer(m_CommandBuffers[i], m_IndicesBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
			vkCmdDrawIndexed(m_CommandBuffers[i], m_IndicesCount, 1, 0, 0, 0);
			vkCmdEndRenderPass(m_CommandBuffers[i]);

			VERIFYVULKANRESULT(vkEndCommandBuffer(m_CommandBuffers[i]));
		}
	}

	Configuration& m_configuration;
	Engine m_engine;
	bool m_isEnd = false;

	VulkanRHI* m_vulkanRHI;

	//std::vector<VkFramebuffer>      m_FrameBuffers;

	//VkImage                         m_DepthStencilImage = VK_NULL_HANDLE;
	//VkImageView                     m_DepthStencilView = VK_NULL_HANDLE;
	//VkDeviceMemory                  m_DepthStencilMemory = VK_NULL_HANDLE;

	//VkRenderPass                    m_RenderPass = VK_NULL_HANDLE;
	//VkSampleCountFlagBits           m_SampleCount = VK_SAMPLE_COUNT_1_BIT;
	//PixelFormat                     m_DepthFormat = PF_D24;

	VkCommandPool					m_CommandPool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer>	m_CommandBuffers;

	std::vector<VkFence> 			m_Fences;
	VkSemaphore 					m_PresentComplete = VK_NULL_HANDLE;
	VkSemaphore 					m_RenderComplete = VK_NULL_HANDLE;

	VertexBuffer 					m_VertexBuffer;
	IndexBuffer 					m_IndicesBuffer;
	UBOBuffer 						m_MVPBuffer;
	UBOData 						m_MVPData;

	VkDescriptorBufferInfo 			m_MVPDescriptor;

	VkDescriptorSetLayout 			m_DescriptorSetLayout = VK_NULL_HANDLE;
	VkDescriptorSet 				m_DescriptorSet = VK_NULL_HANDLE;
	VkPipelineLayout 				m_PipelineLayout = VK_NULL_HANDLE;
	VkDescriptorPool                m_DescriptorPool = VK_NULL_HANDLE;

	VkPipeline 						m_Pipeline = VK_NULL_HANDLE;
	VkPipelineCache                 m_PipelineCache = VK_NULL_HANDLE;

	uint32_t 						m_IndicesCount = 0;
};