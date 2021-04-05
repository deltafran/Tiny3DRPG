#pragma once

#include "VKTexture.h"
#include "CoreMath2.h"
#include "Vector4.h"

struct VKRenderPassInfo
{
    struct ColorEntry
    {
        VKTexture* renderTarget = nullptr;
        VKTexture* resolveTarget = nullptr;

        VkAttachmentLoadOp  loadAction = VK_ATTACHMENT_LOAD_OP_CLEAR;
        VkAttachmentStoreOp storeAction = VK_ATTACHMENT_STORE_OP_STORE;
    };

    struct DepthStencilEntry
    {
        VKTexture* depthStencilTarget = nullptr;
        VKTexture* resolveTarget = nullptr;
        VkAttachmentLoadOp  loadAction = VK_ATTACHMENT_LOAD_OP_CLEAR;
        VkAttachmentStoreOp storeAction = VK_ATTACHMENT_STORE_OP_STORE;
    };

    ColorEntry              colorRenderTargets[MaxSimultaneousRenderTargets];
    DepthStencilEntry       depthStencilRenderTarget;
    int32_t                   numColorRenderTargets;
    bool					multiview = false;

    // Color, no depth
    explicit VKRenderPassInfo(VKTexture* colorRT, VkAttachmentLoadOp colorLoadAction, VkAttachmentStoreOp colorStoreAction, VKTexture* resolveRT)
    {
        numColorRenderTargets = 1;

        colorRenderTargets[0].renderTarget = colorRT;
        colorRenderTargets[0].resolveTarget = resolveRT;
        colorRenderTargets[0].loadAction = colorLoadAction;
        colorRenderTargets[0].storeAction = colorStoreAction;

        depthStencilRenderTarget.depthStencilTarget = nullptr;
        depthStencilRenderTarget.resolveTarget = nullptr;
        depthStencilRenderTarget.loadAction = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthStencilRenderTarget.storeAction = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        memset(&colorRenderTargets[numColorRenderTargets], 0, sizeof(ColorEntry) * (MaxSimultaneousRenderTargets - numColorRenderTargets));
    }

    // Color And Depth
    explicit VKRenderPassInfo(
        VKTexture* colorRT,
        VkAttachmentLoadOp colorLoadAction,
        VkAttachmentStoreOp colorStoreAction,
        VKTexture* depthRT,
        VkAttachmentLoadOp depthLoadAction,
        VkAttachmentStoreOp depthStoreAction
    )
    {
        numColorRenderTargets = 1;

        colorRenderTargets[0].renderTarget = colorRT;
        colorRenderTargets[0].resolveTarget = nullptr;
        colorRenderTargets[0].loadAction = colorLoadAction;
        colorRenderTargets[0].storeAction = colorStoreAction;

        depthStencilRenderTarget.depthStencilTarget = depthRT;
        depthStencilRenderTarget.resolveTarget = nullptr;
        depthStencilRenderTarget.loadAction = depthLoadAction;
        depthStencilRenderTarget.storeAction = depthStoreAction;

        memset(&colorRenderTargets[numColorRenderTargets], 0, sizeof(ColorEntry) * (MaxSimultaneousRenderTargets - numColorRenderTargets));
    }

    // MRTs, No Depth
    explicit VKRenderPassInfo(int32_t numColorRTs, VKTexture* colorRT[], VkAttachmentLoadOp colorLoadAction, VkAttachmentStoreOp colorStoreAction)
    {
        numColorRenderTargets = numColorRTs;

        for (int32_t i = 0; i < numColorRTs; ++i)
        {
            colorRenderTargets[i].renderTarget = colorRT[i];
            colorRenderTargets[i].resolveTarget = nullptr;
            colorRenderTargets[i].loadAction = colorLoadAction;
            colorRenderTargets[i].storeAction = colorStoreAction;
        }

        depthStencilRenderTarget.depthStencilTarget = nullptr;
        depthStencilRenderTarget.resolveTarget = nullptr;
        depthStencilRenderTarget.loadAction = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthStencilRenderTarget.storeAction = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        if (numColorRTs < MaxSimultaneousRenderTargets) {
            memset(&colorRenderTargets[numColorRenderTargets], 0, sizeof(ColorEntry) * (MaxSimultaneousRenderTargets - numColorRenderTargets));
        }
    }

