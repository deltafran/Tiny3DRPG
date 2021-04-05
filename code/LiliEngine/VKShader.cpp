#include "stdafx.h"
#include "VKShader.h"
#include "VulkanDevice.h"
#include "RHIDefinitions.h"
#include "VKVertexBuffer.h"

VKShaderModule* VKShaderModule::Create(std::shared_ptr<VulkanDevice> vulkanDevice, const char* filename, VkShaderStageFlagBits stage)
{
    VkDevice device = vulkanDevice->GetInstanceHandle();

    uint8_t* dataPtr = nullptr;
    uint32_t dataSize = 0;
    if (!FileManager::ReadFile(filename, dataPtr, dataSize))
    {
        MLOGE("Failed load file:%s", filename);
        return nullptr;
    }

    VkShaderModuleCreateInfo moduleCreateInfo;
    ZeroVulkanStruct(moduleCreateInfo, VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO);
    moduleCreateInfo.codeSize = dataSize;
    moduleCreateInfo.pCode = (uint32_t*)dataPtr;

    VkShaderModule shaderModule = VK_NULL_HANDLE;
    VERIFYVULKANRESULT(vkCreateShaderModule(device, &moduleCreateInfo, VULKAN_CPU_ALLOCATOR, &shaderModule));

    VKShaderModule* dvkModule = new VKShaderModule();
    dvkModule->data = dataPtr;
    dvkModule->size = dataSize;
    dvkModule->device = device;
    dvkModule->handle = shaderModule;
    dvkModule->stage = stage;

    return dvkModule;
}

VKShader* VKShader::Create(std::shared_ptr<VulkanDevice> vulkanDevice, bool dynamicUBO, const char* vert, const char* frag, const char* geom, const char* comp, const char* tesc, const char* tese)
{
    VKShaderModule* vertModule = vert ? VKShaderModule::Create(vulkanDevice, vert, VK_SHADER_STAGE_VERTEX_BIT) : nullptr;
    VKShaderModule* fragModule = frag ? VKShaderModule::Create(vulkanDevice, frag, VK_SHADER_STAGE_FRAGMENT_BIT) : nullptr;
    VKShaderModule* geomModule = geom ? VKShaderModule::Create(vulkanDevice, geom, VK_SHADER_STAGE_GEOMETRY_BIT) : nullptr;
    VKShaderModule* compModule = comp ? VKShaderModule::Create(vulkanDevice, comp, VK_SHADER_STAGE_COMPUTE_BIT) : nullptr;
    VKShaderModule* tescModule = tesc ? VKShaderModule::Create(vulkanDevice, tesc, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) : nullptr;
    VKShaderModule* teseModule = tese ? VKShaderModule::Create(vulkanDevice, tese, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) : nullptr;

    VKShader* shader = new VKShader();
    shader->device = vulkanDevice->GetInstanceHandle();
    shader->dynamicUBO = dynamicUBO;

    shader->vertShaderModule = vertModule;
    shader->fragShaderModule = fragModule;
    shader->geomShaderModule = geomModule;
    shader->compShaderModule = compModule;
    shader->tescShaderModule = tescModule;
    shader->teseShaderModule = teseModule;

    shader->Compile();

    return shader;
}

VKShader* VKShader::Create(std::shared_ptr<VulkanDevice> vulkanDevice, const char* comp)
{
    return Create(vulkanDevice, true, nullptr, nullptr, nullptr, comp, nullptr, nullptr);
}

VKShader* VKShader::Create(std::shared_ptr<VulkanDevice> vulkanDevice, const char* vert, const char* frag, const char* geom, const char* comp, const char* tesc, const char* tese)
{
    return Create(vulkanDevice, false, vert, frag, geom, comp, tesc, tese);
}

