#pragma once

#include "VKCommandBuffer.h"
#include "VKTexture.h"
#include "VKModel.h"

class VKDefaultRes
{
public:
    static void Init(std::shared_ptr<VulkanDevice> vulkanDevice, VKCommandBuffer* cmdBuffer);

    static void Destroy();

public:
    static VKTexture* texture2D;
    static VKModel* fullQuad;
};