    // MRTs And Depth
    explicit VKRenderPassInfo(
        int32_t numColorRTs,
        VKTexture* colorRT[],
        VkAttachmentLoadOp colorLoadAction,
        VkAttachmentStoreOp colorStoreAction,
        VKTexture* depthRT,
        VkAttachmentLoadOp depthLoadAction,
        VkAttachmentStoreOp depthStoreAction
    )
    {
        numColorRenderTargets = numColorRTs;

        for (int32_t i = 0; i < numColorRTs; ++i)
        {
            colorRenderTargets[i].renderTarget = colorRT[i];
            colorRenderTargets[i].resolveTarget = nullptr;
            colorRenderTargets[i].loadAction = colorLoadAction;
            colorRenderTargets[i].storeAction = colorStoreAction;
        }

        depthStencilRenderTarget.depthStencilTarget = depthRT;
        depthStencilRenderTarget.resolveTarget = nullptr;
        depthStencilRenderTarget.loadAction = depthLoadAction;
        depthStencilRenderTarget.storeAction = depthStoreAction;

        if (numColorRTs < MaxSimultaneousRenderTargets) {
            memset(&colorRenderTargets[numColorRenderTargets], 0, sizeof(ColorEntry) * (MaxSimultaneousRenderTargets - numColorRenderTargets));
        }
    }

    // Depth, No Color
    explicit VKRenderPassInfo(VKTexture* depthRT, VkAttachmentLoadOp depthLoadAction, VkAttachmentStoreOp depthStoreAction)
    {
        numColorRenderTargets = 0;

        depthStencilRenderTarget.depthStencilTarget = depthRT;
        depthStencilRenderTarget.resolveTarget = nullptr;
        depthStencilRenderTarget.loadAction = depthLoadAction;
        depthStencilRenderTarget.storeAction = depthStoreAction;

        memset(&colorRenderTargets[numColorRenderTargets], 0, sizeof(ColorEntry) * MaxSimultaneousRenderTargets);
    }
};

class VKRenderTargetLayout
{
public:
    VKRenderTargetLayout(const VKRenderPassInfo& renderPassInfo);

    uint16_t SetupSubpasses(VkSubpassDescription* outDescs, uint32_t maxDescs, VkSubpassDependency* outDeps, uint32_t maxDeps, uint32_t& outNumDependencies) const;

public:
    VkAttachmentReference   colorReferences[MaxSimultaneousRenderTargets];
    VkAttachmentReference   depthStencilReference;
    VkAttachmentReference   resolveReferences[MaxSimultaneousRenderTargets];
    VkAttachmentReference   inputAttachments[MaxSimultaneousRenderTargets + 1];

    VkAttachmentDescription descriptions[MaxSimultaneousRenderTargets * 2 + 1];

    uint8_t                   numAttachmentDescriptions = 0;
    uint8_t                   numColorAttachments = 0;
    uint8_t                   numInputAttachments = 0;
    bool                    hasDepthStencil = false;
    bool                    hasResolveAttachments = false;
    VkSampleCountFlagBits   numSamples = VK_SAMPLE_COUNT_1_BIT;
    uint8_t                   numUsedClearValues = 0;

    VkExtent3D	            extent3D;
    bool					multiview = false;
};

class VKRenderPass
{
public:
    VKRenderPass(VkDevice inDevice, const VKRenderTargetLayout& rtLayout);

    ~VKRenderPass()
    {
        if (renderPass != VK_NULL_HANDLE)
        {
            vkDestroyRenderPass(device, renderPass, VULKAN_CPU_ALLOCATOR);
            renderPass = VK_NULL_HANDLE;
        }
    }

public:
    VKRenderTargetLayout	layout;
    VkRenderPass		    renderPass = VK_NULL_HANDLE;
    uint32_t				    numUsedClearValues = 0;
    VkDevice		        device = VK_NULL_HANDLE;
};

class VKFrameBuffer
{
public:
    VKFrameBuffer(VkDevice device, const VKRenderTargetLayout& rtLayout, const VKRenderPass& renderPass, const VKRenderPassInfo& renderPassInfo);