void VKShader::ProcessAttachments(spirv_cross::Compiler& compiler, spirv_cross::ShaderResources& resources, VkPipelineStageFlags stageFlags)
{
    for (int32_t i = 0; i < resources.subpass_inputs.size(); ++i)
    {
        spirv_cross::Resource& res = resources.subpass_inputs[i];
        spirv_cross::SPIRType type = compiler.get_type(res.type_id);
        spirv_cross::SPIRType base_type = compiler.get_type(res.base_type_id);
        const std::string& varName = compiler.get_name(res.id);

        int32_t set = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
        int32_t binding = compiler.get_decoration(res.id, spv::DecorationBinding);

        VkDescriptorSetLayoutBinding setLayoutBinding = {};
        setLayoutBinding.binding = binding;
        setLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        setLayoutBinding.descriptorCount = 1;
        setLayoutBinding.stageFlags = stageFlags;
        setLayoutBinding.pImmutableSamplers = nullptr;

        setLayoutsInfo.AddDescriptorSetLayoutBinding(varName, set, setLayoutBinding);

        auto it = imageParams.find(varName);
        if (it == imageParams.end())
        {
            ImageInfo imageInfo = {};
            imageInfo.set = set;
            imageInfo.binding = binding;
            imageInfo.stageFlags = stageFlags;
            imageInfo.descriptorType = setLayoutBinding.descriptorType;
            imageParams.insert(std::make_pair(varName, imageInfo));
        }
        else
        {
            it->second.stageFlags |= stageFlags;
        }

    }
}

void VKShader::ProcessUniformBuffers(spirv_cross::Compiler& compiler, spirv_cross::ShaderResources& resources, VkShaderStageFlags stageFlags)
{
    for (int32_t i = 0; i < resources.uniform_buffers.size(); ++i)
    {
        spirv_cross::Resource& res = resources.uniform_buffers[i];
        spirv_cross::SPIRType type = compiler.get_type(res.type_id);
        spirv_cross::SPIRType base_type = compiler.get_type(res.base_type_id);
        const std::string& varName = compiler.get_name(res.id);
        const std::string& typeName = compiler.get_name(res.base_type_id);
        uint32_t uniformBufferStructSize = compiler.get_declared_struct_size(type);

        int32_t set = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
        int32_t binding = compiler.get_decoration(res.id, spv::DecorationBinding);

        // [layout (binding = 0) uniform MVPDynamicBlock] 
        VkDescriptorSetLayoutBinding setLayoutBinding = {};
        setLayoutBinding.binding = binding;
        setLayoutBinding.descriptorType = (typeName.find("Dynamic") != std::string::npos || dynamicUBO) ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        setLayoutBinding.descriptorCount = 1;
        setLayoutBinding.stageFlags = stageFlags;
        setLayoutBinding.pImmutableSamplers = nullptr;

        setLayoutsInfo.AddDescriptorSetLayoutBinding(varName, set, setLayoutBinding);

        auto it = bufferParams.find(varName);
        if (it == bufferParams.end())
        {
            BufferInfo bufferInfo = {};
            bufferInfo.set = set;
            bufferInfo.binding = binding;
            bufferInfo.bufferSize = uniformBufferStructSize;
            bufferInfo.stageFlags = stageFlags;
            bufferInfo.descriptorType = setLayoutBinding.descriptorType;
            bufferParams.insert(std::make_pair(varName, bufferInfo));
        }
        else
        {
            it->second.stageFlags |= setLayoutBinding.stageFlags;
        }
    }
}

