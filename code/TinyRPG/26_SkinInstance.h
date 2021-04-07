#pragma once

#define INSTANCE_COUNT 8000

class SkinInstance final
{
public:
	SkinInstance(Configuration& configuration) noexcept;

	void StartGame() noexcept;
private:
	SkinInstance() = delete;
	SkinInstance(const SkinInstance&) = delete;
	SkinInstance(SkinInstance&&) = delete;
	SkinInstance operator=(const SkinInstance&) = delete;
	SkinInstance operator=(SkinInstance&&) = delete;

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

	struct ParamDataBlock
	{
		Matrix4x4 model;
		Matrix4x4 view;
		Matrix4x4 projection;
		Vector4 animIndex;
	};

	struct InstanceData
	{
		Vector4 quat0;
		Vector4 quat1;
	};

	void UpdateAnimation(float time, float delta)
	{
		if (m_AutoAnimation) {
			m_AnimTime += delta;
		}

		if (m_AnimTime > m_RoleModel->GetAnimation(0).duration) {
			m_AnimTime = m_AnimTime - m_RoleModel->GetAnimation(0).duration;
		}

		int32_t index = 0;
		for (int32_t i = 0; i < m_Keys.size(); ++i) {
			if (m_AnimTime <= m_Keys[i]) {
				index = i;
				break;
			}
		}

		VKMesh* mesh = m_RoleModel->meshes[0];

		m_ParamData.animIndex.x = m_AnimTexture->width;
		m_ParamData.animIndex.y = m_AnimTexture->height;
		m_ParamData.animIndex.z = index * mesh->bones.size() * 2;
		m_ParamData.animIndex.w = m_Keys.size() * mesh->bones.size() * 2;
	}

	void Draw(float time, float delta)
	{
		int32_t bufferIndex = m_defContext->AcquireBackbufferIndex();

		bool hovered = UpdateUI(time, delta);
		if (!hovered)
			m_ViewCamera.Update(time, delta);

		m_ParamData.view = m_ViewCamera.GetView();
		m_ParamData.projection = m_ViewCamera.GetProjection();

		UpdateAnimation(time, delta);

		m_RoleMaterial->BeginFrame();
		VKMesh* mesh = m_RoleModel->meshes[0];
		m_ParamData.model = mesh->linkNode->GetGlobalMatrix();

		m_RoleMaterial->BeginObject();
		m_RoleMaterial->SetLocalUniform("paramData", &m_ParamData, sizeof(ParamDataBlock));
		m_RoleMaterial->EndObject();
		m_RoleMaterial->EndFrame();

		SetupCommandBuffers(bufferIndex);

		m_defContext->Present(bufferIndex);
	}

	bool UpdateUI(float time, float delta)
	{
		m_GUI->StartFrame();

		m_FrameCounter += 1;
		m_LastFrameTime += delta;
		if (m_LastFrameTime >= 1.0f) {
			m_LastFPS = m_FrameCounter;
			m_FrameCounter = 0;
			m_LastFrameTime = 0.0f;
		}

		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("SkinInTextureDemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

			VKPrimitive* primitive = m_RoleModel->meshes[0]->primitives[0];

			ImGui::SliderInt("Instance", &(primitive->indexBuffer->instanceCount), 1, INSTANCE_COUNT);

			ImGui::Checkbox("AutoPlay", &m_AutoAnimation);

			if (!m_AutoAnimation) {
				ImGui::SliderFloat("Time", &m_AnimTime, 0.0f, m_AnimDuration);
			}

			ImGui::Text("DrawCall:1");
			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / m_LastFPS, m_LastFPS);
			ImGui::End();
		}

		bool hovered = ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsRootWindowOrAnyChildHovered();

		m_GUI->EndFrame();
		m_GUI->Update();