    ~VKFrameBuffer()
    {
        if (frameBuffer != VK_NULL_HANDLE)
        {
            vkDestroyFramebuffer(device, frameBuffer, VULKAN_CPU_ALLOCATOR);
            frameBuffer = VK_NULL_HANDLE;
        }
    }

public:
    typedef std::vector<VkImageView> ImageViews;

    VkDevice            device = VK_NULL_HANDLE;

    VkFramebuffer       frameBuffer = VK_NULL_HANDLE;
    uint32_t              numColorRenderTargets = 0;
    uint32_t              numColorAttachments = 0;

    ImageViews          attachmentTextureViews;
    VkImage             colorRenderTargetImages[MaxSimultaneousRenderTargets];
    VkImage             depthStencilRenderTargetImage = VK_NULL_HANDLE;

    VkExtent2D          extent2D;
};

class VKRenderTarget
{
private:
    VKRenderTarget(const VKRenderPassInfo& inRenderPassInfo)
        : rtLayout(inRenderPassInfo)
        , renderPassInfo(inRenderPassInfo)
        , clearColor(0, 0, 0, 1)
    {
        for (int32_t i = 0; i < inRenderPassInfo.numColorRenderTargets; ++i)
        {
            VkClearValue clearValue = {};
            clearValue.color = { { clearColor.x, clearColor.y, clearColor.z, clearColor.w } };
            clearValues.push_back(clearValue);
        }

        if (inRenderPassInfo.depthStencilRenderTarget.depthStencilTarget)
        {
            VkClearValue clearValue = {};
            clearValue.depthStencil = { 1.0f, 0 };
            clearValues.push_back(clearValue);
        }

        colorLayout = ImageLayoutBarrier::PixelShaderRead;
        depthLayout = ImageLayoutBarrier::PixelShaderRead;
    }

    VKRenderTarget(const VKRenderPassInfo& inRenderPassInfo, Vector4 inClearColor)
        : rtLayout(inRenderPassInfo)
        , renderPassInfo(inRenderPassInfo)
        , clearColor(inClearColor)
    {
        for (int32_t i = 0; i < inRenderPassInfo.numColorRenderTargets; ++i)
        {
            VkClearValue clearValue = {};
            clearValue.color = { { clearColor.x, clearColor.y, clearColor.z, clearColor.w } };
            clearValues.push_back(clearValue);
        }

        if (inRenderPassInfo.depthStencilRenderTarget.depthStencilTarget)
        {
            VkClearValue clearValue = {};
            clearValue.depthStencil = { 1.0f, 0 };
            clearValues.push_back(clearValue);
        }

        colorLayout = ImageLayoutBarrier::PixelShaderRead;
        depthLayout = ImageLayoutBarrier::PixelShaderRead;
    }

public:
    ~VKRenderTarget()
    {
        if (renderPass)
        {
            delete renderPass;
            renderPass = nullptr;
        }

        if (frameBuffer)
        {
            delete frameBuffer;
            frameBuffer = nullptr;
        }
    }

    void BeginRenderPass(VkCommandBuffer cmdBuffer);

    void EndRenderPass(VkCommandBuffer cmdBuffer);

    inline VkRenderPass GetRenderPass() const
    {
        return renderPass->renderPass;
    }

    inline VkFramebuffer GetFrameBuffer() const
    {
        return frameBuffer->frameBuffer;
    }

    static VKRenderTarget* Create(std::shared_ptr<VulkanDevice> vulkanDevice, const VKRenderPassInfo& inRenderPassInfo);

    static VKRenderTarget* Create(std::shared_ptr<VulkanDevice> vulkanDevice, const VKRenderPassInfo& inRenderPassInfo, Vector4 clearColor);

public:
    VKRenderTargetLayout       rtLayout;
    VKRenderPassInfo           renderPassInfo;
    Vector4						clearColor;

    VKRenderPass* renderPass = nullptr;
    VKFrameBuffer* frameBuffer = nullptr;

    VkDevice                    device = VK_NULL_HANDLE;
    VkExtent2D                  extent2D;
    std::vector<VkClearValue>   clearValues;

    ImageLayoutBarrier			colorLayout;
    ImageLayoutBarrier			depthLayout;
};