void VKShader::ProcessTextures(spirv_cross::Compiler& compiler, spirv_cross::ShaderResources& resources, VkShaderStageFlags stageFlags)
{
    for (int32_t i = 0; i < resources.sampled_images.size(); ++i)
    {
        spirv_cross::Resource& res = resources.sampled_images[i];
        spirv_cross::SPIRType type = compiler.get_type(res.type_id);
        spirv_cross::SPIRType base_type = compiler.get_type(res.base_type_id);
        const std::string& varName = compiler.get_name(res.id);

        int32_t set = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
        int32_t binding = compiler.get_decoration(res.id, spv::DecorationBinding);

        VkDescriptorSetLayoutBinding setLayoutBinding = {};
        setLayoutBinding.binding = binding;
        setLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        setLayoutBinding.descriptorCount = 1;
        setLayoutBinding.stageFlags = stageFlags;
        setLayoutBinding.pImmutableSamplers = nullptr;

        setLayoutsInfo.AddDescriptorSetLayoutBinding(varName, set, setLayoutBinding);

        auto it = imageParams.find(varName);
        if (it == imageParams.end())
        {
            ImageInfo imageInfo = {};
            imageInfo.set = set;
            imageInfo.binding = binding;
            imageInfo.stageFlags = stageFlags;
            imageInfo.descriptorType = setLayoutBinding.descriptorType;
            imageParams.insert(std::make_pair(varName, imageInfo));
        }
        else
        {
            it->second.stageFlags |= stageFlags;
        }
    }
}

void VKShader::ProcessInput(spirv_cross::Compiler& compiler, spirv_cross::ShaderResources& resources, VkShaderStageFlags stageFlags)
{
    if (stageFlags != VK_SHADER_STAGE_VERTEX_BIT) {
        return;
    }

    for (int32_t i = 0; i < resources.stage_inputs.size(); ++i)
    {
        spirv_cross::Resource& res = resources.stage_inputs[i];
        spirv_cross::SPIRType type = compiler.get_type(res.type_id);
        const std::string& varName = compiler.get_name(res.id);
        int32_t inputAttributeSize = type.vecsize;

        VertexAttribute attribute = StringToVertexAttribute(varName.c_str());
        if (attribute == VertexAttribute::VA_None)
        {
            if (inputAttributeSize == 1) {
                attribute = VertexAttribute::VA_InstanceFloat1;
            }
            else if (inputAttributeSize == 2) {
                attribute = VertexAttribute::VA_InstanceFloat2;
            }
            else if (inputAttributeSize == 3) {
                attribute = VertexAttribute::VA_InstanceFloat3;
            }
            else if (inputAttributeSize == 4) {
                attribute = VertexAttribute::VA_InstanceFloat4;
            }
            MLOG("Not found attribute : %s, treat as instance attribute : %d.", varName.c_str(), int32_t(attribute));
        }

        int32_t location = compiler.get_decoration(res.id, spv::DecorationLocation);
        VKAttribute dvkAttribute = {};
        dvkAttribute.location = location;
        dvkAttribute.attribute = attribute;
        m_InputAttributes.push_back(dvkAttribute);
    }
}

void VKShader::ProcessStorageBuffers(spirv_cross::Compiler& compiler, spirv_cross::ShaderResources& resources, VkShaderStageFlags stageFlags)
{
    for (int32_t i = 0; i < resources.storage_buffers.size(); ++i)
    {
        spirv_cross::Resource& res = resources.storage_buffers[i];
        spirv_cross::SPIRType type = compiler.get_type(res.type_id);
        spirv_cross::SPIRType base_type = compiler.get_type(res.base_type_id);
        const std::string& varName = compiler.get_name(res.id);

        int32_t set = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
        int32_t binding = compiler.get_decoration(res.id, spv::DecorationBinding);

        VkDescriptorSetLayoutBinding setLayoutBinding = {};
        setLayoutBinding.binding = binding;
        setLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        setLayoutBinding.descriptorCount = 1;
        setLayoutBinding.stageFlags = stageFlags;
        setLayoutBinding.pImmutableSamplers = nullptr;

        setLayoutsInfo.AddDescriptorSetLayoutBinding(varName, set, setLayoutBinding);

        auto it = bufferParams.find(varName);
        if (it == bufferParams.end())
        {
            BufferInfo bufferInfo = {};
            bufferInfo.set = set;
            bufferInfo.binding = binding;
            bufferInfo.bufferSize = 0;
            bufferInfo.stageFlags = stageFlags;
            bufferInfo.descriptorType = setLayoutBinding.descriptorType;
            bufferParams.insert(std::make_pair(varName, bufferInfo));
        }
        else
        {
            it->second.stageFlags = it->second.stageFlags | setLayoutBinding.stageFlags;
        }
    }
}

