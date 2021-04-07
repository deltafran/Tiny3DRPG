#pragma once

#pragma once

#define NUM_LIGHTS 64

class SkeletonMatrix4x4 final
{
public:
	SkeletonMatrix4x4(Configuration& configuration) noexcept;

	void StartGame() noexcept;
private:
	SkeletonMatrix4x4() = delete;
	SkeletonMatrix4x4(const SkeletonMatrix4x4&) = delete;
	SkeletonMatrix4x4(SkeletonMatrix4x4&&) = delete;
	SkeletonMatrix4x4 operator=(const SkeletonMatrix4x4&) = delete;
	SkeletonMatrix4x4 operator=(SkeletonMatrix4x4&&) = delete;

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
		Matrix4x4 projection;
	};

#define MAX_BONES 64
	struct BonesTransformBlock
	{
		Matrix4x4 bones[MAX_BONES];
	};

	void Draw(float time, float delta)
	{
		int32_t bufferIndex = m_defContext->AcquireBackbufferIndex();

		bool hovered = UpdateUI(time, delta);
		if (!hovered)
			m_ViewCamera.Update(time, delta);

		m_MVPData.view = m_ViewCamera.GetView();
		m_MVPData.projection = m_ViewCamera.GetProjection();

		UpdateAnimation(time, delta);

		// Room
		// m_RoleModel->rootNode->localMatrix.AppendRotation(delta * 90.0f, Vector3::UpVector);
		m_RoleMaterial->BeginFrame();
		for (int32_t i = 0; i < m_RoleModel->meshes.size(); ++i)
		{
			VKMesh* mesh = m_RoleModel->meshes[i];

			// model data
			m_MVPData.model = mesh->linkNode->GetGlobalMatrix();

			// bones data
			for (int32_t j = 0; j < mesh->bones.size(); ++j)
			{
				int32_t boneIndex = mesh->bones[j];
				VKBone* bone = m_RoleModel->bones[boneIndex];
				m_BonesData.bones[j] = bone->finalTransform;
				m_BonesData.bones[j].Append(mesh->linkNode->GetGlobalMatrix().Inverse());
			}

			if (mesh->bones.size() == 0) {
				m_BonesData.bones[0].SetIdentity();
			}

			m_RoleMaterial->BeginObject();
			m_RoleMaterial->SetLocalUniform("bonesData", &m_BonesData, sizeof(BonesTransformBlock));
			m_RoleMaterial->SetLocalUniform("uboMVP", &m_MVPData, sizeof(ModelViewProjectionBlock));
			m_RoleMaterial->EndObject();
		}
		m_RoleMaterial->EndFrame();

		SetupCommandBuffers(bufferIndex);

		m_defContext->Present(bufferIndex);
	}

	void UpdateAnimation(float time, float delta)
	{
		if (m_AutoAnimation) {
			m_RoleModel->Update(time, delta);
		}
		else {
			m_RoleModel->GotoAnimation(m_AnimTime);
		}
	}

	bool UpdateUI(float time, float delta)
	{
		m_GUI->StartFrame();

		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("SkeletonMatrix4x4Demo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

			if (ImGui::SliderInt("Anim", &m_AnimIndex, 0, m_RoleModel->animations.size() - 1)) {
				SetAnimation(m_AnimIndex);
			}

			ImGui::SliderFloat("Speed", &(m_RoleModel->GetAnimation().speed), 0.0f, 10.0f);

			ImGui::Checkbox("AutoPlay", &m_AutoAnimation);

			if (!m_AutoAnimation) {
				ImGui::SliderFloat("Time", &m_AnimTime, 0.0f, m_AnimDuration);
			}

			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
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

	void LoadAssets()
	{
		VKCommandBuffer* cmdBuffer = VKCommandBuffer::Create(m_VulkanDevice, m_defContext->m_CommandPool);

		// model
		m_RoleModel = VKModel::LoadFromFile(
			"data/models/xiaonan/nvhai.fbx",
			m_VulkanDevice,
			cmdBuffer,
			{
				VertexAttribute::VA_Position,
				VertexAttribute::VA_UV0,
				VertexAttribute::VA_Normal,
				VertexAttribute::VA_SkinIndex,
				VertexAttribute::VA_SkinWeight
			}
		);
		m_RoleModel->rootNode->localMatrix.AppendRotation(180, Vector3::UpVector);

		SetAnimation(0);

		// shader
		m_RoleShader = VKShader::Create(
			m_VulkanDevice,
			true,
			"data/shaders/22_SkeletonMatrix4x4/obj.vert.spv",
			"data/shaders/22_SkeletonMatrix4x4/obj.frag.spv"
		);

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

		delete cmdBuffer;
	}

	void DestroyAssets()
	{
		delete m_RoleShader;
		delete m_RoleDiffuse;
		delete m_RoleMaterial;
		delete m_RoleModel;
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
		for (int32_t j = 0; j < m_RoleModel->meshes.size(); ++j) {
			m_RoleMaterial->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, j);
			m_RoleModel->meshes[j]->BindDrawCmd(commandBuffer);
		}

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

		for (int32_t i = 0; i < MAX_BONES; ++i) {
			m_BonesData.bones[i].SetIdentity();
		}
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

	ModelViewProjectionBlock	m_MVPData;
	BonesTransformBlock			m_BonesData;

	VKModel* m_RoleModel = nullptr;
	VKShader* m_RoleShader = nullptr;
	VKTexture* m_RoleDiffuse = nullptr;
	VKMaterial* m_RoleMaterial = nullptr;

	ImageGUIContext* m_GUI = nullptr;

	bool                        m_AutoAnimation = true;
	float                       m_AnimDuration = 0.0f;
	float                       m_AnimTime = 0.0f;
	int32_t                       m_AnimIndex = 0;
};