		return hovered;
	}

	void SetAnimation(int32_t index)
	{
		m_RoleModel->SetAnimation(index);
		m_AnimDuration = m_RoleModel->animations[index].duration;
		m_AnimTime = 0.0f;
		m_AnimIndex = index;
	}

	void CreateAnimTexture(VKCommandBuffer* cmdBuffer)
	{
		std::vector<float> animData(64 * 64 * 4);
		VKAnimation& animation = m_RoleModel->GetAnimation();

		m_Keys.push_back(0);
		for (auto it = animation.clips.begin(); it != animation.clips.end(); ++it)
		{
			VKAnimationClip& clip = it->second;
			for (int32_t i = 0; i < clip.positions.keys.size(); ++i) {
				if (m_Keys.back() < clip.positions.keys[i]) {
					m_Keys.push_back(clip.positions.keys[i]);
				}
			}
			for (int32_t i = 0; i < clip.rotations.keys.size(); ++i) {
				if (m_Keys.back() < clip.rotations.keys[i]) {
					m_Keys.push_back(clip.rotations.keys[i]);
				}
			}
			for (int32_t i = 0; i < clip.scales.keys.size(); ++i) {
				if (m_Keys.back() < clip.scales.keys[i]) {
					m_Keys.push_back(clip.scales.keys[i]);
				}
			}
		}

		VKMesh* mesh = m_RoleModel->meshes[0];

		for (int32_t i = 0; i < m_Keys.size(); ++i)
		{
			m_RoleModel->GotoAnimation(m_Keys[i]);
			int32_t step = i * mesh->bones.size() * 8;

			for (int32_t j = 0; j < mesh->bones.size(); ++j)
			{
				int32_t boneIndex = mesh->bones[j];
				VKBone* bone = m_RoleModel->bones[boneIndex];
				Matrix4x4 boneTransform = bone->finalTransform;
				boneTransform.Append(mesh->linkNode->GetGlobalMatrix().Inverse());
				boneTransform.AppendRotation(180, Vector3::ForwardVector);
				Quat quat = boneTransform.ToQuat();
				Vector3 pos = boneTransform.GetOrigin();
				float dx = (+0.5) * (pos.x * quat.w + pos.y * quat.z - pos.z * quat.y);
				float dy = (+0.5) * (-pos.x * quat.z + pos.y * quat.w + pos.z * quat.x);
				float dz = (+0.5) * (pos.x * quat.y - pos.y * quat.x + pos.z * quat.w);
				float dw = (-0.5) * (pos.x * quat.x + pos.y * quat.y + pos.z * quat.z);
				int32_t index = step + j * 8;
				animData[index + 0] = quat.x;
				animData[index + 1] = quat.y;
				animData[index + 2] = quat.z;
				animData[index + 3] = quat.w;
				animData[index + 4] = dx;
				animData[index + 5] = dy;
				animData[index + 6] = dz;
				animData[index + 7] = dw;
			}
		}

		m_AnimTexture = VKTexture::Create2D(
			(const uint8_t*)animData.data(), animData.size() * sizeof(float), VK_FORMAT_R32G32B32A32_SFLOAT,
			64, 64,
			m_VulkanDevice,
			cmdBuffer
		);
		m_AnimTexture->UpdateSampler(
			VK_FILTER_NEAREST,
			VK_FILTER_NEAREST,
			VK_SAMPLER_MIPMAP_MODE_NEAREST,
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
		);
	}

	void LoadAssets()
	{
		VKCommandBuffer* cmdBuffer = VKCommandBuffer::Create(m_VulkanDevice, m_defContext->m_CommandPool);

		// shader
		m_RoleShader = VKShader::Create(
			m_VulkanDevice,
			true,
			"data/shaders/26_SkinInstance/obj.vert.spv",
			"data/shaders/26_SkinInstance/obj.frag.spv"
		);

		// model
		m_RoleModel = VKModel::LoadFromFile(
			"data/models/xiaonan/nvhai.fbx",
			m_VulkanDevice,
			cmdBuffer,
			{
				 VertexAttribute::VA_Position,
				VertexAttribute::VA_UV0,
				VertexAttribute::VA_Normal,
				VertexAttribute::VA_SkinPack,
			}
		);
		m_RoleModel->rootNode->localMatrix.AppendRotation(180, Vector3::UpVector);

		// animation
		SetAnimation(0);
		CreateAnimTexture(cmdBuffer);

		// instance data
		VKMesh* mesh = m_RoleModel->meshes[0];
		Matrix4x4 meshGlobal = mesh->linkNode->GetGlobalMatrix();
		VKPrimitive* primitive = m_RoleModel->meshes[0]->primitives[0];
		primitive->instanceDatas.resize(9 * INSTANCE_COUNT);

		for (int32_t i = 0; i < INSTANCE_COUNT; ++i)
		{
			Vector3 translate;
			translate.x = math::RandRange(-300.0f, 300.0f);
			translate.y = math::RandRange(-180.0f, 180.0f);
			translate.z = math::RandRange(-150.0f, 150.0f);

			Matrix4x4 matrix = meshGlobal;
			matrix.AppendRotation(math::RandRange(0.0f, 360.0f), Vector3::UpVector);
			matrix.AppendTranslation(translate);

			Quat quat = matrix.ToQuat();
			Vector3 pos = matrix.GetOrigin();
			float dx = (+0.5) * (pos.x * quat.w + pos.y * quat.z - pos.z * quat.y);
			float dy = (+0.5) * (-pos.x * quat.z + pos.y * quat.w + pos.z * quat.x);
			float dz = (+0.5) * (pos.x * quat.y - pos.y * quat.x + pos.z * quat.w);
			float dw = (-0.5) * (pos.x * quat.x + pos.y * quat.y + pos.z * quat.z);

			int32_t index = i * 9;
			primitive->instanceDatas[index + 0] = quat.x;
			primitive->instanceDatas[index + 1] = quat.y;
			primitive->instanceDatas[index + 2] = quat.z;
			primitive->instanceDatas[index + 3] = quat.w;
			primitive->instanceDatas[index + 4] = dx;
			primitive->instanceDatas[index + 5] = dy;
			primitive->instanceDatas[index + 6] = dz;
			primitive->instanceDatas[index + 7] = dw;
			primitive->instanceDatas[index + 8] = math::RandRange(0, m_Keys.size()) * mesh->bones.size() * 2;
		}

		primitive->indexBuffer->instanceCount = 1024;
		primitive->instanceBuffer = VKVertexBuffer::Create(m_VulkanDevice, cmdBuffer, primitive->instanceDatas, m_RoleShader->instancesAttributes);

		// texture
		m_RoleDiffuse = VKTexture::Create2D(
			"data/models/xiaonan/b001.jpg",
			m_VulkanDevice,
			cmdBuffer
		);

		// material
		m_RoleMaterial = VKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_RoleShader
		);
		m_RoleMaterial->PreparePipeline();
		m_RoleMaterial->SetTexture("diffuseMap", m_RoleDiffuse);
		m_RoleMaterial->SetTexture("animMap", m_AnimTexture);

		delete cmdBuffer;
	}

	void DestroyAssets()
	{
		delete m_RoleShader;
		delete m_RoleDiffuse;
		delete m_RoleMaterial;
		delete m_RoleModel;
		delete m_AnimTexture;
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

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_RoleMaterial->GetPipeline());
		m_RoleMaterial->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
		m_RoleModel->meshes[0]->BindDrawCmd(commandBuffer);

		m_GUI->BindDrawCmd(commandBuffer, m_RenderPass);

		vkCmdEndRenderPass(commandBuffer);

		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));
	}

	void InitParmas()
	{
		VKBoundingBox bounds = m_RoleModel->rootNode->GetBounds();
		Vector3 boundSize = bounds.max - bounds.min;
		Vector3 boundCenter = bounds.min + boundSize * 0.5f;

		m_ViewCamera.SetPosition(boundCenter.x, boundCenter.y, boundCenter.z - boundSize.Size() * 2.0);
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

	bool							m_AutoRotate = true;
	bool 							m_Ready = false;

	VKCamera						m_ViewCamera;

	ParamDataBlock				m_ParamData;

	VKModel* m_RoleModel = nullptr;
	VKShader* m_RoleShader = nullptr;
	VKTexture* m_RoleDiffuse = nullptr;
	VKMaterial* m_RoleMaterial = nullptr;

	ImageGUIContext* m_GUI = nullptr;

	VKTexture* m_AnimTexture = nullptr;
	std::vector<float>			m_Keys;
	bool                        m_AutoAnimation = true;
	float                       m_AnimDuration = 0.0f;
	float                       m_AnimTime = 0.0f;
	int32_t                       m_AnimIndex = 0;

	int32_t						m_FrameCounter = 0;
	float						m_LastFrameTime = 0.0f;
	float						m_LastFPS = 0.0f;
};