void VKShader::ProcessStorageImages(spirv_cross::Compiler& compiler, spirv_cross::ShaderResources& resources, VkShaderStageFlags stageFlags)
{
    for (int32_t i = 0; i < resources.storage_images.size(); ++i)
    {
        spirv_cross::Resource& res = resources.storage_images[i];
        spirv_cross::SPIRType type = compiler.get_type(res.type_id);
        spirv_cross::SPIRType base_type = compiler.get_type(res.base_type_id);
        const std::string& varName = compiler.get_name(res.id);

        int32_t set = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
        int32_t binding = compiler.get_decoration(res.id, spv::DecorationBinding);

        VkDescriptorSetLayoutBinding setLayoutBinding = {};
        setLayoutBinding.binding = binding;
        setLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        setLayoutBinding.descriptorCount = 1;
        setLayoutBinding.stageFlags = stageFlags;
        setLayoutBinding.pImmutableSamplers = nullptr;

        setLayoutsInfo.AddDescriptorSetLayoutBinding(varName, set, setLayoutBinding);

        auto it = imageParams.find(varName);
        if (it == imageParams.end())
        {
            ImageInfo imageInfo = {};
            imageInfo.set = set;
            imageInfo.binding = binding;
            imageInfo.stageFlags = stageFlags;
            imageInfo.descriptorType = setLayoutBinding.descriptorType;
            imageParams.insert(std::make_pair(varName, imageInfo));
        }
        else
        {
            it->second.stageFlags |= stageFlags;
        }

    }
}

