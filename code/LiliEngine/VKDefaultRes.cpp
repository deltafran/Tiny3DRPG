#include "stdafx.h"
#include "VKDefaultRes.h"

VKTexture* VKDefaultRes::texture2D = nullptr;
VKModel* VKDefaultRes::fullQuad = nullptr;

void VKDefaultRes::Init(std::shared_ptr<VulkanDevice> vulkanDevice, VKCommandBuffer* cmdBuffer)
{
    // quad model
    std::vector<float> vertices = {
        -1.0f,  1.0f, 0.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 0.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 1.0f
    };
    std::vector<uint16_t> indices = {
        0, 1, 2, 0, 2, 3
    };

    fullQuad = VKModel::Create(
        vulkanDevice,
        cmdBuffer,
        vertices,
        indices,
        { VertexAttribute::VA_Position, VertexAttribute::VA_UV0 }
    );

    // default 2D
    std::vector<uint8_t> rgba(64 * 64 * 4);
    for (int32_t i = 0; i < 64; ++i)
    {
        for (int32_t j = 0; j < 64; ++j)
        {
            int32_t index = (i * 64 + j) * 4;
            if ((j & 8) ^ (i & 8))
            {
                rgba[index + 0] = 255;
                rgba[index + 1] = 255;
                rgba[index + 2] = 255;
                rgba[index + 3] = 255;
            }
            else
            {
                rgba[index + 0] = 0;
                rgba[index + 1] = 0;
                rgba[index + 2] = 0;
                rgba[index + 3] = 255;
            }
        }
    }

    texture2D = VKTexture::Create2D(rgba.data(), 64 * 64 * 4, VK_FORMAT_R8G8B8A8_UNORM, 64, 64, vulkanDevice, cmdBuffer);
}

void VKDefaultRes::Destroy()
{
    delete fullQuad;
    delete texture2D;
}