void VKShader::ProcessShaderModule(VKShaderModule* shaderModule)
{
    if (!shaderModule) {
        return;
    }

    VkPipelineShaderStageCreateInfo shaderCreateInfo;
    ZeroVulkanStruct(shaderCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
    shaderCreateInfo.stage = shaderModule->stage;
    shaderCreateInfo.module = shaderModule->handle;
    shaderCreateInfo.pName = "main";
    shaderStageCreateInfos.push_back(shaderCreateInfo);

    spirv_cross::Compiler compiler((uint32_t*)shaderModule->data, shaderModule->size / sizeof(uint32_t));
    spirv_cross::ShaderResources resources = compiler.get_shader_resources();

    ProcessAttachments(compiler, resources, shaderModule->stage);
    ProcessUniformBuffers(compiler, resources, shaderModule->stage);
    ProcessTextures(compiler, resources, shaderModule->stage);
    ProcessStorageImages(compiler, resources, shaderModule->stage);
    ProcessInput(compiler, resources, shaderModule->stage);
    ProcessStorageBuffers(compiler, resources, shaderModule->stage);

}

void VKShader::Compile()
{
    ProcessShaderModule(vertShaderModule);
    ProcessShaderModule(fragShaderModule);
    ProcessShaderModule(geomShaderModule);
    ProcessShaderModule(compShaderModule);
    ProcessShaderModule(tescShaderModule);
    ProcessShaderModule(teseShaderModule);
    GenerateInputInfo();
    GenerateLayout();
}

void VKShader::GenerateInputInfo()
{
    std::sort(m_InputAttributes.begin(), m_InputAttributes.end(), [](const VKAttribute& a, const VKAttribute& b) -> bool {
        return a.location < b.location;
        });

    for (int32_t i = 0; i < m_InputAttributes.size(); ++i)
    {
        VertexAttribute attribute = m_InputAttributes[i].attribute;
        if (attribute == VA_InstanceFloat1 || attribute == VA_InstanceFloat2 || attribute == VA_InstanceFloat3 || attribute == VA_InstanceFloat4) {
            instancesAttributes.push_back(attribute);
        }
        else {
            perVertexAttributes.push_back(attribute);
        }
    }

    inputBindings.resize(0);
    if (perVertexAttributes.size() > 0)
    {
        int32_t stride = 0;
        for (int32_t i = 0; i < perVertexAttributes.size(); ++i) {
            stride += VertexAttributeToSize(perVertexAttributes[i]);
        }
        VkVertexInputBindingDescription perVertexInputBinding = {};
        perVertexInputBinding.binding = 0;
        perVertexInputBinding.stride = stride;
        perVertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        inputBindings.push_back(perVertexInputBinding);
    }

    if (instancesAttributes.size() > 0)
    {
        int32_t stride = 0;
        for (int32_t i = 0; i < instancesAttributes.size(); ++i) {
            stride += VertexAttributeToSize(instancesAttributes[i]);
        }
        VkVertexInputBindingDescription instanceInputBinding = {};
        instanceInputBinding.binding = 1;
        instanceInputBinding.stride = stride;
        instanceInputBinding.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
        inputBindings.push_back(instanceInputBinding);
    }

    int location = 0;
    if (perVertexAttributes.size() > 0)
    {
        int32_t offset = 0;
        for (int32_t i = 0; i < perVertexAttributes.size(); ++i)
        {
            VkVertexInputAttributeDescription inputAttribute = {};
            inputAttribute.binding = 0;
            inputAttribute.location = location;
            inputAttribute.format = VertexAttributeToVkFormat(perVertexAttributes[i]);
            inputAttribute.offset = offset;
            offset += VertexAttributeToSize(perVertexAttributes[i]);
            inputAttributes.push_back(inputAttribute);

            location += 1;
        }
    }

    if (instancesAttributes.size() > 0)
    {
        int32_t offset = 0;
        for (int32_t i = 0; i < instancesAttributes.size(); ++i)
        {
            VkVertexInputAttributeDescription inputAttribute = {};
            inputAttribute.binding = 1;
            inputAttribute.location = location;
            inputAttribute.format = VertexAttributeToVkFormat(instancesAttributes[i]);
            inputAttribute.offset = offset;
            offset += VertexAttributeToSize(instancesAttributes[i]);
            inputAttributes.push_back(inputAttribute);

            location += 1;
        }
    }

}

void VKShader::GenerateLayout()
{
    std::vector<VKDescriptorSetLayoutInfo>& setLayouts = setLayoutsInfo.setLayouts;

    std::sort(setLayouts.begin(), setLayouts.end(), [](const VKDescriptorSetLayoutInfo& a, const VKDescriptorSetLayoutInfo& b) -> bool {
        return a.set < b.set;
        });

    for (int32_t i = 0; i < setLayouts.size(); ++i)
    {
        std::vector<VkDescriptorSetLayoutBinding>& bindings = setLayouts[i].bindings;
        std::sort(bindings.begin(), bindings.end(), [](const VkDescriptorSetLayoutBinding& a, const VkDescriptorSetLayoutBinding& b) -> bool {
            return a.binding < b.binding;
            });
    }

    for (int32_t i = 0; i < setLayoutsInfo.setLayouts.size(); ++i)
    {
        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
        VKDescriptorSetLayoutInfo& setLayoutInfo = setLayoutsInfo.setLayouts[i];

        VkDescriptorSetLayoutCreateInfo descSetLayoutInfo;
        ZeroVulkanStruct(descSetLayoutInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
        descSetLayoutInfo.bindingCount = setLayoutInfo.bindings.size();
        descSetLayoutInfo.pBindings = setLayoutInfo.bindings.data();
        VERIFYVULKANRESULT(vkCreateDescriptorSetLayout(device, &descSetLayoutInfo, VULKAN_CPU_ALLOCATOR, &descriptorSetLayout));

        descriptorSetLayouts.push_back(descriptorSetLayout);
    }

    VkPipelineLayoutCreateInfo pipeLayoutInfo;
    ZeroVulkanStruct(pipeLayoutInfo, VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);
    pipeLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
    pipeLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    VERIFYVULKANRESULT(vkCreatePipelineLayout(device, &pipeLayoutInfo, VULKAN_CPU_ALLOCATOR, &pipelineLayout));
}